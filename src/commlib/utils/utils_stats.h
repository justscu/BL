#pragma once

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <vector>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 统计，均值、方差、标准差、分位数
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Sta {
public:
    struct Rst {
        int64_t  cnt = 0;
        int64_t  min = 0;
        int64_t  max = 0;
        int64_t  avg = 0; // average value.

        uint64_t variance = 0; // 方差（方差越小就越稳定, 方差描述随机变量对于数学期望的偏离程度）
        uint64_t stddev   = 0; // 标准差 sqrt(variance)

        // 分位数
        int64_t m10 = 0; // 10分位
        int64_t m25 = 0; // 25分位
        int64_t m50 = 0; // 中位数
        int64_t m75 = 0; // 75分位
        int64_t m90 = 0; // 90分位
        int64_t m95 = 0; // 95分位
        int64_t m99 = 0; // 99分位
    };

public:
    Rst& operator()(int64_t *arr, int64_t cnt);
    Rst& operator()(std::vector<int64_t> &vec);

    const char *err() const { return err_; }

private:
    void calc1(const int64_t *arr, int64_t cnt);
    // 方差, 标准差
    void calc_variance_stddev(const int64_t *arr, int64_t cnt);
    // percentile, 分位数 [0, 100]
    int64_t get_percentile(const int64_t *arr, int64_t cnt, int64_t percentile) const;

private:
    Rst  rst_;
    char err_[255] = {0};
};
