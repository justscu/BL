#include <assert.h>
#include <stdint.h>
#include "utils_stl_mempool.h"


// 测试数据类型
struct TestData {
    uint32_t value;

    TestData() : value(0) {}
    TestData(int v) : value(v) { }
};

// 测试内存池
void utils_mempool_test() {
    // 创建一个内存池，初始容量为10
    UtilsMemPool<TestData> pool;
    assert(pool.init(10) == true);
    assert(pool.free_size() == 10);

    // 分配10个内存块
    TestData* blocks[15];
    for (int i = 0; i < 10; ++i) {
        blocks[i] = pool.new_buf();
        blocks[i]->value = i;
    }

    assert(pool.free_size() == 0);

    // 检查分配的内存块是否正确
    for (int i = 0; i < 10; ++i) {
        assert(blocks[i]->value == i);
    }

    // 释放所有内存块
    for (int i = 0; i < 10; ++i) {
        pool.del_buf(blocks[i]);
    }

    assert(pool.free_size() == 10);

    // 再次分配10个内存块，确保内存池复用
    for (int i = 0; i < 15; ++i) {
        blocks[i] = pool.new_buf();
        blocks[i]->value = i + 10;
    }

    assert(pool.free_size() == 0);

    // 检查再次分配的内存块是否正确
    for (int i = 0; i < 10; ++i) {
        assert(blocks[i]->value == i + 10);
    }

    // 释放所有内存块
    for (int i = 0; i < 10; ++i) {
        pool.del_buf(blocks[i]);
    }

    assert(pool.free_size() == 10);

    // 销毁内存池
    pool.uninit();

    // 尝试在销毁后分配内存，应该会动态分配新的内存块
    TestData* newBlock = pool.new_buf();
    newBlock->value = 100;
    assert(newBlock->value == 100);

    // 释放新分配的内存块
    pool.del_buf(newBlock);
}
