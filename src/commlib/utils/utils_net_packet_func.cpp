#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "utils_net_packet_func.h"


# define   likely(t) __builtin_expect(!!(t), 1)
# define unlikely(t) __builtin_expect((t), 0)


/*
 * 1. ip包头(共20个字节)按照每16个bit作为一个值依次进行相加 
 * 2. 将计算结果的进位加到低16位上 
 * 3. 将结果取反 
 */
static
uint16_t ip_hdr_checksum(const ip_hdr *hdr) {
    uint32_t csum = 0;

    const uint16_t *p = (const uint16_t*)hdr;
    csum  = p[0];
    csum += p[1];
    csum += p[2];
    csum += p[3];
    csum += p[4];
    // csum += p[5]; // chk_sum
    csum += p[6];
    csum += p[7];
    csum += p[8];
    csum += p[9];

    // if ip header more than 20.
    uint32_t hdrlen = hdr->ihl * 4;
    if (unlikely(hdrlen > 20)) {
        p += 10;
        hdrlen -= 20;
        for (uint32_t i = 0; i < hdrlen/2; ++i) {
            csum += p[i];
        }
    }

    csum =  (csum >> 16u) + (csum & 0xffff);
    csum += (csum >> 16u);

    return (~csum) & 0xffff;
}

// UDP、TCP校验和：首部+数据+12个字节伪首部(源IP地址、目的IP地址、协议、TCP/UDP包长)
static
uint16_t udp_hdr_checksum(const ip_hdr *ip, const udp_hdr *udp) {
    // 亚头部:
    //      4 byte源ip地址 + 4 byte目的ip地址 + 1 byte协议 + UDP 长度(2byte)(udp包头长度+数据长度)
    uint32_t sum = (ip->src_ip & 0xFFFF) + (ip->src_ip >> 16)
                 + (ip->dst_ip & 0xFFFF) + (ip->dst_ip >> 16)
                 + ntohs(IPPROTO_UDP)
                 + (uint16_t)(udp->length);

    // UDP包头: 2 byte源端口 + 2 byte目的端口 + 2 byte UDP包长(此处是udp包头自带的值不用变) + 0x0000 (checksum)
    sum += (uint16_t)(udp->src_port)
         + (uint16_t)(udp->dst_port)
         + (uint16_t)(udp->length);

    //
    const uint16_t *p = (const uint16_t*)(((const char*)udp) + sizeof(udp_hdr));
    uint16_t len = (ntohs(udp->length) - sizeof(udp_hdr))/2;
    for (uint32_t i = 0; i < len; ++i) {
        sum += p[i];
    }

    if ((ntohs(udp->length) - sizeof(udp_hdr)) % 2 == 1) {
        sum += (p[len] >> 8);
    }

    sum = ((sum >> 16) & 0xFFFF) + (sum & 0xFFFF);
    sum += (sum >> 16);
    sum = (~sum) & 0xffff;
    return sum;
}

