#pragma once

#include <stdint.h>
#include <atomic>
#include <new>
#include <assert.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// used only for one thread.
// speed(O0): 1 ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class CycleQueue {
public:
    CycleQueue(uint64_t capacity) : capacity_(capacity) {
        if (capacity_ < 1024) { capacity_ = 1024; }
        assert((capacity_ & (capacity_-1)) == 0);
    }

    ~CycleQueue() { unInit(); }

    CycleQueue(const CycleQueue&) = delete;
    CycleQueue& operator=(const CycleQueue &) = delete;

    bool init() {
        unInit();
        slots_ = new (std::nothrow) T[capacity_];
        return (slots_ != nullptr);
    }

    void unInit() {
        if (slots_) {
            delete [] slots_;
            slots_= nullptr;
        }
    }

    T* alloc() { return &(slots_[(ridx_++)%capacity_]); }

    void reset() { ridx_ = 0; }
    uint64_t capacity() const { return capacity_; }

private:
    uint64_t          ridx_ = 0;
    uint64_t      capacity_ = 0;
    alignas(64) T   *slots_ = nullptr;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue: Single-Producer, Single-Consumer
// Throughput      : (in)90.91M/S, (out)90.91M/S
// Latency(in->out): 84ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class SPSCQueue {
public:
    SPSCQueue(uint64_t capacity) {
        assert(reinterpret_cast<char*>(&ridx_) - reinterpret_cast<char*>(&widx_) >= static_cast<std::ptrdiff_t>(kCacheLineSize));
        capacity_ = capacity;
        if (capacity_ < 1024) {
            capacity_ = 1024;
        }
        // need one slack element
        capacity_ += 1;
        if (capacity_ > SIZE_MAX- 2*kPadding) {
            capacity_ = SIZE_MAX - 2*kPadding;
        }
    }

    bool init() {
        slots_ = new (std::nothrow) T[capacity_ + 2 * kPadding];
        return (slots_ != nullptr);
    }

    void unInit() {
        if (slots_) {
            delete [] slots_;
            slots_ = nullptr;
        }
    }

    T* alloc() {
        const uint64_t widx = widx_.load(std::memory_order_relaxed);
        next_widx_ = widx+1;
        if (next_widx_ == capacity_) { next_widx_ = 0; }

        if (next_widx_ == ridx_cache_) {
            ridx_cache_ = ridx_.load(std::memory_order_acquire);
            if (next_widx_ == ridx_cache_) {
                return nullptr;
            }
        }

        return &(slots_[widx + kPadding]);
    }

    T* front() {
        const uint64_t ridx = ridx_.load(std::memory_order_relaxed);
        if (ridx == widx_cache_) {
            widx_cache_ = widx_.load(std::memory_order_acquire);
            if (ridx == widx_cache_) {
                return nullptr;
            }
        }

        return &(slots_[ridx_+kPadding]);
    }

    void push() {
        widx_.store(next_widx_, std::memory_order_release);
    }

    void pop() {
        const uint64_t ridx = ridx_.load(std::memory_order_relaxed);
        uint64_t next_ridx = ridx + 1;
        if (next_ridx == capacity_) {
            next_ridx = 0;
        }

        ridx_.store(next_ridx, std::memory_order_release);
    }

    void reset() {
        widx_.store(0, std::memory_order_release);
        ridx_.store(0, std::memory_order_release);
        ridx_cache_ = 0;
        widx_cache_ = 0;
        next_widx_  = 0;
    }

    uint64_t size() const {
        std::ptrdiff_t diff = widx_.load(std::memory_order_acquire) - ridx_.load(std::memory_order_acquire);
        if (diff < 0) {
            diff += capacity_;
        }

        return static_cast<uint64_t>(diff);
    }

    bool empty() const { return 0 == size(); }
    uint64_t capacity() const { return capacity_ - 1; }

private:
    static constexpr uint16_t kCacheLineSize = 64;
    static constexpr uint64_t kPadding = (kCacheLineSize-1)/sizeof(T) + 1;

    uint64_t capacity_ = 0;
    T        *slots_ = nullptr;

    alignas(kCacheLineSize) std::atomic<uint64_t> widx_ = {0};
    alignas(kCacheLineSize) uint64_t ridx_cache_ = 0;
    alignas(kCacheLineSize) std::atomic<uint64_t> ridx_ = {0};
    alignas(kCacheLineSize) uint64_t widx_cache_ = 0;

    alignas(kCacheLineSize) uint64_t next_widx_ = 0;

    char padding_[kCacheLineSize - sizeof(next_widx_)] = {0};
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue: Single-Producer, Single-Consumer
// Throughput      : (in)16.39M/S, (out)15.87M/S
// Latency(in->out): 86ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class SPSCQueue1 {
public:
    SPSCQueue1(uint64_t capacity) : capacity_(capacity) {
        if (capacity_ < 1024) { capacity_ = 1024; }
        assert((capacity_ & (capacity_-1)) == 0);
    }

    ~SPSCQueue1() { unInit(); }

    SPSCQueue1(const SPSCQueue1&) = delete;
    SPSCQueue1& operator= (const SPSCQueue1&) = delete;

    bool init() {
        unInit();
        slots_ = new (std::nothrow) T[capacity_];
        return (slots_ != nullptr);
    }

    void unInit() {
        if (slots_) {
            delete [] slots_;
            slots_= nullptr;
        }
    }

    T* alloc() {
        const uint64_t ridx = ridx_.load(std::memory_order_relaxed);
        const uint64_t widx = widx_.load(std::memory_order_relaxed);

        // is full ?
        if (widx - ridx == capacity()) {
            return nullptr;
        }

        return &(slots_[widx % capacity()]);
    }

    T* front() {
        const uint64_t ridx = ridx_.load(std::memory_order_relaxed);
        const uint64_t widx = widx_.load(std::memory_order_acquire);

        // is empty ?
        if (widx == ridx) {
            return nullptr;
        }
        return &(slots_[ridx % capacity()]);
    }

    void push() { widx_.fetch_add(1, std::memory_order_release); }
    void pop()  { ridx_.fetch_add(1, std::memory_order_relaxed); }

    void reset() {
        widx_.store(0, std::memory_order_release);
        ridx_.store(0, std::memory_order_release);
    }

    uint64_t capacity() const { return capacity_; }
    bool empty() const { return widx_ == ridx_; }

private:
    static constexpr uint32_t kCacheLineSize = 64;

    alignas(kCacheLineSize) uint64_t capacity_ = 0;
    alignas(kCacheLineSize)          T *slots_ = nullptr;
    alignas(kCacheLineSize) std::atomic<uint64_t> widx_{0};
    alignas(kCacheLineSize) std::atomic<uint64_t> ridx_{0};

    alignas(kCacheLineSize) uint64_t next_widx_ = 0;

    char padding_[kCacheLineSize];
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
    static constexpr uint32_t kCacheLineSize = 64;

    alignas(kCacheLineSize)  char *cell_ = nullptr;
    const TYPE cell_max_size = 0;
    const TYPE value_size = 0;

    TYPE ridx_{0};

    alignas(kCacheLineSize) std::atomic<TYPE> widx_{0};
    alignas(kCacheLineSize) std::atomic<TYPE> tick_{0};
};
