#pragma once

#include <stdint.h>

using UtilsBenchMarkFunc = double(*)();

struct BenchMarkTestInfo {
    UtilsBenchMarkFunc func;
    const char        *func_name;
    double             func_cost[10]; // func run 10-times.
    const char        *func_desc;
};


void utils_benchmark_func(BenchMarkTestInfo tests[], const uint32_t tests_cnt);
