#pragma once

#include "utils_net_packet_hdr.h"


// make UDP packet.
// 不支持分片
class MakeUdpPkt {
public:
    MakeUdpPkt() = default;

public:
    // 初始化mac头
    void init_mac_hdr(char *pkt, const char *smac);

    // 初始化 ip 部分头
    bool init_ip_hdr_partial(char *pkt, const char *sip, const char *dip);

    // 初始化 udp 部分头部
    void init_udp_hdr_partial(char *pkt, uint16_t sport, uint16_t dport);

    inline constexpr int32_t udp_payload_offset() const {
        return sizeof(mac_hdr) + sizeof(ip_hdr) + sizeof(udp_hdr);
    }

    // 返回数据包总长度
    // udp_payload_len: UDP载荷长度
    // ip_id: ip的16位标识
    int32_t set_hdr_finish(char *pkt, int32_t udp_payload_len, uint16_t ip_id);

    const char *last_err() { return last_err_; }

private:
    char last_err_[256] = {0};
};

class DecodeUdpPkt {
public:
    void decode(const char *pkt, int32_t pkt_len);
};
