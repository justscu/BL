#pragma once

#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// send udp packet, calc delay-cost.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UdpPG {
public:
    void udp_pong(uint16_t port);
    void send_udp(const char *dip, uint16_t dport, int32_t pkt_len);
    void recv_udp(uint16_t port);

private:
    volatile int16_t state_ = 0; // 1, warm up; 2, testing; 3, over
};
