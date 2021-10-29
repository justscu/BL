#include "parse_l2_layer.h"
#include "parse_l3_layer.h"
#include <functional>
#include <algorithm>

bool ParseIPLayer::init(const char *src_ip, const char *dst_ip,
                           uint16_t src_port, uint16_t dst_port,
                           uint8_t protocol)
{
    cmp_ip_   = inet_addr(dst_ip);
    cmp_ip_ <<= 32;
    cmp_ip_  |= inet_addr(src_ip);

    cmp_port_   = htons(dst_port);
    cmp_port_ <<= 16;
    cmp_port_  |= htons(src_port);

    protocol_ = protocol;

    if (protocol_ == PROTOCOL_UDP) {
        udp_layer_ = new (std::nothrow) ParseUDPLayer;
    }
    else if (protocol_ == PROTOCOL_TCP) {
        tcp_layer_ = new (std::nothrow) ParseTCPLayer;
    }

    return (tcp_layer_ != nullptr) || (udp_layer_ != nullptr);
}

// str: IP层数据(含头部), 如果需要解析，返回true
bool ParseIPLayer::need_parse(const char *str) const {
    const IpHdr *hd = (const IpHdr*)str;
    int32_t len = hd->ihl * 4;

    uint64_t   ip = *(uint64_t*)(str+12);
    // uint32_t port = *(uint32_t*)(str+len);
    uint8_t proto = *(uint8_t*)(str+9);
    if ((ip == cmp_ip_) && (proto == protocol_)) {
        return true;
    }
    return false;
}

void ParseIPLayer::parse(const char *str, const int32_t len) {
    if (!need_parse(str)) { return; }

    const IpHdr *hd = (const IpHdr*)str;
    const uint32_t payload_len = ip_pkg_length(hd) - ip_header_length(hd);

    ipfragment frag;
    frag.offset = frag_offset(hd);
    frag.len    = payload_len;
    frag.addr   = str + ip_header_length(hd);

    if (is_not_fragment(hd)) {
        if (protocol_ == PROTOCOL_UDP) {
            udp_layer_->parse(frag);
        }
        else if (protocol_ == PROTOCOL_TCP) {
            tcp_layer_->parse(frag);
        }

        return;
    }

    fprintf(stdout, " fragment. ");

    const uint16_t key = hd->id;
    std::unordered_map<uint16_t, ippkg>::iterator it = ip_pkgs_.find(key);
    if (it == ip_pkgs_.end()) {
        ippkg ip;
        ip.identifier     = key;
        ip.src_ip         = hd->src_ip;
        ip.dst_ip         = hd->dst_ip;
        ip.data_total_len = payload_len;
        ip.recv_last_fragment = is_last_fragment(hd);
        ip.fragments.emplace_back(frag);

        ip_pkgs_.emplace(key, ip);
        return;
    }

    if (insert_new_fragment(it->second.fragments, frag)) {
        it->second.data_total_len += payload_len;

        if (!it->second.recv_last_fragment) {
            it->second.recv_last_fragment = is_last_fragment(hd);
        }
        if (it->second.recv_last_fragment) {
            if (is_done(it->second)) {
                if (protocol_ == PROTOCOL_UDP) {
                    udp_layer_->parse(it->second.fragments);
                }
                else if (protocol_ == PROTOCOL_TCP) {
                    tcp_layer_->parse(it->second.fragments);
                }

                fprintf(stdout, " done[0x%x, %u]. ", it->second.identifier, it->second.data_total_len);
                ip_pkgs_.erase(it);
            }
        }
    }
}

bool ParseIPLayer::is_done(const ippkg &pkg) const {
    uint32_t len = 0;
    for (auto &it : pkg.fragments) {
        len += it.len;
    }

    if (len > 0) {
        return len == pkg.data_total_len;
    }
    return false;
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
