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


// 初始化mac头
void MakeUdpPkt::init_mac_hdr(char *pkt, const char *smac) {
    mac_hdr *hdr = (mac_hdr*)pkt;
    memcpy(hdr->srcaddr, smac, 6);

    hdr->dstaddr[0] = 0x01;
    hdr->dstaddr[1] = 0x00;
    hdr->dstaddr[2] = 0x5e;
    hdr->dstaddr[3] = smac[3] & 0x7F;
    hdr->dstaddr[4] = smac[4] & 0xFF;
    hdr->dstaddr[5] = smac[5] & 0xFF;

    hdr->proto = PROTOCOL_IP;
}

// 初始化 ip 部分头
bool MakeUdpPkt::init_ip_hdr_partial(char *pkt, const char *sip, const char *dip) {
    ip_hdr *hdr = (ip_hdr*)(pkt+sizeof(mac_hdr));

    hdr->ihl = (sizeof(ip_hdr) >> 2); // *4, fix length.
    hdr->version = 4;
    hdr->tos = 0;
    // hdr->total_len = 0;
    // hdr->id = 0;
    hdr->frag_offset = 0; // no offset
    hdr->ttl = 64;
    hdr->protocol = PROTOCOL_UDP;
    hdr->chk_sum = 0;

    if (1 != inet_pton(AF_INET, sip, &hdr->src_ip)) {
        snprintf(last_err_, sizeof(last_err_)-1, "inet_pton sip[%s] failed. %s.", sip, strerror(errno));
        return false;
    }
    if (1 != inet_pton(AF_INET, dip, &hdr->dst_ip)) {
        snprintf(last_err_, sizeof(last_err_)-1, "inet_pton dip[%s] failed. %s.", dip, strerror(errno));
        return false;
    }

    return true;
}

// 初始化 udp 部分头部
void MakeUdpPkt::init_udp_hdr_partial(char *pkt, uint16_t sport, uint16_t dport) {
    udp_hdr *hdr = (udp_hdr*)(pkt+sizeof(mac_hdr)+sizeof(ip_hdr));

    hdr->src_port = htons(sport);
    hdr->dst_port = htons(dport);
    // hdr->length;
    hdr->chk_sum = 0; // 不计算校验和
}

// 返回数据包总长度
// udp_payload_len: UDP载荷长度
// ip_id: ip的16位标识
int32_t MakeUdpPkt::set_hdr_finish(char *pkt, int32_t udp_payload_len, uint16_t ip_id) {
    assert(udp_payload_len + udp_payload_offset() < 1500);

    // IP
    ip_hdr *ip4 = (ip_hdr*)(pkt+sizeof(mac_hdr));
    ip4->total_len = htons(sizeof(ip_hdr) + sizeof(udp_hdr) + udp_payload_len);
    ip4->id        = htons(ip_id);
    ip4->chk_sum   = ip_hdr_checksum(ip4);

    // udp
    udp_hdr *udp = (udp_hdr*)(pkt+sizeof(mac_hdr)+sizeof(ip_hdr));
    udp->length  = htons(sizeof(udp_hdr) + udp_payload_len);
    udp->chk_sum = udp_hdr_checksum(ip4, udp);

    return udp_payload_offset() + udp_payload_len;
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
