#include <new>
#include <string.h>
#include "buffer.h"

bool SrSwBuffer::init() {
    unInit();

    buf_ = new (std::nothrow) char[buf_sent_size + buf_size];
    if (!buf_) {
        log_err("SrSwBuffer new char[%d] faild", buf_sent_size + buf_size);
        return false;
    }

    vec_ = new (std::nothrow) Cell[vec_size];
    if (!vec_) {
        log_err("SrSwFiFoVector new Cell[%lu] faild", vec_size);
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

void SrSwBuffer::reset() {
	wpos_ = 0;
	vec_widx_ = vec_ridx_ = 0;
	total_wt_len_ = total_rd_len_ = 0;
}

bool SrSwBuffer::write(const cap_hdr *hd, const char *str) {
    if (vec_widx_ - vec_ridx_ < vec_size-1) {
        const uint64_t idx = (vec_widx_ % vec_size);
        vec_[idx].str = buf_ + wpos_;
        // copy
        {
            memcpy(&(vec_[idx].hd), hd, sizeof(cap_hdr));
            memcpy(buf_ + wpos_, str, hd->cap_len);
            wpos_ += hd->cap_len;
            if (wpos_ >= buf_size) {
                wpos_ = 0;
            }
        }
        // print
        if (0) {
            total_wt_len_ += (uint32_t)hd->cap_len;
            log_dbg("Vector__in : [%lu] [%p] , total[%lu]\n",
                    vec_widx_, vec_[idx].str, total_wt_len_);

        }
        ++vec_widx_;
        return true;
    }

    // log_dbg("buff full: vec_widx[%lu] vec_ridx[%lu], vec_size[%lu] \n", vec_widx_, vec_ridx_, vec_size);
    return false;
}

bool SrSwBuffer::read(cap_hdr *hd, const char * &str) {
	if (vec_ridx_ < vec_widx_) {
	    const uint64_t idx = (vec_ridx_ % vec_size);
	    memcpy(hd, &(vec_[idx].hd), sizeof(cap_hdr));
	    str = vec_[idx].str;
	    // log_dbg("Vector_out : [%lu] [%p] \n", vec_ridx_, str);
	    ++vec_ridx_;
	    return true;
	}


    // log_dbg("buff empty: vec_widx[%lu] vec_ridx[%lu]. \n", vec_widx_, vec_ridx_);
    return false;
}
