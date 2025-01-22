#pragma once

#include <new>
#include <stdint.h>
#include <cstddef>
#include <mutex>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 使用链表实现内存池
//  非线程安全
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<typename DataType>
class UtilsMemPool {
public:
    using Type = DataType;

    struct alignas(8) Block {
        Block *next;
        Type      T;

        // default construct.
        Block() :next(nullptr), T(Type()) { }
    };

    ~UtilsMemPool() { uninit(); }

public:
    // 初始化内存池
    // capacity: 内存池默认大小
    bool init(uint64_t capacity) {
        for (uint64_t i = 0; i < capacity; ++i) {
            Block *b = new Block;
            b->next = free_blocks_;
            free_blocks_ = b;
        }

        return true;
    };

    // 释放内存池
    void uninit() {
        Block *cur = free_blocks_;
        while (cur) {
            Block *next = cur->next;
            delete cur;
            cur = next;
        }

        free_blocks_ = nullptr;
    }

    //
    // 申请一块内存，当超过默认大小时，会调用new进行分配
    Type* new_buf() {
        if (free_blocks_) {
            Block *ret = free_blocks_;
            free_blocks_ = free_blocks_->next;
            return &(ret->T);
        }
        else {
            Block *ret = new Block;
            return &(ret->T);
        }
    }

    // 释放一块内存
    void del_buf(Type *buf) {
        Block *b = reinterpret_cast<Block*>(reinterpret_cast<char*>(buf) - offsetof(Block, T));
        b->next  = free_blocks_;

        free_blocks_ = b;
    }

    // 获取空闲内存块的数量
    uint64_t free_size() const {
        uint64_t ret = 0;

        Block *cur = free_blocks_;
        while (cur) {
            ++ret;
            cur = cur->next;
        }

        return ret;
    }

private:
    Block *free_blocks_ = nullptr;
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 使用链表实现内存池
//  添加互斥锁, 线程安全
//  支持多个线程同时访问和修改共享资源
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<typename DataType>
class UtilsMemPoolWithLock {
public:
    using Type = DataType;

    struct alignas(8) Block {
        Block *next;
        Type      T;

        Block() : next(nullptr), T(Type()) { }
    };

    ~UtilsMemPoolWithLock() { uninit(); }

public:
    // 初始化内存池
    bool init(uint64_t capacity) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (uint64_t i = 0; i < capacity; ++i) {
            Block *b = new Block;
            b->next = free_blocks_;
            free_blocks_ = b;
        }
        return true;
    }

    // 销毁内存池
    void uninit() {
        std::lock_guard<std::mutex> lock(mutex_);
        Block *cur = free_blocks_;
        while (cur) {
            Block *next = cur->next;
            delete cur;
            cur = next;
        }
        free_blocks_ = nullptr;
    }

    // 申请一块内存
    Type* new_buf() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (free_blocks_) {
            Block *ret = free_blocks_;
            free_blocks_ = free_blocks_->next;
            return &(ret->T);
        } else {
            Block *ret = new Block;
            return &(ret->T);
        }
    }

    // 释放一块内存
    void del_buf(Type *buf) {
        std::lock_guard<std::mutex> lock(mutex_);
        ptrdiff_t offset = offsetof(Block, T);
        Block *b = reinterpret_cast<Block*>(reinterpret_cast<char*>(buf) - offset);
        b->next = free_blocks_;
        free_blocks_ = b;
    }

    // 获取空闲内存块的数量
    uint64_t free_size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        uint64_t ret = 0;
        Block *cur = free_blocks_;
        while (cur) {
            ++ret;
            cur = cur->next;
        }
        return ret;
    }

private:
    Block *free_blocks_ = nullptr;
    mutable std::mutex mutex_;
};
