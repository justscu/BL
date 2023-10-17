#include <stdint.h>
#include <stdio.h>
#include <string>
#include "utils.h"
#include "rtt_test.h"

void utils_queue_test();
void utils_queue_full_test();

int32_t ttl_test(int32_t argc, char **argv);
void Utils_test_Sta();
void Utils_test_cpu();
void StrUtils_test();
void utils_benchmark_test();

int32_t main(int32_t argc, char **argv) {
    utils_queue_test();
    utils_queue_full_test();
    //utils_benchmark_test();
    //return ttl_test(argc, argv);
    // utils_queue_test();
    // Utils_test_cpu();

    return 0;
}

