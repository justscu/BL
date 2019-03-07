#ifndef __ELAPSE_H__
#define __ELAPSE_H__

#include <chrono>

class XElapse {
public:
    inline
    void start() {
        t = std::chrono::high_resolution_clock::now();
    }

    int64_t stop_ns(); // return ns.
    int64_t stop_us(); // return us.

private:
    std::chrono::high_resolution_clock::time_point t;
};

// cycle
inline void cpu_delay(int64_t cycle) {
    for (int64_t k = 0; k < cycle; ++k) {
        __asm__ __volatile__("rep;nop" : : : "memory");
    }
}

#endif // __ELAPSE_H__
