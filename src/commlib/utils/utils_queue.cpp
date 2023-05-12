#include <atomic>
#include <new>
#include <stdio.h>
#include <assert.h>
#include "utils_queue.h"
// #include "log.h"



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC: Multi-Producer, Single-Consumer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MPSCQueue::MPSCQueue(TYPE value_size, TYPE cell_size) : cell_max_size(cell_size), value_size(value_size) { }

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
