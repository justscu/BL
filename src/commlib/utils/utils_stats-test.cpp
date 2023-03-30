#include "utils_stats.h"
#include "fmt/format.h"

void Utils_test_Sta() {
    int64_t arr[100];
    for (int32_t i = 0; i < 30; ++i) {
        arr[i] = 2*i+5;
    }

    for (int32_t i = 0; i < 30; ++i) {
        arr[30+i] = (i*9+5)/3;
    }

    for (int32_t i = 0; i < 30; ++i) {
        arr[60+i] = (i*2+5)/2-50;
    }

    for (int32_t i = 0; i < 10; ++i) {
        arr[90+i] = (i*8+5)/2+8;
    }

    Sta sta;
    const Sta::Rst &rst = sta(arr, 100);

    for (int32_t i = 0; i < 10; ++i) {
        fmt::print("[{:>2}] ", i);
        for (int32_t j = 0; j < 10; ++j) {
            fmt::print("{:>4} ", arr[i*10+j]);
        }
        fmt::print("\n");
    }

    fmt::print("{} {} {} {} {} | {} {} {} {}\n",
            rst.cnt, rst.min, rst.max, rst.avg, rst.stddev,
            rst.m50, rst.m90, rst.m95, rst.m99);
}
