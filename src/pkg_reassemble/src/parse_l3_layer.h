#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include "parse_l2_layer.h"

// tcp-pkg-queue
struct tcppkgq {
    uint32_t    seq;          // 本tcp包的开始序号
    uint32_t    tcp_payload_len; // tcp载荷长度(去掉tcp头)
    const char *tcp_payload;     // tcp载荷(去掉tcp头)
};

using L3DataReadyCBFunc = void(*)(const char *src, const int32_t len);

static void tcp_data_ready_dbfunc(const char *src, const int32_t len) {
    std::ofstream ofs;
    ofs.open("/tmp/tcp_rb.txt", std::ios::app | std::ios::binary);
    ofs.write(src, len);
    ofs.flush();
}

class ParseL3LayerBase {
public:
    ParseL3LayerBase() { tcp_data_ready_cbfunc_ = tcp_data_ready_dbfunc; }
    virtual ~ParseL3LayerBase() {}

    void set_filter_src_port(uint16_t src_port) { filter_src_port_ = src_port; }
    bool need_parse(const char *l3_str) const;

    virtual uint32_t parse(const ipfragment &frag, const captime *ct) = 0;
    virtual void parse(const std::vector<ipfragment> &frags, const captime *ct) = 0;

protected:
    L3DataReadyCBFunc tcp_data_ready_cbfunc_ = nullptr;
private:
    uint16_t filter_src_port_ = 0; // 按源端口进行过滤
};

class ParseTCPLayer : public ParseL3LayerBase {
public:
    virtual ~ParseTCPLayer() { }
    // 非分片的IP包适用
    // TCP时，返回下一个tcp包开始的序号
    virtual uint32_t parse(const ipfragment &frag, const captime *ct) override;
    // 分片IP包适用
    virtual void parse(const std::vector<ipfragment> &frags, const captime *ct) override;

private:
    bool is_sync_pkg(const tcp_hdr *hd) const;
    bool is_fin_pkg(const tcp_hdr *hd) const;
    bool is_rst_pkg(const tcp_hdr *hd) const;

    // if out of order, return false.
    bool check_and_callback(const tcppkgq &pkg);
    void add_tcppkg_to_cache(const tcppkgq &pkg);
    void check_and_callback_tcppkg_in_cache();

private:
    std::list<tcppkgq> pkgs_cache_list_; // 按照tcp_sequence顺序，存放TCP流

    uint64_t num_of_recved_pkgs_ = 0;
    uint32_t next_tcp_seq_ = 0; // 下一个tcp seq序号
    uint32_t  pre_ok_time_ = 0; // second, 上一次可分析的整数据包时间(capture time)
};

class ParseUDPLayer : public ParseL3LayerBase {
public:
    // 独立IP包适用
    virtual uint32_t parse(const ipfragment &frag, const captime *ct) override;
    // 分片IP包适用
    virtual void parse(const std::vector<ipfragment> &frags, const captime *ct) override;
};
