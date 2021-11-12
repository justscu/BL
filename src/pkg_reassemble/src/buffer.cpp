#include <new>
#include <string.h>
#include "buffer.h"
#include "define.h"

bool SrSwBuffer::init() {
    unInit();

    buf_ = new (std::nothrow) char[buf_sent_size + buf_size];
    if (!buf_) {
        log_err("SrSwBuffer new char[%d] faild. \n", buf_sent_size + buf_size);
        return false;
    }

    vec_ = new (std::nothrow) Cell[vec_size];
    if (!vec_) {
        log_err("SrSwFiFoVector new Cell[%lu] faild. \n", vec_size);
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

    if (vec_) {
        delete [] vec_;
        vec_ = nullptr;
    }
}

bool SrSwBuffer::write(const char *str, const int32_t len) {
    if (vec_widx_ - vec_ridx_ < vec_size-1) {
        const uint64_t idx = (vec_widx_ % vec_size);
        vec_[idx].str = buf_ + wpos_;
        // copy
        {
            memcpy(buf_ + wpos_, str, len);
            wpos_ += len;
            if (wpos_ >= buf_size) {
                wpos_ = 0;
            }
        }
        // print
        if (0) {
            total_wt_len_ += (uint32_t)len;
            log_dbg("Vector__in : [%lu] [%p] , total[%lu]\n",
                    vec_widx_, vec_[idx].str, total_wt_len_);

        }
        ++vec_widx_;
        return true;
    }

    log_dbg("buff full: vec_widx[%lu] vec_ridx[%lu], vec_size[%lu] \n",
            vec_widx_, vec_ridx_, vec_size);
    return false;
}

bool SrSwBuffer::read(const char * &str) {
    if (vec_ridx_ >= vec_widx_) {
        // log_dbg("buff empty: vec_ridx[%lu] vec_widx[%lu]. \n", vec_ridx_, vec_widx_);
        return false;
    }

    const uint64_t idx = (vec_ridx_ % vec_size);
    str = vec_[idx].str;
    log_dbg("Vector_out : [%lu] [%p] \n", vec_ridx_, str);
    ++vec_ridx_;
    return true;
}
