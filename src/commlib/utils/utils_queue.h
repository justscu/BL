#pragma once

#include <stdint.h>
#include <atomic>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// used only for one thread.
// speed(O0): 3 ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CycleQueue {
    using TYPE=uint32_t;
public:
    CycleQueue() { }
    ~CycleQueue() { unInit(); }

    CycleQueue(const CycleQueue&) = delete;
    CycleQueue& operator=(const CycleQueue &) = delete;

    bool init(TYPE value_size, TYPE cell_size);
    void unInit();

    void* alloc() { return cell_ + ((cell_used_++)%cell_max_size_)*value_size_; }
    void  clear() { cell_used_ = 0; }
    TYPE   used() const { return cell_used_; }
    void  reset() { cell_used_ = 0; }

private:
    char         *cell_ = nullptr;
    TYPE cell_max_size_ = 0;
    TYPE    value_size_ = 0;

    TYPE cell_used_ = 0;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue: Single-Producer, Single-Consumer
// speed: 56ns(O2).
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SPSCQueue {
    using TYPE=uint32_t;
public:
    SPSCQueue() { }
    ~SPSCQueue() { unInit(); }

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator= (const SPSCQueue&) = delete;

    bool init(TYPE value_size, TYPE cell_size);
    void unInit();

    void* alloc();
    void* front();

    void push();
    void pop();

    void reset();
    bool is_empty();

private:
    char         *cell_ = nullptr;
    TYPE cell_max_size_ = 0;
    TYPE    value_size_ = 0;

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
