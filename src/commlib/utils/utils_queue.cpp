#include <atomic>
#include <assert.h>
#include "utils_queue.h"
// #include "log.h"

SPSCQueue::SPSCQueue(TYPE value_size, TYPE cell_size) : value_size(value_size), cell_max_size(cell_size) { }

bool SPSCQueue::init() {
    assert((cell_max_size & (cell_max_size-1)) == 0);
    unInit();
    cell_ = new char [cell_max_size * value_size];
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
    if (widx_ - ridx_ == cell_max_size) {
        // log_dbg("queue full.");
        return nullptr;
    }

    return cell_ + (widx_%cell_max_size)*value_size;
}

void* SPSCQueue::front() {
    if (widx_ == ridx_) {
        // log_dbg("queue empty");
        return nullptr;
    }

    return cell_ +(ridx_%cell_max_size)*value_size;
}

void SPSCQueue::push() {
    // __asm__ __volatile__("mfence" ::: "memory");
    // ++widx_;
    widx_.fetch_add(1);
}

void SPSCQueue::pop() {
    // __asm__ __volatile__("mfence" ::: "memory");
    // ++ridx_;
    ridx_.fetch_add(1);
}


