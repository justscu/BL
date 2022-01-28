#include <atomic>
#include <new>
#include <assert.h>
#include "utils_queue.h"
// #include "log.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CycleQueue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool CycleQueue::init(TYPE value_size, TYPE cell_size) {
    assert((cell_size & (cell_size-1)) == 0);

    cell_max_size_ = cell_size;
    value_size_    = value_size;

    unInit();
    cell_ = new (std::nothrow) char [cell_max_size_ * value_size_];
    if (!cell_) {
        return false;
    }

    return true;
}

void CycleQueue::unInit() {
    if (cell_) {
        delete [] cell_;
        cell_ = nullptr;
    }
    cell_used_ = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SPSCQueue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool SPSCQueue::init(TYPE value_size, TYPE cell_size) {
    assert((cell_size & (cell_size-1)) == 0);

    value_size_    = value_size;
    cell_max_size_ = cell_size;

    unInit();
    cell_ = new (std::nothrow) char [cell_max_size_ * value_size_];
    if (!cell_) {
        return false;
    }

    return true;
}

void SPSCQueue::unInit() {
    if (cell_) {
        delete [] cell_;
        cell_ = nullptr;
    }
    widx_ = 0;
    ridx_ = 0;
}

void* SPSCQueue::alloc() {
    const TYPE ridx = ridx_.load(std::memory_order_relaxed);
    const TYPE widx = widx_.load(std::memory_order_relaxed);
    if (widx - ridx == cell_max_size_) {
        // log_dbg("queue full.");
        return nullptr;
    }

    return cell_ + (widx%cell_max_size_)*value_size_;
}

void* SPSCQueue::front() {
    const TYPE ridx = ridx_.load(std::memory_order_relaxed);
    const TYPE widx = widx_.load(std::memory_order_acquire);

    if (widx == ridx) {
        // log_dbg("queue empty");
        return nullptr;
    }

    return cell_ +(ridx%cell_max_size_)*value_size_;
}

void SPSCQueue::push() {
    widx_.fetch_add(1, std::memory_order_release);
}

void SPSCQueue::pop() {
    ridx_.fetch_add(1, std::memory_order_relaxed);
}

void SPSCQueue::reset() {
    widx_.store(0, std::memory_order_release);
    ridx_.store(0, std::memory_order_release);
}

bool SPSCQueue::empty() {
    return widx_ == ridx_;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC: Multi-Producer, Single-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MPSCQueue::MPSCQueue(TYPE value_size, TYPE cell_size) : value_size(value_size), cell_max_size(cell_size) { }

bool MPSCQueue::init() {
    assert((cell_max_size & (cell_max_size-1)) == 0);
    unInit();
    cell_ = new (std::nothrow) char[cell_max_size * value_size];
    if (!cell_) {
        return false;
    }
    return true;
}

void MPSCQueue::unInit() {
    if (cell_) {
        delete [] cell_;
        cell_ = nullptr;
    }
}

void* MPSCQueue::alloc() {
    TYPE used = tick_.fetch_add(1, std::memory_order_relaxed);
    if (used >= cell_max_size) {
        tick_.fetch_sub(1, std::memory_order_relaxed);
        // full.
        return nullptr;
    }

    assert(used <= cell_max_size);
    TYPE widx = widx_.load(std::memory_order_relaxed);
    return cell_ + (widx%cell_max_size) * value_size;
}

void* MPSCQueue::front() {
    TYPE used = tick_.load(std::memory_order_relaxed);
    if (used > 0) {
        return cell_ + (ridx_%cell_max_size) * value_size;
    }
    return nullptr;
}

void MPSCQueue::push() {
    widx_.fetch_add(1, std::memory_order_release);
    // tick_.fetch_add(1, std::memory_order_release);
}

void MPSCQueue::pop() {
    ++ridx_;
    tick_.fetch_sub(1, std::memory_order_release);
}
