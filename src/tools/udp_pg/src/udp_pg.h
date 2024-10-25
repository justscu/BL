#pragma once

#include <stdint.h>

#define SEND_CNT 1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// send udp packet, calc delay-cost.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UdpPG {
public:
    void udp_pong(uint16_t port);
    void send_udp(const char *lip, uint16_t lport, const char *dip, uint16_t dport, int32_t pkt_len);
    void recv_udp(uint16_t port);
};
