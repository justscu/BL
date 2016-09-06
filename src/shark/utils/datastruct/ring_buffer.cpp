#include <assert.h>
#include <stdio.h>
#include <new>
#include <string.h>
#include "ring_buffer.h"

namespace DS {

static inline uint32_t min(const uint32_t a, const uint32_t b) { return a <= b ? a : b; }

RingBuffer::RingBuffer(uint32_t size) {
    assert(size != 0);
    buf_size_ = size;
    buf_ = new (std::nothrow) char[size];
    assert(buf_ != nullptr);
    rPos_  = wPos_ = 0;
    bFull_ = false;
}
RingBuffer::~RingBuffer() {
    if (buf_ != nullptr) {
        delete [] buf_;
        buf_ = nullptr;
    }
}

uint32_t RingBuffer::used_size() const {
    if (bFull_) { return buf_size_; }
    else if (wPos_ >= rPos_) { return wPos_ - rPos_; }
    else { return wPos_ + buf_size_ - rPos_; }
}

// 若成功，则返回扩展后的size.
int32_t RingBuffer::expand() {
    uint32_t size = buf_size_ * 2;
    char* buf = new (std::nothrow) char[size];
    if (buf == nullptr) {
        assert(buf != nullptr);
        return 0; // failed.
    }
    uint32_t rLen = read(buf, used_size());
    bFull_ = false;
    rPos_  = 0;
    wPos_  = rLen;
    delete [] buf_;
    buf_      = buf;
    buf_size_ = size;
    return buf_size_;
}

// 返回实际写入的字节数
uint32_t RingBuffer::write(const char* dat, const uint32_t datLen) {
    assert(dat != nullptr && datLen != 0);
    if (bFull_ == true && rPos_ == wPos_) return 0; // full.
    if (rPos_ <= wPos_) {
        uint32_t   canWrite = min(datLen,   buf_size_-wPos_+rPos_);
        uint32_t rightWrite = min(canWrite, buf_size_-wPos_);
        memcpy(buf_+wPos_, dat, rightWrite);
        if (rightWrite < canWrite) {
            memcpy(buf_, dat+rightWrite, canWrite-rightWrite);
        }

        wPos_ = (wPos_+canWrite)%buf_size_;
        if (wPos_ == rPos_) { bFull_ = true; }
        return canWrite;
    } else {
        uint32_t canWrite = min(datLen, rPos_-wPos_);
        memcpy(buf_+wPos_, dat, canWrite);
        wPos_ += canWrite;
        if (wPos_ == rPos_) { bFull_ = true; }
        return canWrite;
    }
}

// 返回实际读入的字节数
uint32_t RingBuffer::read(char* dat, const uint32_t datLen) {
    assert(dat != nullptr && datLen != 0);
    if (wPos_ == rPos_ && bFull_ == false) return 0;
    if (wPos_ > rPos_) {
        uint32_t canRead = min(datLen, wPos_-rPos_);
        memcpy(dat, buf_+rPos_, canRead);
        rPos_ += canRead;
        bFull_ = false;
        assert(rPos_ <= wPos_);
        return canRead;
    } else {
        uint32_t canRead   = min(datLen,  buf_size_-(rPos_-wPos_));
        uint32_t rightRead = min(canRead, buf_size_-rPos_);
        memcpy(dat, buf_+rPos_, rightRead);
        if (rightRead < canRead) {
            memcpy(dat+rightRead, buf_, canRead-rightRead);
        }
        rPos_ = (rPos_+canRead)%buf_size_;
        bFull_ = false;
        return canRead;
    }
}

}
