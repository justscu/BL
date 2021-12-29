#include <endian.h>
#include <cstdarg>
#include <sched.h>
#include <sys/syscall.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include "utils_times.h"
#include "utils_benchmark.h"

static void bind_thread_to_cpu(int32_t cpuid) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET (cpuid, &set);

    sched_setaffinity((pid_t)syscall(SYS_gettid), sizeof(set), &set);
}

static int32_t print_local(const double value) {
    if (value < 1.0e-06) {
        return printf("%8.2f ns", 1e09 * value);
    }
    else if (value < 1.0e-03) {
        return printf("%8.2f us", 1e06 * value);
    }
    else if (value < 1.0) {
        return printf("%8.2f ms", 1e03 * value);
    }
    else {
        return printf("%8.2f s", value);
    }
}

void utils_benchmark_func(BenchMarkTestInfo tests[], const uint32_t tests_cnt) {
    bind_thread_to_cpu(2);
    UtilsCycles::init();

    const uint32_t run_times = sizeof(tests[0].func_cost) / sizeof(tests[0].func_cost[0]);

    // run test.
    printf("每次计算需要多少时间 \n");
    for (uint32_t i = 0; i < run_times; ++i) {
        for (uint32_t j = 0; j < tests_cnt; ++j) {
            BenchMarkTestInfo &t = tests[j];
            const double sec = t.func();
            tests[j].func_cost[i] = sec;
        }
    }
    // run test.
    {
        printf("function name                  min        max       mean        mid \n");
        printf("--------------------------------------------------------------------\n");
        for (uint32_t i = 0; i < tests_cnt; ++i) {
            BenchMarkTestInfo &t = tests[i];

            std::sort(std::begin(t.func_cost), std::end(t.func_cost));

            const double min = t.func_cost[0];
            const double max = t.func_cost[9];
            const double mid = t.func_cost[4]; // 50 分位
            const double mean= std::accumulate(std::begin(t.func_cost), std::end(t.func_cost), 0.0) / (sizeof(t.func_cost)/sizeof(t.func_cost[0]));

            int32_t width = printf("%-23s", t.func_name);

            width += print_local(min);
            width += print_local(max);
            width += print_local(mean);
            width += print_local(mid);

            printf("%*s %s \n", 66-width, "", t.func_desc);
        }
    }
}
