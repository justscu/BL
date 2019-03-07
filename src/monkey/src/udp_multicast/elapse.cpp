#include "elapse.h"

int64_t XElapse::stop_ns() {
    std::chrono::high_resolution_clock::time_point n = std::chrono::high_resolution_clock::now();
    return (std::chrono::duration_cast<std::chrono::nanoseconds>(n-t)).count();
}

int64_t XElapse::stop_us() {
    std::chrono::high_resolution_clock::time_point n = std::chrono::high_resolution_clock::now();
    return (std::chrono::duration_cast<std::chrono::microseconds>(n-t)).count();
}
