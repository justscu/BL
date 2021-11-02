#pragma once

#include <list>
#include "parse_l2_layer.h"

// tcp-pkg-queue
struct tcppkgq {
    uint32_t    seq;          // 本tcp包的开始序号
    uint32_t    tcp_data_len; // tcp载荷长度
    const char *tcp_data;
};

class ParseL3LayerBase {
public:
    virtual ~ParseL3LayerBase() {}

    void set_filter_src_port(uint16_t src_port) { filter_src_port_ = src_port; }
    bool need_parse(const char *l3_str) const;

    virtual uint32_t parse(const ipfragment &frag) = 0;
    virtual void parse(const std::vector<ipfragment> &frags) = 0;

private:
    uint16_t filter_src_port_ = 0;
};

class ParseTCPLayer : public ParseL3LayerBase {
public:
    virtual ~ParseTCPLayer() { }
    // 独立IP包适用
    // TCP时，返回下一个tcp包开始的序号
    virtual uint32_t parse(const ipfragment &frag) override;
    // 分片IP包适用
    virtual void parse(const std::vector<ipfragment> &frags) override;

private:
    bool is_sync_pkg(const tcp_hdr *hd) const;
    bool is_fin_pkg(const tcp_hdr *hd) const;
    bool is_reset_pkg(const tcp_hdr *hd) const;

    void insert_new_tcppkg(const tcppkgq &pkg);
    void print_tcp_pkgs() const;

private:
    std::list<tcppkgq> pkgs_list_; // 按照tcp_sequence顺序，存放TCP流
};

class ParseUDPLayer : public ParseL3LayerBase {
public:
    // 独立IP包适用
    virtual uint32_t parse(const ipfragment &frag) override;
    // 分片IP包适用
    virtual void parse(const std::vector<ipfragment> &frags) override;
};
