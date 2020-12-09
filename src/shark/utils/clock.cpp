#include "clock.h"

// cpu's timestamp counter. Return 0 if it's not available.
uint64_t Clock::rdtsc() const {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((static_cast<uint64_t> (high)) << 32) | low;
}

// high precision timestamp.
uint64_t Clock::get_us() const {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * usecs_per_msec + tv.tv_usec;
}

// low precision timestamp. 
// In tight loops, 10X faster than high precision timestamp.
uint64_t Clock::get_ms() {
    uint64_t tsc = rdtsc();
    // not support TSC.
    if (tsc == 0) {
        return now_us() / usecs_per_msec;
    }

    // 2GHZ cpu, 2000000 cycles per 1ms.
    const int64_t cycle = 2000000;
    // If TSC haven't jumped back (in case of migration to a different CPU core),
    // and If not too much time elapsed since last measurement,
    // return the cached time value.
    if (likely((tsc >= last_tsc) && (tsc - last_tsc_ <= cycle / 2))) {
        return last_time_;
    }

    last_tsc_  = tsc;
    last_time_ = now_us() / usecs_per_msec;
    return last_time_;
}

