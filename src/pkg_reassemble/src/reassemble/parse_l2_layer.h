#pragma once

#include <arpa/inet.h>
#include <unordered_map>
#include <vector>
#include "headers.h"

// IP分片信息
struct ipfragment {
    uint32_t  offset; // 在IP分片中的偏移
    uint32_t     ip_payload_len; // 本分片长度(去掉头部)
    const char *ip_payload_addr; // ip数据地址(去掉头部)
};

// ip完整数据包
struct ippkg {
    uint16_t identifier; // 16bit ip标识
    uint32_t src_ip;
    uint32_t dst_ip;
    uint32_t data_total_len; // IP数据总长度(去掉头部)
    bool recv_last_fragment; // 是否收到最后一个分片
    uint32_t recv_fragment_time; // 最后一次收到分片的时间(秒，计算超时用)

    std::vector<ipfragment> fragments;
};

// ip层监控数据
struct ipmonitor {
    uint32_t cnt_timeout   = 0; // 超时包个数
    uint32_t cnt_fragments = 0; // IP分片包的个数
};

class ParseL3LayerBase;
class ParseIPLayer {
public:
    void set_ip_filter(const char *src_ip, const char *dst_ip);
    void set_protocol_filter(const char *protocol);
    // 如果不需要过滤某个端口，可以设置为0
    void set_port_filter(uint16_t src_port, uint16_t dst_port);

    bool create_l3_layer();

    // str: IP层数据(含头部)
    // len: IP层数据长度
    void parse(const char *str, const int32_t len, const timeval *t);

private:
    bool need_parse(const char *str) const;

    // 若不是分片包，返回true
    bool is_not_fragment(const ip_hdr *hd) const {
        return (hd->frag_offset & 0xFF3F) == 0;
    }

    uint32_t ip_header_length(const ip_hdr *hd) const {
        return (hd->ihl) * 4;
    }
    uint32_t ip_pkg_length(const ip_hdr *hd) const {
        return ntohs(hd->total_len);
    }
    uint32_t frag_offset(const ip_hdr *hd) const {
        return ntohs(hd->frag_offset & 0xFF1F) * 8;
    }
    // 是否为最后一个分片
    bool is_last_fragment(const ip_hdr *hd) const {
        return !(hd->frag_offset & 0x0020);
    }

    uint16_t pkg_identifier(const ip_hdr *hd) const {
        return hd->id;
    }

    bool is_done(const ippkg &pkg) const;

    void print(const ip_hdr *hd) const;
    void print(const uint8_t *str, const int32_t len) const;

    // IP包去重
    // 若插入成功，返回true;
    // 无需插入，返回false.
    bool insert_new_fragment(std::vector<ipfragment> &vec, const ipfragment &frag);

private:
    uint64_t filte_ip_   = 0;
    uint16_t filte_src_port_ = 0;
    uint8_t  filte_protocol_ = 0;

    ipmonitor mon;

    // 只有需要分片的，才放入
    std::unordered_map<uint16_t, ippkg> ip_pkgs_;
    ParseL3LayerBase *l3_layer_ = nullptr;
};
