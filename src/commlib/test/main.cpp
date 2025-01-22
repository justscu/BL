#include <stdint.h>
#include <stdio.h>
#include <string>
#include <type_traits>
#include <iostream>
#include <typeinfo>
#include <vector>
#include <atomic>
#include "utils.h"
#include "rtt_test.h"

extern void mulicast_test(int32_t argc, char **argv);

#if 1
void utils_queue_test();
void utils_queue_full_test();


int32_t ttl_test(int32_t argc, char **argv);
void Utils_test_Sta();
void Utils_test_cpu();
void StrUtils_test();
void utils_benchmark_test();
void utils_bitmap_test();
void utils_mempool_test();

int32_t main(int32_t argc, char **argv) {
    utils_mempool_test();
}

#endif


