#include <stdint.h>
#include <stdio.h>
#include <string>
#include <type_traits>
#include <iostream>
#include <typeinfo>
#include <vector>
#include <atomic>
//#include "utils.h"
//#include "rtt_test.h"

#if 1
void utils_queue_test();
void utils_queue_full_test();


int32_t ttl_test(int32_t argc, char **argv);
void Utils_test_Sta();
void Utils_test_cpu();
void StrUtils_test();
void utils_benchmark_test();

int32_t main(int32_t argc, char **argv) {
//    utils_queue_test();
//    utils_queue_full_test();
    //utils_benchmark_test();
    //return ttl_test(argc, argv);
    // utils_queue_test();
    // Utils_test_cpu();

    std::atomic<uint32_t> m{1};

    uint32_t v1 = m;
    uint32_t v2 = m.fetch_add(1);

    fprintf(stdout, "%d, %d. \n", v1, v2);

    return 0;
}

#endif


