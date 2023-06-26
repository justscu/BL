#include <immintrin.h>
#include <x86intrin.h>
#include <random>
#include <string.h>
#include "utils_algorithm.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 使用AVX2指令进行加速，需要makefile中增加`-mavx2`
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 逐字节累加
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int8_t Algorithm::accumulate(const int8_t *str, const int64_t str_len) {
    int8_t ret = 0;
    for (int64_t i = 0; i < str_len; ++i) {
        ret += str[i];
    }
    return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 逐字节累加
// 使用AVX2进行计算，每次计算32字节(256bit). 比accumulate快6倍
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int8_t Algorithm::accumulate_by_avx2(const int8_t *str, const int64_t str_len) {
    int8_t ret = 0;

    const int64_t STEP = 32;
    const int64_t  cycle = str_len / STEP;
    const int64_t remain = str_len % STEP;

    const __m256i *addr = (const __m256i*)str;
    if (cycle > 0) {
        __m256i mid = _mm256_loadu_si256(addr++);
        for (int64_t i = 1; i < cycle; ++i) {
            mid = _mm256_add_epi8(mid, _mm256_loadu_si256(addr++));
        }

        const int8_t *v = (const int8_t*)&mid;
        for (int32_t i = 0; i < STEP; ++i) {
            ret += v[i];
        }
    }

    if (remain > 0) {
        const int8_t *r = (const int8_t*)addr;
        for (int32_t i = 0; i < remain; ++i) {
            ret += r[i];
        }
    }

    return ret;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d为double数组, d[0] *= ble, d[1] *= ble, d[2] *= ble, d[3] *= ble
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::mul_double(double *d, const int64_t size, const double dbl) {
    for (int64_t i = 0; i < size; ++i) {
        d[i] *= dbl;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d1 *= b; d2  *= b; d3 *= b; d4 *= b;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::mul_by_avx2(double &d1, double &d2, double &d3, double &d4, const double ble) {
    __m256d rst = _mm256_mul_pd(_mm256_set_pd(d4, d3, d2, d1), _mm256_set1_pd(ble));

    const double *p = (const double *)&rst;
    d1 = p[0];
    d2 = p[1];
    d3 = p[2];
    d4 = p[3];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d为double[4]数组, d[0] *= ble, d[1] *= ble, d[2] *= ble, d[3] *= ble
// cost: 0.7 ns(-o2)
// 使用_mm256_loadu_pd的话，耗时会扩大到7.3ns
// 使用memcpy，耗时会扩大到3.7ns
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::mul_double_by_avx2(double d[4], const double ble) {
#if 0

    __m256d rst = _mm256_mul_pd(_mm256_loadu_pd(d), _mm256_set1_pd(ble));
    memcpy(d, (const double*)&rst, sizeof(double)*4);

#else

    __m256d rst = _mm256_mul_pd(_mm256_set_pd(d[3], d[2], d[1], d[0]), _mm256_set1_pd(ble));

    const double *p = (const double *)&rst;
    d[0] = p[0];
    d[1] = p[1];
    d[2] = p[2];
    d[3] = p[3];
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d为double数组, d[0] *= ble, d[1] *= ble, d[2] *= ble, d[3] *= ble
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::mul_double_by_avx2(double *d, const int64_t size, const double dbl) {
    const int64_t   STEP = 4;
    const int64_t  cycle = size / STEP;
    const int64_t remain = size % STEP;

    double *addr = d;
    if (cycle > 0) {
        __m256d k = _mm256_set1_pd(dbl);
        for (int64_t i = 0; i < cycle; ++i) {
            __m256d tmp = _mm256_mul_pd(_mm256_set_pd(double(addr[3]), double(addr[2]), double(addr[1]), double(addr[0])), k);

            double *rst = (double*)&tmp;
            addr[0] = rst[0];
            addr[1] = rst[1];
            addr[2] = rst[2];
            addr[3] = rst[3];

            addr += 4;
        }
    }

    for (int64_t i = 0; i < remain; ++i) {
        addr[i] *= dbl;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d为int64类型整数，d[i] = d[i] * dbl. size为数组长度
// cost: 2.2ns(-o2) 每个除法计算
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::div_int64(int64_t *d, const int64_t size, const int64_t dbl) {
    for (int64_t i = 0; i < size; ++i) {
        d[i] /= dbl;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// d为int64类型整数，d[i] = d[i] * dbl. size为数组长度
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Algorithm::div_int64_by_avx2(int64_t *d, const int64_t size, const int64_t dbl) {
#if 0
    const int64_t   STEP = 4;
    const int64_t  cycle = size / STEP;
    const int64_t remain = size % STEP;

    int64_t *addr = d;
    if (cycle > 0) {
        __m256i k = _mm256_set1_epi64x(dbl);
        for (int64_t i = 0; i < cycle; ++i) {
            __m256i tmp = _mm256_div_epi16(_mm256_set_epi64x(addr[3], addr[2], addr[1], addr[0]), k);
            // __m256d tmp = _mm256_mul_pd(_mm256_set_pd(double(addr[3]), double(addr[2]), double(addr[1]), double(addr[0])), k);

            int64_t *rst = (int64_t*)&tmp;
            addr[0] = rst[0];
            addr[1] = rst[1];
            addr[2] = rst[2];
            addr[3] = rst[3];

            addr += 4;
        }
    }

    for (int64_t i = 0; i < remain; ++i) {
        addr[i] /= dbl;
    }
#endif
}

void Algorithm::filled_by_random(char *str, const int64_t size) {
    std::default_random_engine e(time(nullptr));
    std::uniform_int_distribution<int32_t> u(-255, 255);

    for (int64_t i = 0; i < size; ++i) {
        str[i] = (char)u(e);
    }
}

void Algorithm::filled_by_random(double *dbl, const int64_t size) {
    std::default_random_engine e(time(nullptr));
    std::uniform_real_distribution<double> u(-1000000, 1000000);

    for (int64_t i = 0; i < size; ++i) {
        dbl[i] = u(e);
    }
}

void Algorithm::filled_by_random(int64_t *d, const int64_t size) {
    std::default_random_engine e(time(nullptr));
    std::uniform_int_distribution<int64_t> u(-1000000, 1000000);

    for (int64_t i = 0; i < size; ++i) {
        d[i] = u(e);
    }
}
