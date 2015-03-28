## 内存池

c语言中使用函数`malloc/calloc/realloc/free`来分配和释放内存; 
c++语言中`new/delete/new[]/delete[]`来分配和释放内存。

### 1 使用分配函数的问题
- 在分配和释放内存时，需要在用户空间与内核空间切换。不断调用new/malloc，需要不断的进行上下文切换，比较花CPU时间。
- 需要自己释放内存。若忘记释放内存，会导致内存泄漏。（内存泄漏->可用物理内存越来越少->使用硬盘作为虚拟内存->访问硬盘速度很慢）
- 频繁的分配/释放内存，会导致内存碎片，从而降低程序的运行效率。

### 2 使用内存池的一些策略
- 在程序启动时，预先申请一块较大的内存块。在需要内存时，从该内存块中划分一块出来。这样可以减少new/malloc时系统调用的次数。
- 对使用的请求进行优化。比如我们的程序经常需要1K的内存，那我们就可以预先分配一组1K的内存块。等需要的时候，直接拿来使用。
- 当不需要某块内存的时候，先将这块内存放到一个容器中，而不是delete。当程序再次需要内存时，可以直接从容器中查找适合的内存块。

### 3 C++实现
下面代码，用两个链表来实现一个简单的内存池。一个为m_used，为正在被使用的；另一个m_freed，可以提供内存供分配。
```cpp
// buffer.h
#pragma once
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream>

struct Entry
{
    Entry* next;
    size_t data_len; //总大小
    size_t data_used; //已经使用的
    unsigned char data[0];
};

// 2个链表，分别是已经使用的空间，和没有使用的空间
class MemoryPool
{
private:
    Entry* m_used; //已经使用的
    Entry* m_freed; //没有使用的
private:
    size_t GetEntrySize(const Entry* en)
    {
        size_t size = -1;

        while(en)
        {
            en = en->next;
            size++;
        }
        return size;
    }
public:
    void print_Size()
    {
        std::cout << "m_used = " << GetEntrySize(m_used) << std::endl;
        std::cout << "m_freed= " << GetEntrySize(m_freed) << std::endl << std::endl;
    }
public:
    MemoryPool();
    ~MemoryPool();

    void* memory_new(size_t size);
    void memory_delete(void* p);
};
```

```cpp
// buffer.cpp

#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

MemoryPool::MemoryPool()
{
    // 头节点
    m_used = (Entry*) ::operator new(sizeof(Entry));
    m_freed = (Entry*) ::operator new(sizeof(Entry));

    memset(m_used, 0x00, sizeof(Entry));
    memset(m_freed, 0x00, sizeof(Entry));
}

MemoryPool::~MemoryPool()
{
    Entry* p = m_used;
    while (p != NULL)
    {
        Entry* next = p->next;
        delete[] p;
        p = next;
    }

    p = m_freed;
    while (p != NULL)
    {
        Entry* next = p->next;
        delete[] p;
        p = next;
    }
}

void* MemoryPool::memory_new(size_t size)
{
    Entry* p1 = m_freed;
    Entry* p2 = m_freed->next;
    while (p2 != NULL && p2->data_len < size)
    {
        p1 = p2;
        p2 = p2->next;
    }

    // 有空余的空间可以使用
    if (p2 != NULL)
    {
        p2->data_used = size;
        // 从m_free中去掉该节点
        p1->next = p2->next;

        // 将该节点加入到m_used中
        p2->next = m_used->next;
        m_used->next = p2;

        return p2->data;
    }
    // 需要重新开辟空间
    else
    {
        Entry* newEntry =
         (Entry*) new char[sizeof(Entry) + size * sizeof(char)];
        newEntry->data_len = size;
        newEntry->data_used = size;
        newEntry->next = m_used->next;

        m_used->next = newEntry;
        return (void*) newEntry->data;
    }
}

void MemoryPool::memory_delete(void* p)
{
    Entry* p1 = m_used;
    Entry* p2 = m_used->next;
    while(p2 != NULL)
    {
        int iCal = (unsigned char*)p - (unsigned char*)&(p2->data[0]);
        // 找到了该地址
        if(iCal >= 0 && iCal <= (int)p2->data_len)
        {
            // 从m_used中删除.
            p2->data_used = 0;
            p1->next = p2->next;
            //加入到m_freed中.
            p2->next = m_freed->next;
            m_freed->next = p2;
            return;
        }
        else
        {
            p1 = p2;
            p2 = p2->next;
        }
    }
}
```

