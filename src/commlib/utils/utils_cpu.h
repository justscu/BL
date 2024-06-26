#pragma once

#include <sched.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>


// cpuid start with 0.
inline void bind_thread_to_cpu(int32_t cpuid) {
#ifndef __APPLE__
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET (cpuid, &set);

    sched_setaffinity((pid_t)syscall(SYS_gettid), sizeof(set), &set);
#endif
}


inline void cpu_delay(uint64_t delay)
{
    for (uint64_t i = 0; i < delay; i++)
    {
#ifdef _WIN32
        __nop();
#else
        __asm__ __volatile__("rep;nop" : : : "memory");
#endif
    }
}


inline int32_t sched_cpuid() {
    return sched_getcpu();
}
