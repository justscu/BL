#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils_times.h"


double UtilsCycles::cycles_per_second_      = 0.0;
double UtilsCycles::cycles_per_milisecond_  = 0.0;
double UtilsCycles::cycles_per_microsecond_ = 0.0;
double UtilsCycles::cycles_per_nanosecond_  = 0.0;

void UtilsCycles::init() {
    if (cycles_per_second_ > 0.0) { return; }

    struct timeval start_tm, stop_tm;

    uint64_t old = 0;
    while (true) {
        if (0 != gettimeofday(&start_tm, nullptr)) {
            fprintf(stderr, "gettimeofday error. [%s] \n", strerror(errno));
            abort();
        }

        uint64_t start_cycles = rdtsc();

        while (true) {
            if (0 != gettimeofday(&stop_tm, nullptr)) {
                fprintf(stderr, "gettimeofday error. [%s] \n", strerror(errno));
                abort();
            }
            uint64_t stop_cycles = rdtsc();

            uint64_t m = (stop_tm.tv_sec - start_tm.tv_sec) * 1000000 + (stop_tm.tv_usec - start_tm.tv_usec);
            if (m > 10000) {
                cycles_per_second_ = ((double(stop_cycles-start_cycles)) / (double)(m)) * 1000000;
                break;
            }
        } // while
        double delta = cycles_per_second_ / 100000.0;
        if (old > (cycles_per_second_ - delta) && old < (cycles_per_second_ + delta)) {
            // set values
            cycles_per_milisecond_  = cycles_per_second_ / 1000.0;
            cycles_per_microsecond_ = cycles_per_second_ / 1000000.0;
            cycles_per_nanosecond_  = cycles_per_microsecond_ / 1000.0;

            break;
        }

        old = cycles_per_second_;
    } // while
}
