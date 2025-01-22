#pragma once

#include <new>
#include <stdint.h>
#include <cstddef>


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
    //
    // capacity: 内存池默认大小
    bool init(uint64_t capacity) {
        for (uint64_t i = 0; i < capacity; ++i) {
            Block *b = new Block;
            b->next = free_blocks_;
            free_blocks_ = b;
        }

        return true;
    };

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
    // 申请一块内存池，当超过默认大小时，会调用new进行分配
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

    void del_buf(Type *buf) {
        Block *b = reinterpret_cast<Block*>(reinterpret_cast<char*>(buf) - offsetof(Block, T));
        b->next  = free_blocks_;

        free_blocks_ = b;
    }

private:
    Block *free_blocks_ = nullptr;
};
