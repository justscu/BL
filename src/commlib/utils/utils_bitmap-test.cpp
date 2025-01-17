#include <assert.h>
#include "utils_bitmap.h"


void test_init_uninit() {
    UtilsBitmap bitmap(128);

    assert(bitmap.init());  // 初始化成功

    bitmap.unInit();  // 释放内存

    assert(bitmap.init());  // 重新初始化成功

    bitmap.unInit();  // 再次释放内存
}

void test_set() {
    UtilsBitmap bitmap(128);
    assert(bitmap.init());

    bitmap.set(0);
    bitmap.set(5);
    bitmap.set(10);
    bitmap.set(127);

    bitmap.print();

    assert(bitmap.is_set(0));
    assert(bitmap.is_set(5));
    assert(bitmap.is_set(10));
    assert(bitmap.is_set(127));

    bitmap.unInit();
}

void test_clear() {
    UtilsBitmap bitmap(128);
    assert(bitmap.init());

    bitmap.set(0);
    bitmap.set(5);
    bitmap.set(10);
    bitmap.set(127);

    bitmap.print();

    bitmap.clear(0);
    bitmap.clear(5);
    bitmap.clear(10);
    bitmap.clear(127);

    bitmap.print();

    assert(!bitmap.is_set(0));
    assert(!bitmap.is_set(5));
    assert(!bitmap.is_set(10));
    assert(!bitmap.is_set(127));

    bitmap.unInit();
}

void test_out_of_bounds() {
    UtilsBitmap bitmap(128);
    assert(bitmap.init());

    assert(!bitmap.set(128));  // 越界
    assert(!bitmap.clear(128));  // 越界
    assert(!bitmap.is_set(128));  // 越界

    bitmap.print();

    bitmap.unInit();
}

void test_find_first_miss() {
    // 测试用例1: 基本功能测试
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        bitmap.set(0);
        bitmap.set(5);
        bitmap.set(10);
        bitmap.set(127);

        assert(bitmap.find_first_miss(0, 128) == 1);  // 第一个未设置的位是1
        assert(bitmap.find_first_miss(10, 128) == 11);  // 第一个未设置的位是11
        assert(bitmap.find_first_miss(127, 128) == 128);  // 没有未设置的位
    }

    // 测试用例2: 边界条件测试
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        bitmap.set(0);
        bitmap.set(127);

        assert(bitmap.find_first_miss(0, 128) == 1);  // 第一个未设置的位是1
        assert(bitmap.find_first_miss(127, 128) == 128);  // 没有未设置的位
    }

    // 测试用例3: 全部位都设置的情况
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        for (uint64_t i = 0; i < 128; ++i) {
            bitmap.set(i);
        }

        assert(bitmap.find_first_miss(0, 128) == 128);  // 没有未设置的位
    }

    // 测试用例4: 全部位都未设置的情况
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        assert(bitmap.find_first_miss(0, 128) == 0);  // 第一个未设置的位是0
    }
}

void test_get_miss_count() {
    // 测试用例1: 基本功能测试
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        bitmap.set(0);
        bitmap.set(5);
        bitmap.set(10);
        bitmap.set(127);

        assert(bitmap.get_miss_count(0, 128) == 124);  // 128 - 4 = 124
        assert(bitmap.get_miss_count(10, 128) == 117);  // 118 - 1 = 117
        assert(bitmap.get_miss_count(127, 128) == 0);  // 没有未设置的位

        bitmap.unInit();
    }

    // 测试用例2: 边界条件测试
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        bitmap.set(0);
        bitmap.set(127);

        assert(bitmap.get_miss_count(0, 128) == 126);  // 128 - 2 = 126
        assert(bitmap.get_miss_count(127, 128) == 0);  // 没有未设置的位

        bitmap.unInit();
    }

    // 测试用例3: 全部位都设置的情况
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        for (uint64_t i = 0; i < 128; ++i) {
            bitmap.set(i);
        }

        assert(bitmap.get_miss_count(0, 128) == 0);  // 没有未设置的位

        bitmap.unInit();
    }

    // 测试用例4: 全部位都未设置的情况
    {
        UtilsBitmap bitmap(128);
        assert(bitmap.init());

        assert(bitmap.get_miss_count(0, 128) == 128);  // 所有位都未设置

        bitmap.unInit();
    }
}

void utils_bitmap_test() {
    test_init_uninit();
    test_set();
    test_clear();
    test_out_of_bounds();
    test_find_first_miss();
}
