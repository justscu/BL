#pragma once

#include <stdint.h>

// single-read, single-write buffer.
class SrSwBuffer {
public:
    struct Cell {
        const char *str = nullptr;
    };

public:
    bool init();
    void unInit();

    void reset() { wpos_ = 0; vec_widx_ = vec_ridx_ = 0; }

    bool write(const char *str, const int32_t len);
    bool read(const char * &str);

private:
    int32_t      wpos_ = 0;
    char         *buf_ = nullptr; // save raw data.
    const int32_t buf_sent_size =   4*1024*1024; // sentry size
    const int32_t buf_size      = 256*1024*1024;

private:
    volatile uint64_t vec_widx_ = 0;
    volatile uint64_t vec_ridx_ = 0;
    Cell             *vec_ = nullptr;
    const uint64_t    vec_size = 128*1024;

private:
    uint64_t total_wt_len_ = 0; // write
    uint64_t total_rd_len_ = 0; // read
};
