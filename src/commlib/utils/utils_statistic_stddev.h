#pragma once

#include <stdint.h>
#include <limits.h>
#include <math.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 统计: 简单的"平均偏差"计算方法
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Stddev {
public:
    struct stddev {
        uint64_t  count = 0; // 个数
        int64_t     min = LONG_MAX; // 最小值
        int64_t     max = LONG_MIN; // 最小值
        int64_t     sum = 0; // 和
        uint64_t sum_sq = 0; // 平方和
    };

public:
    static void reset(stddev *d);
    static void add(stddev *d, int64_t value);
    static void remove(stddev *d, int64_t old);
    static void modify(stddev *d, int64_t oldv, int64_t newv);
    //  avg: 均值
    // mdev: 平均偏差（偏离平均值的程度），偏差越大说明波动越大
    static void get(const stddev *d, double &avg, double &mdev);
};
