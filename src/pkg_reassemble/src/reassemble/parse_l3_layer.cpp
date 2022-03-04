#include <assert.h>
#include "headers.h"
#include "parse_l2_layer.h"
#include "parse_l3_layer.h"

// 无符号比较大小
#define seq_equal(a, b)  (a) == (b)
#define seq_before(a, b) (int32_t((a)-(b))) < 0
#define seq_after(a, b)  seq_before(b, a)

bool ParseL3LayerBase::need_parse(const char *l3_str) const {
    return filter_src_port_ == *(uint16_t*)l3_str;
}

uint32_t ParseTCPLayer::parse(const timeval *ct, const ipfragment &frag) {
    assert(frag.offset == 0);

    // TCP流，包含头部
    const char *tcp_flow = frag.ip_payload_addr;
    const tcp_hdr *hd = (const tcp_hdr*)tcp_flow;


    const uint32_t payload_len = frag.ip_payload_len - hd->thl * 4;
    //
    if (payload_len == 0) {
        if (is_sync_pkg(hd)) {
            num_of_recved_pkgs_ = 0;
            next_tcp_seq_ = ntohl(hd->seq_no) + 1;
            pre_ok_time_  = 0;
            pkgs_cache_list_.clear();
            log_dbg("tcp_syn.");
        }
        else if (is_fin_pkg(hd) || is_rst_pkg(hd)) {
            num_of_recved_pkgs_ = 0;
            next_tcp_seq_ = 0;
            pre_ok_time_  = 0;
            pkgs_cache_list_.clear();
            log_dbg("tcp_fin or tcp_rst.");
        }
        return 0;
    }

    tcppkgq pkg;
    pkg.seq = ntohl(hd->seq_no);
    pkg.tcp_payload = frag.ip_payload_addr + hd->thl * 4;
    pkg.tcp_payload_len = payload_len;

    ++num_of_recved_pkgs_;
    log_dbg("TCP: [0x%x] [%u] ", pkg.seq, pkg.tcp_payload_len);

    if (check_and_callback(pkg)) {
        check_and_callback_tcppkg_in_cache();
        pre_ok_time_ = ct->tv_sec;
    }
    else {
        add_tcppkg_to_cache(pkg);
        // timeout.
        if (pre_ok_time_ + TCP_TIMEOUT_SECONDS < ct->tv_sec) {
            num_of_recved_pkgs_ = 0;
            next_tcp_seq_ = 0;
            pre_ok_time_ = ct->tv_sec;
            pkgs_cache_list_.clear();

            log_err("tcp_time_out.");
        }
    }
    return pkg.seq + payload_len;
}

void ParseTCPLayer::parse(const timeval *ct, const std::vector<ipfragment> &frags) {
    std::vector<ipfragment>::const_iterator it = frags.begin();
    uint32_t next_seq = parse(ct, *it);

    for (++it; it != frags.end(); ++it) {
        tcppkgq pkg;
        pkg.seq = next_seq;
        pkg.tcp_payload_len = it->ip_payload_len;
        pkg.tcp_payload = it->ip_payload_addr;
        next_seq += it->ip_payload_len;

        ++num_of_recved_pkgs_;
        log_dbg("TCP: [0x%x] [%u] ", pkg.seq, pkg.tcp_payload_len);

        if (check_and_callback(pkg)) {
            check_and_callback_tcppkg_in_cache();
        }
        else {
            add_tcppkg_to_cache(pkg);
        }
    }
}

bool ParseTCPLayer::is_sync_pkg(const tcp_hdr *hd) const {
    return hd->flag_syn == 1;
}

bool ParseTCPLayer::is_fin_pkg(const tcp_hdr *hd) const {
    return hd->flag_fin == 1;
}

bool ParseTCPLayer::is_rst_pkg(const tcp_hdr *hd) const {
    return hd->flag_rst == 1;
}

// if tcppkg out of order, return false;
// else return true, and callback function.
bool ParseTCPLayer::check_and_callback(const tcppkgq &pkg) {
    // 收到的第一个包
    if (num_of_recved_pkgs_ == 1) {
        data_ready_cbfunc_(pkg.tcp_payload, pkg.tcp_payload_len);
        next_tcp_seq_    = pkg.seq + pkg.tcp_payload_len;
        return true;
    }

    // 正常接收
    if (seq_equal(pkg.seq, next_tcp_seq_)) {
        data_ready_cbfunc_(pkg.tcp_payload, pkg.tcp_payload_len);
        next_tcp_seq_ += pkg.tcp_payload_len;
        return true;
    }

    // 重传包(包里面的数据，也可能部分有效果)
    if (seq_before(pkg.seq, next_tcp_seq_)) {
        log_dbg("recv_tcp_seq lower than expected(retransmit). ");
        if (seq_after(pkg.seq + pkg.tcp_payload_len, next_tcp_seq_)) {
            log_dbg("but part of data is new.");
            const uint32_t idx = next_tcp_seq_ - pkg.seq;
            next_tcp_seq_ += (pkg.tcp_payload_len - idx);

            data_ready_cbfunc_(pkg.tcp_payload + idx, pkg.tcp_payload_len - idx);
        }
        return true;
    }

    // 收到的包seq大于期望的包(说明中间有丢包)，需要将该包存入缓存
    return false;
}

void ParseTCPLayer::add_tcppkg_to_cache(const tcppkgq &pkg) {
    log_dbg("add_cache[0x%x,0x%x] ", pkg.seq, pkg.tcp_payload_len);
    if (pkgs_cache_list_.empty()) {
        pkgs_cache_list_.emplace_back(pkg);
        return;
    }

    // 从后往前找
    std::list<tcppkgq>::reverse_iterator rit = pkgs_cache_list_.rbegin();
    const uint64_t next_seq = rit->seq + rit->tcp_payload_len;
    if (next_seq <= pkg.seq) {
        pkgs_cache_list_.emplace(rit.base(), pkg);
        return;
    }

    //
    for (++rit; rit != pkgs_cache_list_.rend(); ++rit) {
        if (seq_equal(rit->seq, pkg.seq)) {
            if (seq_before(rit->tcp_payload_len, pkg.tcp_payload_len)) {
                rit->tcp_payload     = pkg.tcp_payload;
                rit->tcp_payload_len = pkg.tcp_payload_len;
                return;
            }
        }
        else if (seq_before(rit->seq, pkg.seq)) {
            pkgs_cache_list_.emplace(rit.base(), pkg);
            return;
        }
    } // while

    pkgs_cache_list_.emplace_front(pkg);
}

void ParseTCPLayer::check_and_callback_tcppkg_in_cache() {
    std::list<tcppkgq>::iterator it = pkgs_cache_list_.begin();
    while (it != pkgs_cache_list_.end()) {
        if (check_and_callback(*it)) {
            log_dbg("del_cache[0x%x, 0x%x] ", it->seq, it->tcp_payload_len);
            it = pkgs_cache_list_.erase(it);
        }
        else {
            break;
        }
    }
}

uint32_t ParseUDPLayer::parse(const timeval *, const ipfragment &frag) {
    assert(frag.offset == 0);

    log_dbg("UDP: [%u] ", frag.ip_payload_len - sizeof(udp_hdr));
    return 0;
}

void ParseUDPLayer::parse(const timeval *, const std::vector<ipfragment> &frags) {
    std::vector<ipfragment>::const_iterator it = frags.begin();
    log_dbg("UDP: [%u] ", it->ip_payload_len - sizeof(udp_hdr));
    for (++it; it != frags.end(); ++it) {
        fprintf(stdout, " [%u] ", it->ip_payload_len);
    }
}
