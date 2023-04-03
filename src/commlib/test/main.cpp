#include <stdint.h>
#include <stdio.h>
#include <string>
#include "utils.h"
#include "rtt_test.h"

void utils_queue_test();

int32_t ttl_test(int32_t argc, char **argv);
void Utils_test_Sta();

int32_t main(int32_t argc, char **argv) {
    return ttl_test(argc, argv);
    // utils_queue_test();

    return 0;
}

