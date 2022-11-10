#pragma once

#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
// 计算时间差值
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TimeCalc {
public:
    // out: us.
    static int64_t diff1(const timeval &beg, const timeval &end) {
        return (end.tv_sec - beg.tv_sec) * 1000000 + (end.tv_usec - beg.tv_usec);
    }

    // t1, t2: 20220607121338.123567
    // return : [t1 - t2] us.
    static int64_t diff2(const char *t1, const char *t2) {
        int64_t hh = (t1[ 8]-t2[ 8])*10 + (t1[ 9]- t2[ 9]);
        int64_t mm = (t1[10]-t2[10])*10 + (t1[11]- t2[11]);
        int64_t ss = (t1[12]-t2[12])*10 + (t1[13]- t2[13]);

        int64_t sss = atoi(t1+15) - atoi(t2+15);
        
        return (hh*3600 + mm*60 + ss) * 1000000 + sss;
    }

    // t1 : 20220607121338.123567 (us)
    // t2 : 20220607121338123 (ms)
    // return : [t1 - t2] us.
    static int64_t diff3(const char *t1, const char *t2) {
        int64_t hh = (t1[ 8]-t2[ 8])*10 + (t1[ 9]- t2[ 9]);
        int64_t mm = (t1[10]-t2[10])*10 + (t1[11]- t2[11]);
        int64_t ss = (t1[12]-t2[12])*10 + (t1[13]- t2[13]);

        int64_t sss = atoi(t1+15) - atoi(t2+14) * 1000;
        return (hh*3600 + mm*60 + ss) * 1000000 + sss;
    }

    // etm: yyyymmddHHMMSSsss (ms)
    // tv: local time
    // tv - etm: -> return us.
    static int64_t diff4(int64_t etm, const timeval &tv) {
        tm t = {0};
        localtime_r(&tv.tv_sec, &t);

        int64_t l_hour = t.tm_hour;
        int64_t l_min  = t.tm_min;
        int64_t l_sec  = t.tm_sec;
        int64_t l_usec = tv.tv_usec;

        int64_t e_hour = (etm / 10000000) % 100;
        int64_t e_min  = (etm / 100000) % 100;
        int64_t e_sec  = (etm / 1000) % 100;
        int64_t e_usec = (etm % 1000) * 1000;

        int64_t ret = (l_hour - e_hour) * 60 * 60 * 1000000
                    + (l_min  - e_min ) * 60 * 1000000
                    + (l_sec  - e_sec ) * 1000000
                    + (l_usec - e_usec);

        return ret;
    }

    // t:  yyyymmddhhMMss.sss
    //  or yyyymmddhhMMssSSS
    //  or yyyymmddhhMMss
    // 20220607150807.123456 -> 150807
    static int64_t hms(const char *t) {
        char buf[8];
        memcpy(buf, t+8, 6);
        buf[6] = 0;
        return atoi(buf);
    }

    // t: yyyymmddHHMMSSsss
    // return -> HHMMSS
    static int32_t hms(int64_t t) {
        t %= 1000000000;
        t /= 1000;
        return t;
    }

    // in(83733) -> out(31053);
    static int32_t today_second(int32_t hhmmss) {
       int32_t hh = hhmmss / 10000;
       int32_t mm = hhmmss % 10000 / 100;
       int32_t ss = hhmmss % 100;
       return hh * 60 * 60 + mm * 60 + ss;
    }

    // in(08:37:33) -> out(31053);
    static int32_t today_second(const char *hhmmss) {
       int32_t hh = (hhmmss[0] - '0') * 10 + (hhmmss[1] - '0');
       int32_t mm = (hhmmss[3] - '0') * 10 + (hhmmss[4] - '0');
       int32_t ss = (hhmmss[6] - '0') * 10 + (hhmmss[7] - '0');
       return hh * 60 * 60 + mm * 60 + ss;
    }

    static int32_t today_second(void) {
        time_t t;
        time(&t);

        struct tm p;
        localtime_r(&t, &p);
        return p.tm_hour * 60 * 60 + p.tm_min * 60 + p.tm_sec;
    }

    // us: 20220607121338.123567 -> 121338123567
    static int64_t today_us(const char *us) {
        int64_t t1 = 0;
        t1 += ((us[ 8]-'0') * 10 + (us[ 9]-'0')) * 3600;
        t1 += ((us[10]-'0') * 10 + (us[11]-'0')) *   60;
        t1 += ( us[12]-'0') * 10 + (us[13]-'0');

        return t1 *1000000 + atoi(us+15);
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

    // out: 20210518121314.056253
    static bool format2(const timeval &tv, char *out) {
        struct tm p;
        if (localtime_r(&tv.tv_sec, &p)) {
            sprintf(out, "%04d%02d%02d%02d%02d%02d.%06ld",
                    p.tm_year+1900, p.tm_mon+1, p.tm_mday,
                    p.tm_hour, p.tm_min, p.tm_sec,
                    tv.tv_usec);
            return true;
        }
        return false;
    }

    // out: 20210518-121314
    static bool format3(const time_t &t, char *out) {
        struct tm p;
        if (localtime_r(&t, &p)) {
            sprintf(out, "%04d%02d%02d-%02d%02d%02d",
                    p.tm_year+1900, p.tm_mon+1, p.tm_mday,
                    p.tm_hour, p.tm_min, p.tm_sec);
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

    // out: 20210518-12:13:14
    static bool get_now3(char *out) {
        time_t t;
        time(&t);
        return format3(t, out);
    }

    // out: 20221008
    static bool yyyymmdd(char *out) {
        time_t t;
        time(&t);

        struct tm p;
        if (localtime_r(&t, &p)) {
            sprintf(out, "%04d%02d%02d",
                    p.tm_year+1900, p.tm_mon+1, p.tm_mday);
            return true;
        }
        return false;
    }

    // return : YYYYYMMDDhhmmSSsss
    static uint64_t hmss() {
        uint64_t ret = 0;

        timeval tv = {0};
        gettimeofday(&tv, nullptr);
        struct tm p;
        if (localtime_r(&tv.tv_sec, &p)) {
            ret += ((uint64_t)p.tm_year + 1900) * 10000000000000;
            ret += ((uint64_t)p.tm_mon + 1) * 100000000000;
            ret += ((uint64_t)p.tm_mday) * 1000000000;
            ret += ((uint64_t)p.tm_hour) * 10000000;
            ret += ((uint64_t)p.tm_min)  * 100000;
            ret += ((uint64_t)p.tm_sec)  * 1000;
            ret += tv.tv_usec / 1000;
        }

        return ret;
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

    // Returns microseconds since epoch
    static uint64_t now_ns() {
        std::chrono::high_resolution_clock::duration n = std::chrono::high_resolution_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(n).count();
    }

    // Returns microseconds since epoch
    static uint64_t now_us() {
        std::chrono::high_resolution_clock::duration n = std::chrono::high_resolution_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::microseconds>(n).count();
    }
    // Returns milliseconds since epoch
    static uint64_t now_ms() {
        std::chrono::high_resolution_clock::duration n = std::chrono::high_resolution_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(n).count();
    }

private:
    std::chrono::high_resolution_clock::time_point t_;
};
