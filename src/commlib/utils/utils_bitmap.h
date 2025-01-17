#pragma once

#include <stdint.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 使用数组实现bitmap
//   capacity 为容量
//       idx计数，从0开始
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsBitmap {
public:
    UtilsBitmap(uint64_t capacity) :capacity_(capacity) { }
    ~UtilsBitmap() { unInit(); }

    bool init();
    void unInit();

    bool set(uint64_t idx);
    bool clear(uint64_t idx);

    bool is_set(uint64_t idx) const;

    void print() const;

private:
    const uint64_t capacity_ = 0;
    uint64_t *arr_ = nullptr;
};
