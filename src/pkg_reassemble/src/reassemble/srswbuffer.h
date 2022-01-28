#pragma once

#include <stdint.h>
#include "headers.h"
#include "utils.h"

class SrSwBuffer {
public:
    struct Cell {
        cap_hdr  hd;
        const char *str = nullptr; // without pcap header.
    };

public:
    SrSwBuffer() {}
    ~SrSwBuffer() {}
    SrSwBuffer(const SrSwBuffer&) = delete;
    SrSwBuffer& operator=(const SrSwBuffer&) = delete;

    bool init();
    void unInit();
    void reset();

    // str: 去掉pcap头的数据
    bool write(const cap_hdr *hd, const char *str);
    bool read(cap_hdr *hd, const char* &str);

private:
    // cycle buffer: saving raw data.
    int32_t      wpos_ = 0;
    char         *buf_ = nullptr; // save raw data.
    const int32_t buf_sent_size =   4*1024*1024; // sentry size, 4K.
    const int32_t buf_size      = 256*1024*1024;

private:
    SPSCQueue      que_;
    const uint32_t que_size = buf_size / PKG_AVG_SIZE;

private:
    uint64_t total_wt_len_ = 0; // write
    uint64_t total_rd_len_ = 0; // read
};
