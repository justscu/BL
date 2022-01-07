#pragma once

#include <stdint.h>
#include <atomic>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue : lockfree.
// Single-Producer, Single-consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// memory_order_consume
// memory_order_release
// memory_order_acquire

class SPSCQueue {
    using TYPE=uint32_t;
public:
    SPSCQueue(TYPE value_size, TYPE cell_size);
    ~SPSCQueue() { unInit(); }

    bool init();
    void unInit();

    void* alloc();
    void* front();

    void push();
    void pop();

private:
    char          *cell_ = nullptr;
    const TYPE cell_max_size = 0;
    const TYPE value_size = 0;

    static constexpr size_t kCacheLineSize = 64;
//    alignas(kCacheLineSize) volatile TYPE widx_{0};
//    alignas(kCacheLineSize) volatile TYPE ridx_{0};

    alignas(kCacheLineSize) std::atomic<TYPE> widx_{0};
    alignas(kCacheLineSize) std::atomic<TYPE> ridx_{0};
};


#if 0
template<class TYPE>
class SPSCQueue {
    explicit SPSCQueue(uint16_t max_size) : data_max_size(max_size) { }

public:
    bool init() {
        assert(data_max_size & (data_max_size-1) == 0); // must power of 2.
        data_ = new (std::nothrow) TYPE[data_max_size];
        return data_ != nullptr;
    }

    void unInit() {
        if (data_) {
            delete [] data_;
            data_ = nullptr;
        }
    }

    TYPE* alloc() {
        if (widx_ - ridx_cache_ == data_max_size) {
            ridx_cache_ = ((std::atomic<uint16_t>*)(&ridx_))->load(std::memory_order_consume);
            if (__builtin_expect(widx_ - ridx_cache_ == data_max_size, 0)) {
                return nullptr;
            }
        }

        return &(data_[widx_%data_max_size]);
    }

    void push() {
        ((std::atomic<uint16_t>*)(&widx_))->store(widx_+1, std::memory_order_release);
    }

    bool try_push(const TYPE &d) {
        TYPE *p = alloc();
        if (!p) { return false; }

        memcpy(p, &d, sizeof(TYPE));
        push();
        return true;
    }

    void block_push(const TYPE &d) {
        while (!try_push(d)) {;}
    }

    TYPE* front() {
        if (ridx_ == ((std::atomic<uint16_t>*)(&widx_))->load(std::memory_order_acquire)) {
            return nullptr;
        }
        return &(data_[ridx_%data_max_size]);
    }

    void pop() {
        ((std::atomic<uint16_t>*)(&ridx_))->store(ridx_+1, std::memory_order_release);
    }


private:
    static constexpr size_t kCacheLineSize = 64;
    alignas(kCacheLineSize) uint16_t widx_ = 0;
    alignas(kCacheLineSize) uint16_t ridx_ = 0;
    uint16_t ridx_cache_ = 0;

    const uint16_t                data_max_size = 0;
    alignas(kCacheLineSize) TYPE *data_    = nullptr;
};


template<class T>
class SPSCQueue1 {
public:
    explicit SPSCQueue1(const size_t capacity) {
        capacity_ = capacity;
        if (capacity_ < 1) { capacity_ = 1; }
        ++capacity_;

        if (capacity_ > SIZE_MAX - 2 * kPadding) {
            capacity_ = SIZE_MAX - 2 * kPadding;
        }

        slots_ = new T[capacity_ + 2 *kPadding];

        static_assert(alignof(SPSCQueue1<T>) == kCacheLineSize, "");
        static_assert(sizeof(SPSCQueue1<T>) >= 3*kCacheLineSize, "");
        assert(reinterpret_cast<char*>(&read_idx_) - reinterpret_cast<char*>(&write_idx_) >= static_cast<std::ptrdiff_t>(kCacheLineSize));
    }

    SPSCQueue1(const SPSCQueue1&) = delete;
    SPSCQueue1* operator=(const SPSCQueue1&) = delete;

    template<typename... Args>
    void emplace(Args &&...args) noexcept {
        auto const write_idx = write_idx_.load(std::memory_order_relaxed);
        auto next_write_idx = write_idx + 1;
        if (next_write_idx == capacity_) { next_write_idx = 0; }
    }

private:
    static constexpr size_t kCacheLineSize = 64;
    static constexpr size_t kPadding = (kCacheLineSize - 1) / sizeof(T) + 1;

    size_t capacity_ = 0;
    T*        slots_ = nullptr;
    // Align to cache line size in order to avoid false sharing.
    alignas(kCacheLineSize) std::atomic<size_t> write_idx_ = {0};
    alignas(kCacheLineSize) size_t read_idx_cache_ = 0;
    alignas(kCacheLineSize) std::atomic<size_t> read_idx_ = {0};
    alignas(kCacheLineSize) size_t write_idx_cache_ = 0;

    // padding
    char padding[kCacheLineSize - sizeof(write_idx_cache_)];
};
#endif
