#pragma once

#include "utils_net_packet_hdr.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// make UDP packet.
// 不支持分片
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class MakeUdpPkt {
public:
    MakeUdpPkt() = default;
    virtual ~MakeUdpPkt() { }

public:
    // 初始化部分"mac|ip"头
    // pkt  : 数据包的开始，含 "mac|ip|udp"
    // smac : 源mac
    // sip/dip: 源和目的 ip
    virtual bool init_hdr_partial(char *pkt, const char *smac, const char *sip, const char *dip);
    void set_udp_hdr(char *pkt, uint16_t sport, uint16_t dport);
    int32_t finish_hdr(char *pkt, uint16_t ip_identifier, uint16_t udp_payload_len);

    inline int32_t udp_payload_offset() const {
        return sizeof(mac_hdr) + sizeof(ip_hdr) + sizeof(udp_hdr);
    }

    const char *last_err() { return last_err_; }

protected:
    char last_err_[256] = {0};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// make multicast packet.
// 不支持分片
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class MakeMCastPkt : public MakeUdpPkt {
public:
    MakeMCastPkt() = default;

    // 初始化部分"mac|ip"头
    // pkt  : 数据包的开始，含 "mac|ip|udp"
    // smac : 源mac
    // sip/dip: 源和目的 ip
    virtual bool init_hdr_partial(char *pkt, const char *smac, const char *sip, const char *dip) override;
};


class DecodeUdpPkt {
public:
    void decode(const char *pkt, int32_t pkt_len);
};
