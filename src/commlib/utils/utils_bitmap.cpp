#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "utils_bitmap.h"


bool UtilsBitmap::init() {
    unInit();

    const uint64_t size = (capacity_ + 63) / 64;
    arr_ = new uint64_t[size];

    memset(arr_, 0, sizeof(uint64_t) * size);

    return true;
}

void UtilsBitmap::unInit() {
    if (arr_) {
        delete [] arr_;
        arr_ = nullptr;
    }
}

bool UtilsBitmap::set(uint64_t idx) {
    if (idx < capacity_) {
        const uint64_t pos = idx >> 6;
        const uint64_t bit = idx & 63;

        arr_[pos] |= (1ULL << bit);

        return true;
    }

    return false;
}

bool UtilsBitmap::clear(uint64_t idx) {
    if (idx < capacity_) {
        const uint64_t pos = idx >> 6;
        const uint64_t bit = idx & 63;

        arr_[pos] &= ~(1ULL << bit);

        return true;
    }

    return false;
}

bool UtilsBitmap::is_set(uint64_t idx) const {
    if (idx < capacity_) {
        const uint64_t pos = idx >> 6;
        const uint64_t bit = idx & 63;

        return arr_[pos] & (1ULL << bit);
    }

    return false;
}

// [beg, end)
uint64_t UtilsBitmap::find_first_miss(uint64_t beg, uint64_t end) const {
    assert((beg < end) && (end <= capacity()));

    if (end > capacity()) { end = capacity(); }

    uint64_t beg_pos = beg >> 6;
    uint64_t end_pos = end >> 6;

    // first arr
    const uint64_t beg_bits = beg & 63;
    for (uint64_t i = beg_bits; i < 64; ++i) {
        if (!(arr_[beg_pos] & (1ULL << i))) {
            return (beg_pos << 6) + i;
        }
    }

    // mid arrs
    ++beg_pos;
    for (; beg_pos < end_pos; ++beg_pos) {
        for (uint64_t i = 0; i < 64; ++i) {
            if (!(arr_[beg_pos] & (1ULL << i))) {
                return (beg_pos << 6) + i;
            }
        }
    }

    // last arr
    const uint64_t end_bits = end & 63;
    for (uint64_t i = 0; i < end_bits; ++i) {
        if (!(arr_[beg_pos] & (1ULL << i))) {
            return (beg_pos << 6) + i;
        }
    }

    return end; // no miss
}

// [beg, end)
uint64_t UtilsBitmap::get_miss_count(uint64_t beg, uint64_t end) const {
    assert((beg < end) && (end <= capacity()));

    if (end > capacity()) { end = capacity(); }

    uint64_t count = 0;
    uint64_t beg_pos = beg >> 6;
    uint64_t end_pos = end >> 6;

    // first arr
    const uint64_t beg_bits = beg & 63;
    for (uint64_t i = beg_bits; i < 64; ++i) {
        if (!(arr_[beg_pos] & (1ULL << i))) {
            ++count;
        }
    }

    // mid arrs
    ++beg_pos;
    for (; beg_pos < end_pos; ++beg_pos) {
        count += __builtin_popcountll(~arr_[beg_pos]);  // 计算未设置的位数
    }

    // last arr
    const uint64_t end_bits = end & 63;
    for (uint64_t i = 0; i < end_bits; ++i) {
        if (!(arr_[beg_pos] & (1ULL << i))) {
            ++count;
        }
    }

    return count;
}

void UtilsBitmap::print() const {
    fprintf(stdout, "5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0");

    const uint64_t pos = capacity() / 16;

    for (uint64_t i = 0; i < pos; ++i) {
        fprintf(stdout, "\n");
        for (uint64_t j = 16; j > 0; --j) {
            if (is_set(i * 16 + j - 1)) {
                fprintf(stdout, "1 ");
            }
            else {
                fprintf(stdout, "0 ");
            }
        }
    }

    fprintf(stdout, "\n\n");
}
