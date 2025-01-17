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


void utils_bitmap_test() {
    test_init_uninit();
    test_set();
    test_clear();
    test_out_of_bounds();
}
