#pragma once

#include <stdint.h>
#include <atomic>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue: Single-Producer, Single-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
    char      *cell_ = nullptr;
    const TYPE cell_max_size = 0;
    const TYPE value_size = 0;

    static constexpr uint32_t kCacheLineSize = 64;
    alignas(kCacheLineSize) std::atomic<TYPE> widx_{0};
    alignas(kCacheLineSize) std::atomic<TYPE> ridx_{0};
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC: Multi-Producer, Single-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class MPSCQueue {
    using TYPE=uint32_t;
public:
    MPSCQueue(TYPE value_size, TYPE cell_size);
    ~MPSCQueue() { unInit(); }

    bool init();
    void unInit();

    void* alloc();
    void* front();

    void push();
    void pop();

private:
    char      *cell_ = nullptr;
    const TYPE cell_max_size = 0;
    const TYPE value_size = 0;

    TYPE ridx_{0};
    static constexpr uint32_t kCacheLineSize = 64;
    alignas(kCacheLineSize) std::atomic<TYPE> widx_{0};
    alignas(kCacheLineSize) std::atomic<TYPE> tick_{0};

};
