#pragma once

#include <stdint.h>
#include <atomic>
#include <new>
#include <assert.h>
#include <utility>

/*

| type | queue size | Throughput(in, W/s) | Throughput(out, W/s) | Latency(ns) | tips |
|:----:|------------|--------------------:|---------------------:|------------:|------|
| cycle|  1024*1024 |      2 ns,   5 WW/s |       2 ns,   5 WW/s |        2 ns |
| SPSC |  1024*1024 |     11 ns, 9000 W/s |      11 ns, 9000 W/s |       74 ns |
| SPSC1|  1024*1024 |     58 ns, 1724 W/s |      58 ns, 1724 W/s |       84 ns |
| MPSC |  1024*1024 |    360 ns,  278 W/s |     120 ns,  833 W/s |      233 ns | 2P1C |
| MPMC |  1024*1024 |    660 ns,  152 W/s |     500 ns,  200 W/s |      251 ns | 4P2C |

*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// used only for one thread.
// speed(O0): 2 ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class CycleQueue {
public:
    CycleQueue(uint64_t capacity) : capacity_(capacity) {
        assert((capacity_ & (capacity_-1)) == 0);

        if (capacity_ < 1024) { capacity_ = 1024; }
        if ((capacity_ & (capacity_-1)) != 0) {
            exit(0);
        }
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

    T* alloc() { return &(slots_[(ridx_++) & mod()]); }

    void reset() { ridx_ = 0; }

private:
    constexpr uint64_t mod() const { return capacity_-1; }

private:
    uint64_t          ridx_ = 0;
    uint64_t      capacity_ = 0;
    alignas(64) T   *slots_ = nullptr;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue: Single-Producer, Single-Consumer
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class SPSCQueue1 {
public:
    SPSCQueue1(uint64_t capacity) : capacity_(capacity) {
        assert((capacity_ & (capacity_-1)) == 0);

        if (capacity_ < 1024) { capacity_ = 1024; }
        if ((capacity_ & (capacity_-1)) != 0) {
            exit(0);
        }
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
        if (widx - ridx == capacity_) {
            return nullptr;
        }

        return &(slots_[widx & mod()]);
    }

    T* front() {
        const uint64_t ridx = ridx_.load(std::memory_order_relaxed);
        const uint64_t widx = widx_.load(std::memory_order_acquire);

        // is empty ?
        if (widx == ridx) {
            return nullptr;
        }
        return &(slots_[ridx & mod()]);
    }

    void push() { widx_.fetch_add(1, std::memory_order_release); }
    void pop()  { ridx_.fetch_add(1, std::memory_order_relaxed); }

    void reset() {
        widx_.store(0, std::memory_order_release);
        ridx_.store(0, std::memory_order_release);
    }

    bool empty() const { return widx_ == ridx_; }

private:
    constexpr uint64_t mod() const { return capacity_-1; }

private:
    static constexpr uint32_t kCacheLineSize = 64;

    alignas(kCacheLineSize) uint64_t capacity_ = 0;
    alignas(kCacheLineSize)          T *slots_ = nullptr;
    alignas(kCacheLineSize) std::atomic<uint64_t> widx_{0};
    alignas(kCacheLineSize) std::atomic<uint64_t> ridx_{0};

    alignas(kCacheLineSize) uint64_t next_widx_ = 0;

    char padding_[kCacheLineSize];
};


template<class T>
class Slot {
public:
    Slot() {}
    ~Slot() {
        if (turn & 1) {
            destroy();
        }
    }

    template <typename... Args>
    void construct(Args &&...args) {
        new (&data) T(std::forward<Args>(args)...);
    }

    void destroy() {
        reinterpret_cast<T *>(&data)->~T();
    }

    T &&move() noexcept { return reinterpret_cast<T &&>(data); }

public:
    static constexpr uint32_t kCacheLineSize = 64;
    alignas(kCacheLineSize) std::atomic<uint32_t> turn{0};
    alignas(kCacheLineSize)                     T data;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC: Multi-Producer, Single-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class MPSCQueue {
public:
    MPSCQueue(uint64_t capacity) : capacity_(capacity) {
        assert((capacity_ & (capacity_-1)) == 0);

        if (capacity_ < 1024) { capacity_ = 1024; }
        if ((capacity_ & (capacity_-1)) != 0) {
            exit(0);
        }
    }

    ~MPSCQueue() { unInit(); }

    bool init() {
        unInit();
        slots_ = new (std::nothrow) Slot<T>[capacity_+1];
        if (!slots_) { return false; }

        for (uint64_t i = 0; i < capacity_; ++i) {
            new (&slots_[i]) Slot<T>;
        }
        return true;
    }

    void unInit() {
        if (slots_) {
            for (uint64_t i = 0; i < capacity_; ++i) {
                slots_[i].~Slot<T>();
            }
            delete [] slots_;
            slots_ = nullptr;
        }
    }

    template <typename... Args>
    bool try_push(Args &&...args) noexcept {
        uint64_t tick = tick_.fetch_add(1, std::memory_order_relaxed);
        if (tick >= capacity_) {
            tick_.fetch_sub(1, std::memory_order_relaxed);
            return false; // full.
        }

        uint64_t head = head_.load(std::memory_order_acquire);
        while (true) {
            Slot<T> &slot = slots_[head & mod()];
            if (turn(head)*2 == slot.turn.load(std::memory_order_acquire)) {
                if (head_.compare_exchange_strong(head, head+1)) {
                    slot.construct(std::forward<Args>(args)...);
                    slot.turn.store(turn(head)*2+1, std::memory_order_release);
                    return true;
                }
            }
            else {
                const uint64_t pre = head;
                head = head_.load(std::memory_order_acquire);
                if (pre == head) {
                    return false;
                }
            }
        } // while
    }

    bool try_pop(T &v) {
        uint64_t used = tick_.load(std::memory_order_relaxed);
        if (used == 0) {
            return false; // empty.
        }

        //
        while (true) {
            Slot<T> &slot = slots_[tail_ & mod()];
            if (turn(tail_) * 2 + 1 == slot.turn.load(std::memory_order_acquire)) {
                v = slot.move();
                slot.destroy();
                slot.turn.store(turn(tail_)*2+2, std::memory_order_release);

                ++tail_;
                assert(tick_ > 0);
                tick_.fetch_sub(1, std::memory_order_release);
                return true;
            }
            else {
                return false;
            }
        }
    }

    uint64_t size() const { return tick_.load(std::memory_order_acquire); }
    bool empty() const noexcept { return size() <= 0; }

private:
    uint64_t turn(uint64_t i) const noexcept { return i / capacity_; }
    constexpr uint64_t mod() const { return capacity_-1; }

private:
    static constexpr uint32_t kCacheLineSize = 64;

    alignas(kCacheLineSize) uint64_t capacity_ = 0;
    alignas(kCacheLineSize)    Slot<T> *slots_ = nullptr;

    alignas(kCacheLineSize) std::atomic<uint64_t> head_{0}; // multi-producer
    alignas(kCacheLineSize) std::atomic<uint64_t> tick_{0}; // producer-consumer

    alignas(kCacheLineSize) uint64_t tail_{0}; // one-consumer.
    char padding_[kCacheLineSize];
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPMC: Multi-Producer, Multi-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class T>
class MPMCQueue {
public:
    MPMCQueue(uint64_t capacity) : capacity_(capacity) {
        assert((capacity_ & (capacity_-1)) == 0);

        if (capacity_ < 1024) { capacity_ = 1024; }
        if ((capacity_ & (capacity_-1)) != 0) {
            exit(0);
        }
    }

    ~MPMCQueue() { unInit(); }

    bool init() {
        unInit();
        slots_ = new (std::nothrow) Slot<T>[capacity_+1];
        if (!slots_) { return false; }

        for (uint64_t i = 0; i < capacity_; ++i) {
            new (&slots_[i]) Slot<T>;
        }
        return true;
    }

    void unInit() {
        if (slots_) {
            for (uint64_t i = 0; i < capacity_; ++i) {
                slots_[i].~Slot<T>();
            }
            delete [] slots_;
            slots_ = nullptr;
        }
    }

    template <typename... Args>
    bool try_push(Args &&...args) noexcept {
        uint64_t head = head_.load(std::memory_order_acquire);

        while (true) {
            Slot<T> &slot = slots_[head & mod()];
            if (turn(head)*2 == slot.turn.load(std::memory_order_acquire)) {
                if (head_.compare_exchange_strong(head, head+1)) {
                    slot.construct(std::forward<Args>(args)...);
                    slot.turn.store(turn(head)*2+1, std::memory_order_release);
                    return true;
                }
            }
            else {
                const uint64_t pre = head;
                head = head_.load(std::memory_order_acquire);
                if (pre == head) {
                    return false;
                }
            }
        } // while
    }

    bool try_pop(T &v) {
        uint64_t tail = tail_.load(std::memory_order_acquire);
        while (true) {
            Slot<T> &slot = slots_[tail & mod()];
            if (turn(tail) * 2 + 1 == slot.turn.load(std::memory_order_acquire)) {
                if (tail_.compare_exchange_strong(tail, tail + 1)) {
                    v = slot.move();
                    slot.destroy();
                    slot.turn.store(turn(tail)*2+2, std::memory_order_release);
                    return true;
                }
            }
            else {
                const uint64_t pre = tail;
                tail = tail_.load(std::memory_order_acquire);
                if (pre == tail) {
                    return false;
                }
            }
        }
    }

    uint64_t size() const noexcept {
        std::ptrdiff_t diff = tail_.load(std::memory_order_acquire) - head_.load(std::memory_order_acquire);
        if (diff < 0) {
            diff += capacity_;
        }

        return static_cast<uint64_t>(diff);
    }

    bool empty() const noexcept { return size() <= 0; }

private:
    uint64_t turn(uint64_t i) const noexcept { return i / capacity_; }
    constexpr uint64_t mod() const { return capacity_-1; }

private:
    static constexpr uint32_t kCacheLineSize = 64;

    alignas(kCacheLineSize) uint64_t capacity_ = 0;
    alignas(kCacheLineSize)    Slot<T> *slots_ = nullptr;

    alignas(kCacheLineSize) std::atomic<uint64_t> head_{0};
    alignas(kCacheLineSize) std::atomic<uint64_t> tail_{0};

    char padding_[kCacheLineSize];
};
