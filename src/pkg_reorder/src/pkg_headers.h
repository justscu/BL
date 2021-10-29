#pragma once

#include <stdint.h>

// Mac-Header
//
// 0                   1                   2
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | DstAddr   | SrcAddr   |type| ... data ... | FCS |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// type, 2字节, 上一层协议类型; type=0x0800, IP; type=0x0806, ARP;

struct EthHdr {
    uint8_t  dstaddr[6];
    uint8_t  srcaddr[6];
    uint16_t proto; // 0x0800, IP 协议; 0x0806, ARP协议
};

// 4字节
struct MacFrameTail {
    uint32_t checksum; // 尾部校验
};

// IP header
struct IpHdr {
#if LITTLE_ENDIAN
    uint8_t     ihl:4; // 首部长度
    uint8_t version:4; // 版本
#else
    uint8_t version:4; // 版本
    uint8_t     ihl:4; // 首部长度
#endif
    uint8_t          tos; // 服务类型
    uint16_t   total_len; // 本IP报文长度
    uint16_t          id; // 标识
    uint16_t frag_offset; // 片偏移(需*8)
    uint8_t      ttl;
    uint8_t protocol; // 协议类型
    uint16_t chk_sum; // 首部校验和
    union {
        struct {
            uint32_t  src_ip; // 目的IP
            uint32_t  dst_ip; // 源IP
        };
        uint64_t ip;
    };
};

#define PROTOCOL_TCP  6
#define PROTOCOL_UDP 17

// TCP header
struct tcp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t   seq_no; // 本次发送的开始序号
    uint32_t   ack_no; // 确认序号
#if LITTLE_ENDIAN
    uint8_t reserved1:4;
    uint8_t       thl:4; // tcp header length
    uint8_t        flag; // 8 bit标志
#else
    uint8_t reserved1:4;
    uint8_t       thl:4; // tcp header length
    uint8_t        flag; // 8 bit标志
#endif
    uint16_t window_size;
    uint16_t chk_sum; // TCP check sum
    uint16_t  urgt_p; // 紧急指针
};

struct udp_hdr {
    uint16_t   src_port;
    uint16_t   dst_port;
    uint16_t     length; // UDP数据报长度(包含头,最小为8)
    uint16_t    chk_sum; // UDP校验和
};

#pragma pack()

