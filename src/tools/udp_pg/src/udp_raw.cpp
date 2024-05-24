#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "fmt/format.h"
#include "commx/utils.h"
#include "udp_raw.h"


void send_udp_packet_by_raw(const char *eth_name,
                            const char *dip,
                            uint16_t dport)
{
    char    sip[16];
    uint8_t smac[16];
    int32_t idx = 0;

    Nic nic;
    if (!nic.get_ip(eth_name, sip) || !nic.get_mac(eth_name, smac) || !nic.get_index(eth_name, idx)) {
        fmt::print("{} \n", nic.err());
        return;
    }

    // make raw udp packet.
    char buf[2048];
    MakeUdpPkt udp;
    udp.init_mac_hdr(buf, (const char*)smac);
    udp.init_ip_hdr_partial(buf, sip, dip);
    udp.init_udp_hdr_partial(buf, 11223, dport);

    int32_t sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sockfd == -1) {
        fmt::print("socket failed. {} \n", strerror(errno));
        return;
    }

    struct sockaddr_ll dst;
    memset(&dst, 0, sizeof(sockaddr_ll));
    dst.sll_ifindex = idx;
    dst.sll_pkttype = PACKET_OTHERHOST;

    for (uint32_t i = 0; true; ++i) {
        int32_t rawlen = udp.set_hdr_finish(buf, 128, i);
        int32_t slen = ::sendto(sockfd, buf, rawlen, 0, (struct sockaddr*)&dst, sizeof(struct sockaddr_ll));
        fmt::print("sendto return {}, {} \n", slen, strerror(errno));
        sleep(1);
    }
}
