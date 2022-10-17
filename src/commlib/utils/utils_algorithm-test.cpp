#include <new>
#include <stdio.h>
#include <assert.h>
#include "utils_algorithm.h"
#include "utils_cpu.h"
#include "utils_times.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 实测下来，
// (1)除了累加使用avx快，其余均没有明显优势
// (2)除法比乘法需要更多的cpu cycle.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const int32_t cpuid = 17;
static const int64_t  size = 457823l - 457823l % 4;
static const double    dbl = 0.001;

// 测试加法，测试速度
void Algorithm_test_accumulate() {
    bind_thread_to_cpu(cpuid);
    UtilsCycles::init();

    for (int32_t i = 0; i < 10; ++i) {
        const int64_t size = 253+101*i;
        int8_t *buf = new (std::nothrow) int8_t[size];
        if (!buf) {
            fprintf(stderr, "new failed. \n");
            return;
        }

        Algorithm::filled_by_random((char*)buf, size);

        uint64_t beg1 = UtilsCycles::rdtsc();
        int8_t   sum1 = Algorithm::accumulate(buf, size);
        uint64_t end1 = UtilsCycles::rdtsc();

        uint64_t beg2 = UtilsCycles::rdtsc();
        int8_t   sum2 = Algorithm::accumulate_by_avx2(buf, size);
        uint64_t end2 = UtilsCycles::rdtsc();

        fprintf(stdout, "accumulate         : sum1[%4d]. cost1[%12.3f]us \n", sum1, UtilsCycles::cycles_to_microsecond(end1-beg1));
        fprintf(stdout, "accumulate_by_avx2 : sum2[%4d]. cost2[%12.3f]us \n", sum2, UtilsCycles::cycles_to_microsecond(end2-beg2));

        assert(sum1 == sum2);

        delete [] buf;
    }
}

bool equal(double d1, double d2) {
    return ((d1 - d2) > -0.0001) && ((d1 - d2) < 0.0001);
}

double sum(const double *d, int64_t size) {
    double ret = d[0];
    for (int64_t i = 1; i < size; ++i) {
        ret += d[i];
    }
    return ret;
}

// 测试乘法正确性
void Algorithm_test1_mul() {
    double *buf1 = new (std::nothrow) double[size];
    double *buf2 = new (std::nothrow) double[size];
    double *buf3 = new (std::nothrow) double[size];
    double *buf4 = new (std::nothrow) double[size];
    if (!buf1 || !buf2 || !buf3 || !buf4) {
        fprintf(stderr, "new failed. \n");
        return;
    }

    bind_thread_to_cpu(cpuid);
    UtilsCycles::init();

    Algorithm::filled_by_random(buf1, size);
    memcpy(buf2, buf1, sizeof(double)*size);
    memcpy(buf3, buf1, sizeof(double)*size);
    memcpy(buf4, buf1, sizeof(double)*size);

    Algorithm::mul_double(buf1, size, dbl);
    for (int64_t i = 0; i < size; i+=4) {
        Algorithm::mul_by_avx2(buf2[i], buf2[i+1], buf2[i+2], buf2[i+3], dbl);
        Algorithm::mul_double_by_avx2(buf3+i, dbl);
    }
    Algorithm::mul_double_by_avx2(buf4, size, dbl);

    //
    for (int64_t i = 0; i < size; ++i) {
        assert(equal(buf1[i], buf2[i]));
        assert(equal(buf1[i], buf3[i]));
        assert(equal(buf1[i], buf4[i]));
    }

    delete [] buf1;
    delete [] buf2;
    delete [] buf3;
    delete [] buf4;
}

// 测试耗时
void Algorithm_test2_mul() {
    double *buf1 = new (std::nothrow) double[size];
    if (!buf1) {
        fprintf(stderr, "new failed. \n");
        return;
    }
    fprintf(stdout, "buf1[%p] \n", buf1);

    bind_thread_to_cpu(cpuid);
    UtilsCycles::init();

    uint64_t beg = 0, end = 0;
    double result = 0;

    Algorithm::filled_by_random(buf1, size);
    beg = UtilsCycles::rdtsc();
    Algorithm::mul_double(buf1, size, dbl);
    end = UtilsCycles::rdtsc();
    result = sum(buf1, size);
    fprintf(stdout, "Algorithm::mul_double             : cost1[%12.1f]ns [%15.2f] \n", UtilsCycles::cycles_to_nanosecond(end-beg), result);

    sleep(1);
    Algorithm::filled_by_random(buf1, size);
    beg = UtilsCycles::rdtsc();
    for (int64_t i = 0; i < size; i+=4) {
        Algorithm::mul_by_avx2(buf1[i+0], buf1[i+1], buf1[i+2], buf1[i+3], dbl);
    }
    end = UtilsCycles::rdtsc();
    result = sum(buf1, size);
    fprintf(stdout, "Algorithm::mul_by_avx2            : cost1[%12.1f]ns [%15.2f]. \n", UtilsCycles::cycles_to_nanosecond(end-beg), result);

    sleep(1);

    sleep(1);
    Algorithm::filled_by_random(buf1, size);
    beg = UtilsCycles::rdtsc();
    for (int64_t i = 0; i < size; i+=4) {
        Algorithm::mul_double_by_avx2(buf1+i, dbl);
    }
    end = UtilsCycles::rdtsc();
    result = sum(buf1, size);
    fprintf(stdout, "Algorithm::mul_double_by_avx2     : cost1[%12.1f]ns [%15.2f] \n", UtilsCycles::cycles_to_nanosecond(end-beg), result);

    sleep(1);
    Algorithm::filled_by_random(buf1, size);
    beg = UtilsCycles::rdtsc();
    Algorithm::mul_double_by_avx2(buf1, size, dbl);
    end = UtilsCycles::rdtsc();
    result = sum(buf1, size);
    fprintf(stdout, "Algorithm::mul_double_by_avx2(arr): cost1[%12.1f]ns [%15.2f] \n", UtilsCycles::cycles_to_nanosecond(end-beg), result);

    delete [] buf1;
}

void Algorithm_test1_div() {
    const int64_t  dbl = 1000;
    int64_t *buf1 = new (std::nothrow) int64_t[size];
    if (!buf1) {
        fprintf(stderr, "new failed. \n");
        return;
    }

    uint64_t beg = 0, end = 0;
    bind_thread_to_cpu(cpuid);
    UtilsCycles::init();

    Algorithm::filled_by_random(buf1, size);
    beg = UtilsCycles::rdtsc();
    Algorithm::div_int64(buf1, size, dbl);
    end = UtilsCycles::rdtsc();
    fprintf(stdout, "Algorithm::div_int64              : cost1[%12.1f]ns \n", UtilsCycles::cycles_to_nanosecond(end-beg));

    delete [] buf1;
}
