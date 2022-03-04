#include <new>
#include <string.h>
#include "log.h"
#include "srswbuffer.h"

bool SrSwBuffer::init() {
    if (!que_.init(sizeof(Cell), que_size)) {
        log_err("SPSCQueue init failed.");
        return false;
    }

    buf_ = new (std::nothrow) char[buf_sent_size + buf_size];
    if (!buf_) {
        log_err("SrSwBuffer new char[%d] faild", buf_sent_size + buf_size);
        return false;
    }

    reset();
    return true;
}

void SrSwBuffer::unInit() {
    if (buf_) {
        delete [] buf_;
        buf_ = nullptr;
    }
    que_.unInit();
}

void SrSwBuffer::reset() {
    wpos_ = 0;
    total_wt_len_ = total_rd_len_ = 0;
    que_.reset();
}

// str: 去掉pcap头的数据
void SrSwBuffer::write(const cap_hdr *hd, const char *str) {
    Cell *cell = nullptr;
    while ((cell = (Cell*)que_.alloc()) == nullptr) {
        cpu_delay(5);
    }

    // copy
    {
        memcpy(&(cell->hd), hd, sizeof(cell->hd));
        cell->str = buf_ + wpos_;
        //
        memcpy(buf_+wpos_, str, hd->cap_len);
        wpos_ += hd->cap_len;
        if (wpos_ >= buf_size) { wpos_ = 0; }
    }
    #ifdef COST_TEST
    {
        timeval tv;
        gettimeofday(&tv, nullptr);
        cell->t1 = hd->ct;
        cell->t2 = tv;
    }
    #endif // COST_TEST
    que_.push();
}

void SrSwBuffer::read(cap_hdr *hd, const char* &str) {
    Cell *cell = nullptr;
    while ((cell = (Cell*)que_.front()) == nullptr) {
        cpu_delay(5);
    }

    memcpy(hd, &(cell->hd), sizeof(cap_hdr));
    str = cell->str;
    que_.pop();
}
