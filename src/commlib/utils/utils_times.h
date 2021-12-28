#pragma once

#include <chrono>
#include <time.h>
#include <sys/time.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 获取时间
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsClock {
public:
    static uint64_t get_current_cpu_cycles() {
        uint32_t lo = 0, hi = 0;
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
        return (((uint64_t)hi << 32) | lo);
    }

    static uint64_t get_us() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    static uint64_t get_ms() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec/1000;
    }

    static uint64_t get_second() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec;
    }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 格式化时间
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsTimefmt {
public:
    // out: 20210518-12:13:14
    static bool format(const time_t &t, char *out) {
        struct tm p;
        if (localtime_r(&t, &p)) {
            sprintf(out, "%04d%02d%02d-%02d:%02d:%02d",
                    p.tm_year+1900, p.tm_mon+1, p.tm_mday,
                    p.tm_hour, p.tm_min, p.tm_sec);
            return true;
        }
        return false;
    }

    // out: 20210518-12:13:14.056253
    static bool format(const timeval &tv, char *out) {
        struct tm p;
        if (localtime_r(&tv.tv_sec, &p)) {
            sprintf(out, "%04d%02d%02d-%02d:%02d:%02d.%06ld",
                    p.tm_year+1900, p.tm_mon+1, p.tm_mday,
                    p.tm_hour, p.tm_min, p.tm_sec,
                    tv.tv_usec);
            return true;
        }
        return false;
    }

    // out: 20210518-12:13:14
    static bool get_now1(char *out) {
        time_t t;
        time(&t);
        return format(t, out);
    }

    // out: 20210518-12:13:14.056253
    static bool get_now2(char *out) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return format(tv, out);
    }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//         CPU时钟相关
// 说明: TSC, time stamp counter
// (1) 多核心的CPU， 每个核的频率可能不同;
// (2) 核的频率可能会变。 如功耗原因，降频使用
// (3) 多核CPU，每个核的TSC初值可能不同
// (4) CPU乱序执行
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsCycles {
public:
    UtilsCycles() = delete;
    UtilsCycles(const UtilsCycles&) = delete;

    // 计算一秒多少个cpu cycles
    static void init();
   
    // 获取当前CPU cycles
    static uint64_t rdtsc() {
        uint32_t lo = 0, hi = 0;
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
        return (((uint64_t)hi << 32) | lo);        
    }

    static double cycles_per_second()      { return cycles_per_second_; }
    static double cycles_pre_milisecond()  { return cycles_per_milisecond_; }
    static double cycles_per_microsecond() { return cycles_per_microsecond_; }
    static double cycles_per_nanosecond()  { return cycles_per_nanosecond_; }

    // 将cycle换成秒
    static double cycles_to_second(uint64_t cycles)      { return (double)cycles / cycles_per_second(); }
    static double cycles_to_milisecond(uint64_t cycles)  { return (double)cycles / cycles_pre_milisecond(); }
    static double cycles_to_microsecond(uint64_t cycles) { return (double)cycles / cycles_per_microsecond(); }
    static double cycles_to_nanosecond(uint64_t cycles)  { return (double)cycles / cycles_per_nanosecond(); }

    // sleep 微秒
    static void sleep(uint64_t us) {
        const uint64_t s = us * cycles_per_microsecond() + rdtsc();
        while (rdtsc() < s) { }
    }

private:
    static double cycles_per_second_;      // 每 秒多少个CPU cycle.
    static double cycles_per_milisecond_;  // 每毫秒多少个CPU cycle.
    static double cycles_per_microsecond_; // 每微秒多少个CPU cycle.
    static double cycles_per_nanosecond_;  // 每纳秒多少个CPU cycle.
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测量时间
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsTimeElapse {
public:
    void start() { t_ = std::chrono::high_resolution_clock::now(); }

    // return ns.
    int64_t stop_ns() {
        std::chrono::high_resolution_clock::time_point n = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::nanoseconds>(n-t_)).count();
    }

    // return us.
    int64_t stop_us() {
        std::chrono::high_resolution_clock::time_point n = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::microseconds>(n-t_)).count();
    }

private:
    std::chrono::high_resolution_clock::time_point t_;
};