测试代码
```cpp
// 正确性验证
void MemoryPool_test_1()
{
    MemoryPool pool;
    //
    pool.print_Size();
    char* p1 = (char*) pool.memory_new(100);
    printf("p1[0x%.2x] \n", (unsigned int)p1);
    pool.print_Size();
    char* p2 = (char*) pool.memory_new(200);
    printf("p2[0x%.2x] \n", (unsigned int)p2);
    pool.print_Size();

    memcpy(p1, "hello", 6);
    memcpy(p2, "world!", 7);

    std::cout << p1 << p2 << std::endl;
    pool.memory_delete(p1);
    pool.print_Size();
    pool.memory_delete(p2);
    pool.print_Size();


    char* p3 = (char*) pool.memory_new(10);
    printf("p3[0x%.2x] \n", (unsigned int)p3); // = p2
    pool.print_Size();
    char* p4 = (char*) pool.memory_new(500);
    printf("p4[0x%.2x] \n", (unsigned int)p4);
    pool.print_Size();
    char* p5 = (char*) pool.memory_new(100);
    printf("p5[0x%.2x] \n", (unsigned int)p5); // = p1
    pool.print_Size();
}

// 速度验证
void MemoryPool_test_2()
{
#define CONST_1 5000
#define CONST_2 10
    timeval v1, v2, v3;
    // 用new/delete
    gettimeofday(&v1, NULL);
    for(int i = 0; i < CONST_1; ++i)
    {
        char* p[1000];
        for(int j = 0; j < CONST_2; ++j)
        {
            p[j] = new char[1024];
        }
        for(int j = 0; j < CONST_2; ++j)
        {
            delete [] p[j];
        }
    }
    gettimeofday(&v2, NULL);
    timersub(&v2, &v1, &v3);
    printf("new/delete: [%6d:%6d] \n", v3.tv_sec, v3.tv_usec);

    // 用 MemoryPool
    gettimeofday(&v1, NULL);
    MemoryPool pool;
    for(int i = 0; i < CONST_1; ++i)
    {
        char* p[1000];
        for(int j = 0; j < CONST_2; ++j)
        {
            p[j] = (char*)pool.memory_new(1024);
        }
        //pool.print_Size();
        for(int j = 0; j < CONST_2; ++j)
        {
            pool.memory_delete(p[j]);
        }
        //pool.print_Size();
    }
    gettimeofday(&v2, NULL);
    timersub(&v2, &v1, &v3);
    printf("operator-2: [%6d:%6d] \n", v3.tv_sec, v3.tv_usec);
}
```

### 4 关于多线程
系统提供的new/delete，malloc/free，是支持多线程的。
MemoryPool，需要增加额外的代码来支持多线程。

### 5 关于new/delete的重载
C++提供的原型
```cpp
void* operator new(std::size_t) throw (std::bad_alloc);
void* operator new[](std::size_t) throw (std::bad_alloc);
void operator delete(void*) throw();
void operator delete[](void*) throw();
void* operator new(std::size_t, const std::nothrow_t&) throw();
void* operator new[](std::size_t, const std::nothrow_t&) throw();
void operator delete(void*, const std::nothrow_t&) throw();
void operator delete[](void*, const std::nothrow_t&) throw();

// Default placement versions of operator new.
inline void* operator new(std::size_t, void* __p) throw() { return __p; }
inline void* operator new[](std::size_t, void* __p) throw() { return __p; }

// Default placement versions of operator delete.
inline void operator delete (void*, void*) throw() { }
inline void operator delete[](void*, void*) throw() { }
```

`operator new/delete`重载，只能够重载成:（A）全局函数，但不能是static型的全局函数，以及有命名空间的函数;（B）C++的static成员函数。

`operator new`，必须有个`size_t`类型的参数作为第一个参数，后面参数的个数不定. 如`void* operator new(size_t p1, ...);`
```cpp
void* operator new(size_t size){
    (void) size;
    return malloc(size);
}
```

[更多operator new/delete重载信息](http://book.51cto.com/art/201202/317799.htm)
