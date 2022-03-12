#include "utils_lock.h"

namespace UtilsTest {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 循环队列，每个Item里面，自带原子标志(细力度)
// Multi-Producer Single-Consumer.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <typename TYPE>
class RingBuffer {
public:
    alignas(64) struct Item {
        std::atomic_flag flag{ATOMIC_FLAG_INIT};
        bool            ready{false}; // value is usable ?
        char          padding[256-sizeof(std::atomic_flag) - sizeof(bool) - sizeof(TYPE)];
        TYPE            value;
    };

public:
    RingBuffer(uint32_t size) : size_(size) {
        ring_ = (Item*)malloc(size_*sizeof(Item));
        for (uint32_t i = 0; i < size_; ++i) {
            new (&(ring_[i])) Item; // call constructor
        }
        static_assert(sizeof(Item) == 256, "sizeof(Item) must 256");
    }

    ~RingBuffer() {
        for (uint32_t i = 0; i < size_; ++i) {
            ring_[i].~Item();
        }
        free(ring_);
        ring_ = nullptr;
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 往队列中插入数据: 通过SpinLock支持多线程
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void push(TYPE && data) {
        uint32_t widx = widx_.fetch_add(1, std::memory_order_relaxed) % size_;
        Item &v = ring_[widx];
        UtilSpinLock lock(v.flag);
        v.value = std::move(data);
        v.ready = true;
    }

    bool try_pop(TYPE & data) {
        Item &v = ring_[ridx_ % size_];
        UtilSpinLock lock(v.flag);
        if (v.ready == true) {
            data = std::move(v.value);
            v.ready = false;
            ++ridx_;
            return true;
        }
        return false;
    }

private:
    RingBuffer(RingBuffer &) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

private:
    uint32_t size_ = 0;
    Item   *ring_ = nullptr;
    std::atomic<uint32_t> widx_{0};
    char pad[64];
    uint32_t ridx_{0};
};


} // namespace UtilsTest
