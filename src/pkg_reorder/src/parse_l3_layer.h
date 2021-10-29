#pragma once

#include <list>
#include "parse_l2_layer.h"

// tcp-pkg-queue
struct tcppkgq {
    uint32_t    seq;          // 本tcp包的开始序号
    uint32_t    tcp_data_len; // tcp载荷长度
    const char *tcp_data;
};

class ParseTCPLayer {
public:
    virtual ~ParseTCPLayer() {}
    // 独立IP包适用
    // TCP时，返回下一个tcp包开始的序号
    uint32_t parse(const ipfragment &frag);
    // 分片IP包适用
    void parse(const std::vector<ipfragment> &frags);

private:
    void insert_new_tcppkg(const tcppkgq &pkg);

private:
    uint16_t src_port_ = 0;
    uint16_t dst_port_ = 0;
    std::list<tcppkgq> pkgs_list_; // 存放TCP流
};

class ParseUDPLayer {
public:
    // 独立IP包适用
    void parse(const ipfragment &frag);
    // 分片IP包适用
    void parse(const std::vector<ipfragment> &frags);
};
