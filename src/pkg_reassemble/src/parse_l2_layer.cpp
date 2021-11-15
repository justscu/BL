#include <functional>
#include <algorithm>
#include <strings.h>
#include <assert.h>
#include "headers.h"
#include "parse_l2_layer.h"
#include "parse_l3_layer.h"

void ParseIPLayer::set_ip_filter(const char *src_ip, const char *dst_ip) {
    filte_ip_   = inet_addr(dst_ip);
    filte_ip_ <<= 32;
    filte_ip_  |= inet_addr(src_ip);
}

void ParseIPLayer::set_protocol_filter(const char *protocol) {
    if (0 == strcasecmp(protocol, "tcp")) {
        filte_protocol_ = PROTOCOL_TCP;
    }
    else if (0 == strcasecmp(protocol, "udp")) {
        filte_protocol_ = PROTOCOL_UDP;
    }
    else {
    	assert(0);
    }
}

void ParseIPLayer::set_port_filter(uint16_t src_port, uint16_t dst_port) {
//    filte_src_port_   = htons(dst_port);
//    filte_src_port_ <<= 16;
    filte_src_port_  |= htons(src_port);
}

bool ParseIPLayer::create_l3_layer() {
    if (filte_protocol_ == PROTOCOL_UDP) {
        l3_layer_ = new (std::nothrow) ParseUDPLayer;
    }
    else if (filte_protocol_ == PROTOCOL_TCP) {
        l3_layer_ = new (std::nothrow) ParseTCPLayer;
    }
    else {
    	assert(0);
    	log_err("new l3_layer failed.");
    }

    if (l3_layer_) {
        l3_layer_->set_filter_src_port(filte_src_port_);
        return true;
    }
    return false;
}

// str: IP层数据(含头部), 如果需要解析，返回true
bool ParseIPLayer::need_parse(const char *str) const {
    uint64_t   ip = *(uint64_t*)(str+12);
    uint8_t proto = *(uint8_t*)(str+9);

    return (ip == filte_ip_) && (proto & filte_protocol_);
}

void ParseIPLayer::parse(const char *str, const int32_t len, const captime *ct) {
    print((const ip_hdr *)str);
    if (!need_parse(str)) { return; }

    const ip_hdr *hd = (const ip_hdr*)str;
    const uint32_t payload_len = ip_pkg_length(hd) - ip_header_length(hd);

    ipfragment frag;
    frag.offset = frag_offset(hd);
    frag.ip_payload_len  = payload_len;
    frag.ip_payload_addr = str + ip_header_length(hd);

    if (is_not_fragment(hd)) {
        if (l3_layer_->need_parse(frag.ip_payload_addr)) {
            l3_layer_->parse(frag, ct);
        }
        return;
    }

    log_dbg("ip_fragment.");

    const uint16_t key = pkg_identifier(hd);
    std::unordered_map<uint16_t, ippkg>::iterator it = ip_pkgs_.find(key);
    if (it == ip_pkgs_.end()) {
        ippkg ip;
        ip.identifier     = key;
        ip.src_ip         = hd->src_ip;
        ip.dst_ip         = hd->dst_ip;
        ip.data_total_len = payload_len;
        ip.recv_last_fragment = is_last_fragment(hd);
        ip.recv_fragment_time = ct->sec;
        ip.fragments.emplace_back(frag);

        ip_pkgs_.emplace(key, ip);
        return;
    }

    // IP包超时.
    if (it->second.recv_fragment_time + IP_TIMEOUT_SECONDS < ct->sec) {
        mon.cnt_timeout += 1;

        it->second.data_total_len = 0;
        it->second.fragments.clear();
        log_dbg("(ip_fragment timeout) ");
    }

    if (insert_new_fragment(it->second.fragments, frag)) {
        mon.cnt_fragments += 1;

        it->second.recv_fragment_time = ct->sec;
        it->second.data_total_len    += payload_len;

        if (!it->second.recv_last_fragment) {
            it->second.recv_last_fragment = is_last_fragment(hd);
        }

        if (it->second.recv_last_fragment) {
            if (is_done(it->second)) {
                std::vector<ipfragment>::iterator v = it->second.fragments.begin();
                if (l3_layer_->need_parse(v->ip_payload_addr+v->offset)) {
                    l3_layer_->parse(it->second.fragments, ct);
                    log_dbg("ip_fragment_done(identi[0x%x], len[%u]). ", it->second.identifier, it->second.data_total_len);
                }

                ip_pkgs_.erase(it);
            } // done
        } // if
    }
}

bool ParseIPLayer::is_done(const ippkg &pkg) const {
    uint32_t len = 0;
    for (auto &it : pkg.fragments) {
        len += it.ip_payload_len;
    }

    if (len > 0) {
        return len == pkg.data_total_len;
    }
    return false;
}

void ParseIPLayer::print(const ip_hdr *hd) const {
    char src[32] = {0}, dst[32] = {0};
    inet_ntop(AF_INET, &(hd->src_ip), src, sizeof(src));
    inet_ntop(AF_INET, &(hd->dst_ip), dst, sizeof(dst));

    log_dbg("[%16s -> %16s] ", src, dst);
}

void ParseIPLayer::print(const uint8_t *str, const int32_t len) const {
    for (int32_t i = 0; i < len; ++i) {
        fprintf(stdout, " %02X ", str[i]);
    }
    fprintf(stdout, "\n");
}

bool ParseIPLayer::insert_new_fragment(std::vector<ipfragment> &vec, const ipfragment &frag) {
    std::vector<ipfragment>::iterator it = vec.begin();
    for (; it != vec.end(); ++it) {
        if (it->offset > frag.offset) {
            vec.insert(it, frag);
            return true;
        }
        else if (it->offset == frag.offset) {
            return false;
        }
    }
    vec.push_back(frag);
    return true;
}
