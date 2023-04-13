#include <thread>
#include "utils_cpu.h"
#include "fmt/format.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 对进程绑核(进程中的所有线程) -- 逻辑核
// 整个进程的所有线程，均运行在5-6核心
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
void bind_process_self_to_cpu() {
    fmt::print("enter bind_process_self_to_cpu \n");

    // count of cpu cores.
    int32_t num = sysconf(_SC_NPROCESSORS_CONF);
    fmt::print("cpu cores count : {} \n", num);

    // 获取进程bind的核心
    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    int32_t ret = sched_getaffinity(0, sizeof(old_mask), &old_mask);
    if (ret) {
        fmt::print("sched_getaffinity failed: {}. \n", strerror(errno));
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        // 判断进程是否可以运行到该核心上
        if (CPU_ISSET(i, &old_mask)) {
            fmt::print("1. process {} already bind on cpu core {}. \n", syscall(SYS_getpid), i);
        }
    }

    // set
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(5, &mask); // <--- core 5
    CPU_SET(6, &mask); // <--- core 6
    ret = sched_setaffinity(0, sizeof(mask), &mask);
    if (ret) {
        fmt::print("sched_setaffinity(core 5/6) failed: {}. \n", strerror(errno));
        return;
    }

    // get
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    ret = sched_getaffinity(0, sizeof(new_mask), &new_mask);
    if (ret) {
        fmt::print("sched_getaffinity failed: {}. \n", strerror(errno));
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &new_mask)) {
            fmt::print("2. process {} new bind on cpu core {}. \n", syscall(SYS_getpid), i);
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 对某个线程绑核 -- 逻辑核
// 该线程在cpu_id上执行
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
void bind_thread_self_to_cpu(int32_t cpu_id) {
    fmt::print("enter bind_thread_self_to_cpu \n");

    int32_t num = sysconf(_SC_NPROCESSORS_CONF); // count of cpu cores.

    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    int32_t ret = pthread_getaffinity_np(pthread_self(), sizeof(old_mask), &old_mask);
    if (ret) {
        fmt::print("sched_getaffinity({}) failed: {}. \n", syscall(SYS_gettid), strerror(errno));
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &old_mask)) {
            fmt::print("thread {} already bind on cpu core {}. \n", syscall(SYS_gettid), i);
        }
    }

    // set
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    ret = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if (ret) {
        fmt::print("thread {} bind on cpu core failed: {}. \n", syscall(SYS_gettid), strerror(errno));
        return ;
    }

    // get
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    ret = pthread_getaffinity_np(pthread_self(), sizeof(new_mask), &new_mask);
    if (ret) {
        fmt::print("pthread_getaffinity_np failed: {}. \n", strerror(errno));
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &new_mask)) {
            fmt::print("thread {} new bind on cpu core {}. \n", syscall(SYS_gettid), i);
        }
    }
}


void Utils_test_cpu() {
    bind_process_self_to_cpu();
    std::thread th1([](){
        sleep(1);
        fmt::print("thread 1 \n");
        bind_thread_self_to_cpu(1);
        sleep(10);
    });


    std::thread th2([](){
        sleep(2);
        fmt::print("thread 2 \n");
        bind_thread_self_to_cpu(2);
        sleep(10);
    });
    th1.join();
    th2.join();
}
