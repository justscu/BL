#include <assert.h>
#include "headers.h"
#include "parse_l2_layer.h"
#include "parse_l3_layer.h"

bool ParseL3LayerBase::need_parse(const char *l3_str) const {
    return filter_src_port_ == *(uint16_t*)l3_str;
}

uint32_t ParseTCPLayer::parse(const ipfragment &frag) {
    assert(frag.offset == 0);

    // TCP流，包含头部
    const char *tcp_flow = frag.addr;
    const tcp_hdr *hd = (const tcp_hdr*)tcp_flow;

    tcppkgq pkg;
    pkg.seq = ntohl(hd->seq_no);
    pkg.tcp_data_len = frag.len - hd->thl * 4;
    pkg.tcp_data = frag.addr + hd->thl * 4;
    if (pkg.tcp_data_len > 0) {
        insert_new_tcppkg(pkg);
    }

    return pkg.seq + pkg.tcp_data_len;
}

void ParseTCPLayer::parse(const std::vector<ipfragment> &frags) {
    std::vector<ipfragment>::const_iterator it = frags.begin();
    uint32_t next_seq = parse(*it);
    ++it;

    for (; it != frags.end(); ++it) {
        tcppkgq pkg;
        pkg.seq = next_seq;
        pkg.tcp_data_len = it->len;
        pkg.tcp_data = it->addr;
        next_seq += it->len;

        insert_new_tcppkg(pkg);
    }
}

bool ParseTCPLayer::is_sync_pkg(const tcp_hdr *hd) const {
    return hd->flag & 0x020;
}

bool ParseTCPLayer::is_fin_pkg(const tcp_hdr *hd) const {
    return hd->flag & 0x01;
}

bool ParseTCPLayer::is_reset_pkg(const tcp_hdr *hd) const {
    return hd->flag & 0x40;
}

// list的顺序为tcp sequence的顺序
void ParseTCPLayer::insert_new_tcppkg(const tcppkgq &pkg) {
    fprintf(stdout, "TCP: [0x%x] [%u] ", pkg.seq, pkg.tcp_data_len);

    if (pkgs_list_.empty()) {
        pkgs_list_.emplace_back(pkg);
        return;
    }
    // 从后往前找
    std::list<tcppkgq>::reverse_iterator rit = pkgs_list_.rbegin();
    uint64_t next_seq = rit->seq + rit->tcp_data_len;
    for (; rit != pkgs_list_.rend(); ++rit) {
        if (next_seq <= pkg.seq) {
            pkgs_list_.emplace(rit.base(), pkg);
            print_tcp_pkgs();
            return;
        }
        next_seq = rit->seq + rit->tcp_data_len;
    }

    pkgs_list_.emplace_front(pkg);
    print_tcp_pkgs();
}

void ParseTCPLayer::print_tcp_pkgs() const {
    std::list<tcppkgq>::const_iterator it = pkgs_list_.cbegin();
    uint32_t pre_seq = it->seq;
    uint32_t pre_len = it->tcp_data_len;
    ++it;

    for (; it != pkgs_list_.end(); ++it) {
        if (it->seq == pre_seq + pre_len) {
            pre_seq = it->seq;
            pre_len = it->tcp_data_len;
        }
        else {
            fprintf(stdout, " duplicate[%u, %u] ", it->seq, it->tcp_data_len);
        }
    }
}

uint32_t ParseUDPLayer::parse(const ipfragment &frag) {
    assert(frag.offset == 0);

    fprintf(stdout, "UDP: [%u] ", frag.len - sizeof(udp_hdr));
    return 0;
}

void ParseUDPLayer::parse(const std::vector<ipfragment> &frags) {
    std::vector<ipfragment>::const_iterator it = frags.begin();
    fprintf(stdout, "UDP: [%u] ", it->len - sizeof(udp_hdr));
    ++it;
    for (; it != frags.end(); ++it) {
        fprintf(stdout, " [%u] ", it->len);
    }
}
