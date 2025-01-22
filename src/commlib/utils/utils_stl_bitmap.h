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

    uint64_t capacity() const { return capacity_; }

    bool set(uint64_t idx);
    bool clear(uint64_t idx);

    bool is_set(uint64_t idx) const;

    // 返回[beg, end)间第一个未设置的位置
    uint64_t find_first_miss(uint64_t beg, uint64_t end) const;

    // 返回[beg, end)间未设置的个数
    uint64_t get_miss_count(uint64_t beg, uint64_t end) const;

    void print() const;

private:
    const uint64_t capacity_ = 0;
    uint64_t *arr_ = nullptr;
};
