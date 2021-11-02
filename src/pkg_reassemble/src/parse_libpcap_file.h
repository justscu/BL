#pragma once

#include "parse_l1_layer.h"
#include "parse_l2_layer.h"

// pcap文件的数据格式，使用该格式进行解析，不需要使用libpcap库.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | PcapFileHeader| Pkg Hdr1 | Pkg Data1 | Pkg Hdr2 | Pkg Data2 | ... |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//

#define PCAP_HDR_MAGIC 0xA1B2C3D4
// libPcap-Header
struct pcap_hdr_t {
    int32_t  magic; // 标识位，always 0xa1b2c3d4, 可以判断文件的字节序。若为0xd4c3b2a1，则需要字节反转.
    uint16_t version_major; // 主版本, 0x02
    uint16_t version_minor; // 副版本, 0x04
    int32_t  this_zone;     // 区域时间，未使用, always 0.
    int32_t  sig_figs;      // 时间精度，未使用, always 0.
    int32_t  snap_len;      // 最大抓包长度
    int32_t  link_layer_type; // 数据链路层类型
};

struct PcapPkgHdr {
    int32_t   gmt_time_sec;  // 从1970.1.1开始的秒数
    int32_t micro_time_usec; // 微秒值
    int32_t         cap_len; // 在pcap文件中的长度
    int32_t         pkg_len; // 实际数据包长度，可能大于cap_len.
};

class ParseLibpcapFile {
public:
    bool init(const char *src_ip, const char *dst_ip,
              uint16_t src_port, uint16_t dst_port,
              const char *protocol);

    void read_file(const char *fname);
    int32_t get_pcap_package(const char *str, const int32_t len);
    int32_t get_pcap_file_header(const char *str, int32_t len);
    void parse(const PcapPkgHdr *hdr, const char *pkg);

private:
    pcap_hdr_t pcap_file_head_; // 文件头信息

    char         *buf_ = nullptr;
    const int32_t buf_size = 4*1024*1024; // 4M
    int32_t       idx_ = 0;

private:
    ParseIPLayer   *ip_parser_ = nullptr;
    ParseEthLayer *mac_parser_ = nullptr;
};
