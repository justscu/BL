#ifndef __XTIMER_H__
#define __XTIMER_H__

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

class Timer {
public:
    bool init() {
        double old_cycle = 0.0f;
        timeval start_tv, stop_tv;
        while (true) {
            if (0 != gettimeofday(&start_tv, nullptr)) {
                fprintf(stderr, "gettimeofday failed. %s. \n", strerror(errno));
                return false;
            }
            uint64_t start_cycle = rdtsc();
            while (true) {
                if (0 != gettimeofday(&stop_tv, nullptr)) {
                    fprintf(stderr, "gettimeofday failed. %s. \n", strerror(errno));
                    return false;
                }
                uint64_t stop_cycle = rdtsc();
                uint64_t us = (stop_tv.tv_sec - start_tv.tv_sec) * 1000000 + (stop_tv.tv_usec - start_tv.tv_usec);
                // 10ms
                if (us > 10000) {
                    cycles_per_second_ = static_cast<double>(stop_cycle - start_cycle) * 1000000.0 / static_cast<double>(us);
                    break;
                }
            } // while 2
            double delta = cycles_per_second_ / 100000.0;
            if ((old_cycle > (cycles_per_second_ - delta)) && (old_cycle < (cycles_per_second_ + delta))) {
                cycles_pre_nanosecond_ = cycles_per_second_ / 1000000000.0f;
                return true;
            }
            old_cycle = cycles_per_second_;
        } // while1
    }

    inline double cycles_to_seconds(uint64_t cycles) const {
        return static_cast<double>(cycles) / cycles_per_second_;
    }

    // 微秒
    inline uint64_t microseconds_to_cycles(uint64_t microseconds) const {
        return static_cast<uint64_t>(cycles_pre_nanosecond() * microseconds * 1000 + 0.5);
    }

    // micro-second : 10e-6
    inline void sleep_microseconds(uint64_t microseconds) const {
        const int64_t c = microseconds_to_cycles(microseconds);
        const uint64_t stop = rdtsc() + c;
        while (stop > rdtsc()) {
            ;
        }
    }

    inline uint64_t rdtsc() const {
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
        return (((uint64_t)hi << 32) | lo);
    }

    inline double cycles_per_second() const { return cycles_per_second_; }
    inline double cycles_pre_nanosecond() const { return cycles_pre_nanosecond_; }
private:
    double cycles_per_second_      = 0.0f;
    double cycles_pre_nanosecond_ = 0.0f;
};

#endif // __XTIMER_H__
