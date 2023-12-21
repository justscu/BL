#pragma once

#include <stdint.h>
#include <sys/time.h>

// x86采用小端字节序
// 网络采用大端字节序
// LITTLE_ENDIAN, BIG_ENDIAN,
#define USED_BYTE_ORDER LITTLE_ENDIAN //主机字节序

#pragma pack(push, 1)

// pcap文件的数据格式，使用该格式进行解析，不需要使用libpcap库.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | PcapFileHeader| Pkg Hdr1 | Pkg Data1 | Pkg Hdr2 | Pkg Data2 | ... |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//

#define PCAP_HEAD_MAGIC 0xA1B2C3D4
// libPcap-Header
// sizeof(pcap_hdr_t) = 24.
struct pcap_hdr {
    uint32_t magic; // 标识位，always 0xa1b2c3d4, 可以判断文件的字节序。若为0xd4c3b2a1，则需要字节反转.
    uint16_t version_major; // 主版本, 0x02
    uint16_t version_minor; // 副版本, 0x04
    int32_t  this_zone;     // 区域时间，未使用, always 0.
    int32_t  sig_figs;      // 时间精度，未使用, always 0.
    int32_t  snap_len;      // 最大抓包长度
    int32_t  link_layer_type; // 数据链路层类型
};

//struct  ts
struct capture_hdr {
    uint32_t tm_sec;  // capture time
    uint32_t tm_usec; // capture time
    uint32_t cap_len; // 在pcap文件中的长度
    uint32_t pkg_len; // 实际数据包长度，可能大于cap_len.
};

/////////////////////////////////////////////////////////////////
#define PROTOCOL_IP   0x0008
#define PROTOCOL_ARP  0x0608
#define PROTOCOL_IPV6 0xdd86
// Eth-Header
//
// 0                   1                   2
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | DstAddr   | SrcAddr   |type| ... data ... | FCS |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// type, 2字节, 上一层协议类型: IP, type=0x0800; ARP, type=0x0806;
struct mac_hdr {
    uint8_t  dstaddr[6];
    uint8_t  srcaddr[6];
    uint16_t      proto; // IP, 0x0800; ARP, 0x0806.
};

// 4字节
struct mac_frame_tail {
    uint32_t checksum; // tail checksum.
};


/////////////////////////////////////////////////////////////////
#define PROTOCOL_TCP   0x06
#define PROTOCOL_UDP   0x11

// IP header
struct ip_hdr {
#if (USED_BYTE_ORDER == LITTLE_ENDIAN)
    uint8_t     ihl:4; // 首部长度 X4
    uint8_t version:4; // 版本
#elif (USED_BYTE_ORDER == BIG_ENDIAN)
    uint8_t version:4; // 版本
    uint8_t     ihl:4; // 首部长度 X4
#else
#error "Endian is not LE nor BE ..."
#endif

    uint8_t          tos; // 服务类型
    uint16_t   total_len; // 本IP报文长度
    uint16_t          id; // 标识
    uint16_t frag_offset; // 片偏移(需*8)
    uint8_t          ttl;
    uint8_t     protocol; // 协议类型
    uint16_t     chk_sum; // 首部校验和
    union {
        struct {
            uint32_t  src_ip; // 目的IP
            uint32_t  dst_ip; // 源IP
        };
        uint64_t ip;
    };
    // ... options ...
};

/////////////////////////////////////////////////////////////////
// TCP header
struct tcp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t   seq_no; // 本次发送的开始序号
    uint32_t   ack_no; // 确认序号
#if (USED_BYTE_ORDER == LITTLE_ENDIAN)
    uint8_t  reserved:4;
    uint8_t       thl:4; // tcp header length
    uint8_t  flag_fin:1; // 8 bit标志
    uint8_t  flag_syn:1;
    uint8_t  flag_rst:1;
    uint8_t  flag_psh:1;
    uint8_t  flag_ack:1;
    uint8_t  flag_urg:1;
    uint8_t  flag_ece:1;
    uint8_t  flag_cwr:1;
#elif (USED_BYTE_ORDER == BIG_ENDIAN)
    uint8_t       thl:4; // tcp header length
    uint8_t  reserved:4;
    uint8_t  flag_cwr:1; // 8 bit标志
    uint8_t  flag_ece:1;
    uint8_t  flag_urg:1;
    uint8_t  flag_ack:1;
    uint8_t  flag_psh:1;
    uint8_t  flag_rst:1;
    uint8_t  flag_syn:1;
    uint8_t  flag_fin:1;
#else
#error  "Endian is not LE nor BE..."
#endif
    uint16_t window_size;
    uint16_t chk_sum; // TCP check sum
    uint16_t  urgt_p; // 紧急指针
    // ... options ...
};


/////////////////////////////////////////////////////////////////
struct udp_hdr {
    uint16_t   src_port;
    uint16_t   dst_port;
    uint16_t     length; // UDP数据报长度(包含头,最小为8)
    uint16_t    chk_sum; // UDP校验和
};

#pragma pack(pop)
