#pragma once

#include <stdint.h>

class Algorithm {

public:
    // 累加
    static int8_t accumulate(const int8_t *str, const int64_t size);
    static int8_t accumulate_by_avx2(const int8_t *str, const int64_t size);

    // 乘法
    static void mul_double(double *d, const int64_t size, const double dbl);
    static void mul_by_avx2(double &d1, double &d2, double &d3, double &d4, const double dbl);
    // 乘法, d为double[4]的数组
    static void mul_double_by_avx2(double d[4], const double ble);
    // 乘法, d为double数组
    static void mul_double_by_avx2(double *d, const int64_t size, const double dbl);

    // 除法, d为int64_t数组，size为数组长度, d[i] = d[i] / b;
    static void div_int64(int64_t *d, const int64_t size, const int64_t dbl);
    static void div_int64_by_avx2(int64_t *d, const int64_t size, const int64_t dbl);

    // 用随机数填充
    static void filled_by_random(char   *str, const int64_t size);
    static void filled_by_random(double *dbl, const int64_t size);
    static void filled_by_random(int64_t  *d, const int64_t size);
};
