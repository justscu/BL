#pragma once

#include <vector>
#include <assert.h>
#include <condition_variable>

// copy from spdlog.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// circle queue: not thread safe !!!
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class ValueType>
class UtilsCircleQueue {
public:
    explicit UtilsCircleQueue(size_t max_items) : max_items_(max_items) {
        assert(max_items % 8 == 0);
        vec_.reserve(max_items);
    }

    UtilsCircleQueue(const UtilsCircleQueue &v) = default;
    UtilsCircleQueue& operator=(const UtilsCircleQueue &v) = default;

    // move can't be default.
    UtilsCircleQueue(UtilsCircleQueue &&v) { copy_movable(std::move(v)); }
    UtilsCircleQueue& operator=(UtilsCircleQueue &&v) {
        copy_moveable(std::move(v));
        return *this;
    }

    // if have no room, remove the oldest.
    void push_back(ValueType &&v) {
        vec_[tail_] = std::move(v);
        tail_ = (tail_ + 1) % max_items_;

        // overrun
        if (tail_ == head_) {
            head_ = (head_ + 1) % max_items_;
            ++overrun_cnt_;
        }
    }

    //
    const ValueType& front() const { return vec_[head_]; }
    ValueType&       front() { return vec_[head_]; }

    // return number of elements actually stored.
    size_t size() const {
        if (tail_ >= head_) {
            return tail_ - head_;
        }
        else {
            return max_items_ - (head_ - tail_);
        }
    }

    //
    const ValueType& at(size_t i) const {
        assert(i < size());
        return vec_[(head_ + i) % max_items_];
    }

    void     pop_front() { head_ = (head_+1) % max_items_; }
    bool          full() const { return ((tail_+1)%max_items_) == head_; }
    bool         emtpy() const { return tail_ == head_; }
    size_t overrun_cnt() const { return overrun_cnt_; }



private:
    void copy_moveable(UtilsCircleQueue &&v) {
        max_items_ = v.max_items_;
        head_ = v.head_;
        tail_ = v.tail_;
        overrun_cnt_ = v.overrun_cnt_;
        vec_ = std::move(v);

        v.max_items_ = 0;
        v.head_ = v.tail_ = 0;
        v.overrun_cnt_ = 0;
    }


private:
    typename std::vector<ValueType>::size_type head_ = 0;
    typename std::vector<ValueType>::size_type tail_ = 0;

    std::vector<ValueType> vec_;
    const size_t     max_items_ = 0;
    size_t         overrun_cnt_ = 0;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 多读多写循环队列
// Multi-read & Multi-write Blocking queue.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class ValueType>
class MrmwBlockQueue {
public:
    explicit MrmwBlockQueue(size_t max_items) : que_(max_items) { }

    // block if no room for enqueue.
    void enqueue_wait(ValueType &&v) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            pop_cv_.wait(lock, [this]{return !this->que_.full(); });
            que_.push_back(std::move(v));
        }
        push_cv_.notify_one();
    }

    // enqueue immediately. overrun oldest message.
    void enqueue_nowait(ValueType &&v) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            que_.push_back(std::move(v));
        }
        push_cv_.notify_one();
    }

    //
    bool dequeue_for(ValueType &popped_v, std::chrono::milliseconds wait_millisec) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!push_cv_.wait_for(lock, wait_millisec, [this]{ return !this->que_.empty(); })) {
                return false;
            }
            popped_v = std::move(que_.front());
            que_.pop_front();
        }
        pop_cv_.notify_one();
        return true;
    }

    size_t overrun_cnt() {
        std::unique_lock<std::mutex> lock(mutex_);
        return que_.overrun_cnt();
    }

    size_t size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return que_.size();
    }

private:
    std::mutex mutex_;
    std::condition_variable push_cv_;
    std::condition_variable  pop_cv_;
    UtilsCircleQueue<ValueType> que_;
};
