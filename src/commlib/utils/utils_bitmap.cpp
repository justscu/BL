#include <string.h>
#include <stdio.h>
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

        return (arr_[pos] >> bit) & 0x01;
    }

    return false;
}

void UtilsBitmap::print() const {
    fprintf(stdout, "0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5");

    for (uint64_t idx = 0; idx < capacity_; ++idx) {
        if (idx % 16 == 0) { fprintf(stdout, "\n"); }

        if (is_set(idx)) {
            fprintf(stdout, "1 ");
        }
        else {
            fprintf(stdout, "0 ");
        }
    }

    fprintf(stdout, "\n\n");
}
