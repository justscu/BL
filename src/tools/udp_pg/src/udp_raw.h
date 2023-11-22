#pragma once

#include <stdint.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 通过raw socket发udp包
// 通过tcpdump，能在发包的机器上抓到包
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void send_udp_packet_by_raw(const char *eth_name,
                            const char *dip,
                            uint16_t dport);