uint16_t checksum(const char *str, int32_t len) {
    uint32_t sum = 0;

    for (int32_t i = 0; i < len-1; i += 2) {
        sum += *(uint16_t*)(str+i);
    }

    if ((len & 1) == 1) {
        sum += *(uint8_t*)(str+len-1);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (~sum) & 0xFFFF;
}

uint32_t adler32(uint8_t *buf, int32_t len) {
    const uint32_t base = 65521;
    uint32_t adler = 1;
    uint32_t s1 = adler & 0xffff;
    uint32_t s2 = (adler >> 16) & 0xffff;
    for (int32_t i = 0; i < len; i++) {
        s1 = (s1 + buf[i]) % base;
        s2 = (s2 + s1) % base;
    }
    return (s2 << 16) + s1;
}



bool MakeUdpPkt::init_hdr_partial(char *pkt, const char *smac, const char *sip, const char *dip) {
    // ip 部分信息
    ip_hdr *ip = (ip_hdr*)(pkt+sizeof(mac_hdr));
    {
        ip->ihl = (sizeof(ip_hdr) >> 2); // *4, fix length.
        ip->version = 4;
        ip->tos = 0;
        // ip->total_len = 0;
        // ip->id = 0;
        ip->frag_offset = 0x40; // no offset
        ip->ttl = 64;
        ip->protocol = PROTOCOL_UDP;
        ip->chk_sum = 0;

        if (1 != inet_pton(AF_INET, sip, &ip->src_ip)) {
            snprintf(last_err_, sizeof(last_err_)-1, "inet_pton sip[%s] failed. %s.", sip, strerror(errno));
            return false;
        }
        if (1 != inet_pton(AF_INET, dip, &ip->dst_ip)) {
            snprintf(last_err_, sizeof(last_err_)-1, "inet_pton dip[%s] failed. %s.", dip, strerror(errno));
            return false;
        }
    }

    // mac 部分信息
    mac_hdr *mac = (mac_hdr*)pkt;
    {
        mac->proto = PROTOCOL_IP;
        memcpy(mac->srcaddr, smac, 6);

        mac->dstaddr[0] = 0x01;
        mac->dstaddr[1] = 0x00;
        mac->dstaddr[2] = 0x5e;
        mac->dstaddr[3] = smac[3] & 0x7F;
        mac->dstaddr[4] = smac[4] & 0xFF;
        mac->dstaddr[5] = smac[5] & 0xFF;
    }

    return true;
}

// 初始化 udp 部分头部
void MakeUdpPkt::set_udp_hdr(char *pkt, uint16_t sport, uint16_t dport) {
    udp_hdr *udp = (udp_hdr*)(pkt+sizeof(mac_hdr)+sizeof(ip_hdr));
    udp->src_port = htons(sport);
    udp->dst_port = htons(dport);
    udp->length   = 0;
    udp->chk_sum  = 0; // 不计算校验和
}

// 返回数据包总长度
// udp_payload_len: UDP载荷长度
// ip_id: ip的16位标识
int32_t MakeUdpPkt::finish_hdr(char *pkt, uint16_t ip_identifier, uint16_t udp_payload_len) {
    assert(udp_payload_len + udp_payload_offset() < 1500);

    ip_hdr *ip = (ip_hdr*)(pkt + sizeof(mac_hdr));
    {
        ip->total_len = ntohs(sizeof(ip_hdr) + sizeof(udp_hdr) + udp_payload_len);
        ip->id        = htons(ip_identifier);
        ip->chk_sum   = ip_hdr_checksum((const ip_hdr*)pkt); // must set this value, when use X3.
    }

    udp_hdr *udp = (udp_hdr*)(pkt + sizeof(mac_hdr) + sizeof(ip_hdr));
    {
        udp->length  = ntohs(sizeof(udp_hdr) + udp_payload_len);
        // udp->chk_sum = udp_hdr_checksum(ip, udp);
    }

    return udp_payload_offset() + udp_payload_len;
}

// 初始化部分"mac|ip"头
bool MakeMCastPkt::init_hdr_partial(char *pkt, const char* smac, const char *sip, const char *dip) {
    // ip 部分信息
    ip_hdr *ip = (ip_hdr*)(pkt + sizeof(mac_hdr));
    {
        ip->ihl = (sizeof(ip_hdr) >> 2); // *4, fix length.
        ip->version = 4;
        ip->tos = 0;
        // ip->total_len = 0;
        // ip->id = 0;
        ip->frag_offset = 0x40; // no offset
        ip->ttl = 64;
        ip->protocol = PROTOCOL_UDP;

        if (1 != inet_pton(AF_INET, sip, &ip->src_ip)) {
            snprintf(last_err_, sizeof(last_err_)-1, "inet_pton sip[%s] failed. %s.", sip, strerror(errno));
            return false;
        }
        if (1 != inet_pton(AF_INET, dip, &ip->dst_ip)) {
            snprintf(last_err_, sizeof(last_err_)-1, "inet_pton dip[%s] failed. %s.", dip, strerror(errno));
            return false;
        }

        ip->chk_sum = 0;
    }

    // mac 部分信息
    mac_hdr *mac = (mac_hdr*)pkt;
    {
        mac->proto = PROTOCOL_IP;
        memcpy(mac->srcaddr, smac, 6);

        const uint8_t *addr = (uint8_t*)(&ip->dst_ip);

        mac->dstaddr[0] = 0x01;
        mac->dstaddr[1] = 0x00;
        mac->dstaddr[2] = 0x5e;
        mac->dstaddr[3] = addr[1] & 0x7F;
        mac->dstaddr[4] = addr[2];
        mac->dstaddr[5] = addr[3];
    }

    return true;
}


void DecodeUdpPkt::decode(const char *pkt, int32_t pkt_len) {
    const mac_hdr &mac = *(const mac_hdr*)pkt;
    const ip_hdr &ip = *(const ip_hdr*)(pkt+sizeof(mac_hdr));
    const udp_hdr &udp = *(const udp_hdr*)(pkt+sizeof(mac_hdr) + ip.ihl*4);
    fprintf(stdout, "%.2x \n", mac.proto);
    fprintf(stdout, "%d \n", ip.id);
    fprintf(stdout, "%d. \n", udp.length);

    uint16_t ipsum = ip_hdr_checksum(&ip);
    fprintf(stdout, "%x. %x. \n", ipsum, ip.chk_sum);

    uint32_t udpsum = udp_hdr_checksum(&ip, &udp);
    fprintf(stdout, "%u, %u. \n", udpsum, udp.chk_sum);

}
