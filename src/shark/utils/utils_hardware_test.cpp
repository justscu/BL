#include <cstdarg>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>
#include <string.h>
#include <mutex>
#include "utils_times.h"
#include "utils_hardware_test.h"


// 绑定CPU
static
void bind_thread_to_cpu(int32_t cpuid) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET (cpuid, &set);

    sched_setaffinity((pid_t)syscall(SYS_gettid), sizeof(set), &set);
}

// 使用value，编译器觉得该值被使用了
// 防止被编译器优化掉
static
void discard_value(void *value) {
    int64_t x = *(int64_t*)value;
    if (x == 0x43924776) {
        fprintf(stdout, "value is 0x%lx \n", x);
    }
}

/**
 * This function is used to seralize machine instructions so that no
 * instructions that appear after it in the current thread can run before any
 * instructions that appear before it.
 *
 * It is useful for putting around rdpmc instructions (to pinpoint cache
 * misses) as well as before rdtsc instructions, to prevent time pollution from
 * instructions supposed to be executing before the timer starts.
 */
// 该函数用于机器指令的串行化: 在该指令后的指令执行 一定晚于 在该指令前的指令
// 该指令适用于 rdpmc, rdtsc.
static
void serialize() {
    uint32_t eax, ebx, ecx, edx;
    __asm volatile("cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (1U));
}

////////////////////////////////////
// 自定义加法
////////////////////////////////////
int64_t sum4(int64_t a, int64_t b, int64_t c, int64_t d) {
    return (a+b+c+d);
}

double add_func() {
    int32_t   cnt = 10000*10000;
    int64_t total = 0;

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        total += sum4(i, i, i, i);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    discard_value(&total);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

double add_func_withmutex() {
    int32_t   cnt = 10000*10000;
    int64_t total = 0;

    std::mutex mutex;
    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        mutex.lock();
        total += sum4(i, i, i, i);
        mutex.unlock();
    }
    const uint64_t end = UtilsCycles::rdtsc();

    discard_value(&total);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

////////////////////////////////////
// 递归加法 (模板)
////////////////////////////////////
template <typename T>
T template_sum(T v) {
    return v;
}

template<typename T, typename... Args>
T template_sum(T first, Args... args) {
    return first + template_sum(args...);
}

double add_templates() {
    int32_t   cnt = 10000*10000;
    int64_t total = 0;

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        total += template_sum(4, i, i, i, i);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    discard_value(&total);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}


////////////////////////////////////
// 宏定义加法
////////////////////////////////////
int64_t va_arg_sum(int32_t count, ...) {
    int64_t ret = 0;
    va_list args;
    va_start(args, count);
    for (int32_t i = 0; i < count; ++i) {
        ret += va_arg(args, int64_t);
    }
    va_end(args);
    return ret;
}

double add_va_args() {
    int32_t   cnt = 10000*10000;
    int64_t total = 0;

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        total += va_arg_sum(4, i, i, i, i);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    discard_value(&total);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

////////////////////////////////////
// 直接对数组赋值
////////////////////////////////////
double array_push() {
    const int32_t cnt = 10000 * 100;
    char *base = (char*)malloc(cnt * 4 * sizeof(uint64_t));
    char *buff = base;

    const uint64_t beg = UtilsCycles::rdtsc();
    for (uint32_t i = 0; i < cnt; ++i) {
        *(uint64_t*)buff = 1;
        buff += sizeof(uint64_t);

        *(uint64_t*)buff = 2;
        buff += sizeof(uint64_t);

        *(uint64_t*)buff = 3;
        buff += sizeof(uint64_t);

        *(uint64_t*)buff = 4;
        buff += sizeof(uint64_t);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    free(base);

    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

struct QuadNums {
    uint64_t num1;
    uint64_t num2;
    uint64_t num3;
    uint64_t num4;
};

////////////////////////////////////
// 转成数据结构后，再对数组赋值
////////////////////////////////////
double array_structcast() {
    const int32_t cnt = 10000 * 100;
    char *base = (char*)malloc(cnt * 4 * sizeof(uint64_t));
    char *buff = base;

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        QuadNums *v = (QuadNums*)buff;
        v->num1 = 1;
        v->num2 = 2;
        v->num3 = 3;
        v->num4 = 4;

        buff += sizeof(QuadNums);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

////////////////////////////////////
// 计算一次随机拷贝的时间
////////////////////////////////////
double memcpy_random_size(const uint32_t copy_size) {
    const int32_t cnt = 10000 * 100;
    uint32_t src[cnt], dst[cnt];
    int32_t buff_size = 1024 * 1024 * 1024; // 1G
    char *buf = (char*)malloc(buff_size);

    const uint32_t bound = buff_size - copy_size;
    for (int32_t i = 0; i < cnt; ++i) {
        src[i] = rand() % bound;
        dst[i] = rand() % bound;
    }

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        memcpy(buf+dst[i], buf+src[i], copy_size);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    free(buf);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

double memcpy_random_4() {
    return memcpy_random_size(4);
}

double memcpy_random_8() {
    return memcpy_random_size(8);
}

double memcpy_random_16() {
    return memcpy_random_size(16);
}

double memcpy_random_1K() {
    return memcpy_random_size(1*1024);
}

double memcpy_random_2K() {
    return memcpy_random_size(2*1024);
}

double memcpy_random_4K() {
    return memcpy_random_size(4*1024);
}

double memcpy_random_8K() {
    return memcpy_random_size(8*1024);
}


////////////////////////////////////
// 计算一次memset的时间
////////////////////////////////////
double memset_random_size(const uint32_t set_size) {
    const int32_t cnt = 10000 * 100;
    uint32_t src[cnt];
    int32_t buff_size = 1024 * 1024 * 1024; // 1G
    char *buf = (char*)malloc(buff_size);

    const uint32_t bound = buff_size - set_size;
    for (int32_t i = 0; i < cnt; ++i) {
        src[i] = rand() % bound;
    }

    const uint64_t beg = UtilsCycles::rdtsc();
    for (int32_t i = 0; i < cnt; ++i) {
        memset(buf+src[i], 0x00, set_size);
    }
    const uint64_t end = UtilsCycles::rdtsc();

    free(buf);
    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}

double memset_random_4() {
    return memset_random_size(4);
}

double memset_random_8() {
    return memset_random_size(8);
}

double memset_random_16() {
    return memset_random_size(16);
}

double memset_random_1K() {
    return memset_random_size(1*1024);
}

double memset_random_2K() {
    return memset_random_size(2*1024);
}

double memset_random_4K() {
    return memset_random_size(4*1024);
}

double memset_random_8K() {
    return memset_random_size(8*1024);
}

double memset_random_16K() {
    return memset_random_size(16*1024);
}
////////////////////////////////////
// 计算一次snprintf的时间
////////////////////////////////////
double snprintf_cost() {
    int32_t cnt = 10000 * 100;
    char buf[1024];

    const uint64_t beg = UtilsCycles::rdtsc();
    serialize();
    for (int i = 0; i < cnt; ++i) {
        snprintf(buf, sizeof(buf),
            "%s:%d:%s",
            __FILE__, __LINE__, __func__);
    }
    serialize();
    const uint64_t end = UtilsCycles::rdtsc();

    return UtilsCycles::cycles_to_second(end - beg) / cnt;
}


///////////////////////////////////////////
using TestInfoFunc = double (*)();

struct TestInfo {
    TestInfoFunc     func;
    const char *func_name;
    const char *func_desc;
};

TestInfo tests[] = {
        {add_func,           "add_custom_func",    "自定义加法"},
        {add_func_withmutex, "add_func_withmutex", "自定义加法(with mutex)"},
        {add_templates,      "add_templates",      "递归加法"},
        {add_va_args,        "add_va_args",        "宏定义加法"},

        {array_push,      "array_push",       "数组: 直接赋值"},
        {array_structcast,"array_struct_cast","数组: 转换成struct后再赋值"},

        {memcpy_random_4,  "memcpy_random",   "随机memcpy_4 bytes"},
        {memcpy_random_8,  "memcpy_random",   "随机memcpy_8 bytes"},
        {memcpy_random_16, "memcpy_random",   "随机memcpy_16 bytes"},
        {memcpy_random_1K, "memcpy_random",   "随机memcpy_1K"},
        {memcpy_random_2K, "memcpy_random",   "随机memcpy_2K"},
        {memcpy_random_4K, "memcpy_random",   "随机memcpy_4K"},
        {memcpy_random_8K, "memcpy_random",   "随机memcpy_8K"},

        {memset_random_4,  "memset_random",   "随机memset_4 bytes"},
        {memset_random_8,  "memset_random",   "随机memset_8 bytes"},
        {memset_random_16, "memset_random",   "随机memset_16 bytes"},
        {memset_random_1K, "memset_random",   "随机memset_1K"},
        {memset_random_2K, "memset_random",   "随机memset_2K"},
        {memset_random_4K, "memset_random",   "随机memset_4K"},
        {memset_random_8K, "memset_random",   "随机memset_8K"},
        {memset_random_16K,"memset_random",   "随机memset_16K"},

        {snprintf_cost,    "snprintf_cost",   "snprintf耗时"},
};

////////////////////////////////////
// 测试代码
////////////////////////////////////
void utils_hardware_test_func() {
    bind_thread_to_cpu(2);
    UtilsCycles::init();

    // run test.
    {
        printf("每次计算需要多少时间 \n");
        for (uint32_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
            const TestInfo &t = tests[i];

            double sec = t.func();

            int32_t width = printf("%-23s", t.func_name);
            if (sec < 1.0e-06) {
                width += printf("%8.2f ns", 1e09 * sec);
            }
            else if (sec < 1.0e-03) {
                width += printf("%8.2f us", 1e06 * sec);
            }
            else if (sec < 1.0) {
                width += printf("%8.2f ms", 1e03 * sec);
            }
            else {
                width += printf("%8.2f s", sec);
            }

            printf("%*s %s \n", 26-width, "", t.func_desc);
        }
    }
}
