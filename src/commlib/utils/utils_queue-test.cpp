#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include "utils.h"

#define CNTS (100000000ul)

static
void discard_value(void *value) {
    int64_t x = *(int64_t*)value;
    if (x == 0x43924776) {
        fprintf(stdout, "value is 0x%lx \n", x);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CycleQueue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace CycleQueueTest {
struct Info {
    uint64_t i1;
    double   d2;
    char     c3[8];
};

void test() {
    bind_thread_to_cpu(2);
    CycleQueue que;
    assert(que.init(sizeof(Info), 1024));

    UtilsCycles::init();

    const uint64_t pre = UtilsCycles::rdtsc();
    void *p = nullptr;
    for (uint64_t i = 0; i < CNTS; ++i) {
        p = que.alloc();
    }

    const uint64_t end = UtilsCycles::rdtsc();

    int64_t ret = UtilsCycles::cycles_to_nanosecond(end-pre) / CNTS;
    fprintf(stdout, ">>> CycleQueueTest total[%ld ns] [%ld ns/each].\n", end-pre, ret);
    discard_value(p);
}

} // CycleQueueTest

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct ErrTst {
    uint64_t v;
    char str[16];
    uint64_t mul;
};

namespace ErrTest {

void err_test_write_thread(SPSCQueue *que) {
    bind_thread_to_cpu(2);

    UtilsCycles::init();
    const uint64_t pre = UtilsCycles::rdtsc();

    int64_t full_cnt = 0;
    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst*)que->alloc();
        if (p) {
            p->v = c;
            snprintf(p->str, 16, "%lu", c);
            p->mul = c*10;

            ++c;
            que->push();
        }
        else {
            ++full_cnt;
        }
    }

    const uint64_t end = UtilsCycles::rdtsc();
    int64_t ret = UtilsCycles::cycles_to_nanosecond(end-pre) / CNTS;
    fprintf(stdout, "err_test_write_thread [%ld ns], queue full_cnt[%ld].\n", ret, full_cnt);
}

void err_test_read_thread(SPSCQueue *que) {
    bind_thread_to_cpu(3);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst *)que->front();
        if (p) {
            char buf[16];
            snprintf(buf, 16, "%lu", c);

            if (p->v != c || 0 != strncmp(buf, p->str, 16) || p->mul != c*10) {
                fprintf(stdout, "ERROR: [%lu, %s, %lu] Queue[%lu, %s, %lu] ", c, buf, c*10, p->v, p->str, p->mul);
                assert(0);
            }

            ++c;
            que->pop();
        }
        if (c % 10000 == 0) { usleep(5); }
    }

    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "err_test__read_thread [%ld ns]. \n", ret);
}

void test_for_SPSCQueue() {
    SPSCQueue que;
    que.init(sizeof(ErrTst), 32);

    std::thread th1(err_test_write_thread, &que);
    std::thread th2(err_test_read_thread, &que);
    th1.join();
    th2.join();

    fprintf(stdout, "err_test_for_SPSCQueue finish. \n");
}

} // namespace ErrTest

namespace SpeedTest {
void speed_test_write_thread(SPSCQueue *que) {
    bind_thread_to_cpu(2);

    const uint64_t pre = UtilsCycles::rdtsc();

    uint64_t full_cnt = 0;
    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst*)que->alloc();
        if (p) {
            ++c;
            p->v = UtilsCycles::rdtsc();
            que->push();
        }
        else {
            ++full_cnt;
        }
    }
    const uint64_t end = UtilsCycles::rdtsc();
    int64_t ret = UtilsCycles::cycles_to_nanosecond(end-pre) / CNTS;
    fprintf(stdout, "speed_test_write_thread [%ld ns]. queue full_cnt[%ld] \n", ret, full_cnt);
}

void speed_test_read_thread(SPSCQueue *que) {
    bind_thread_to_cpu(3);

    const uint64_t pre = UtilsCycles::rdtsc();

    uint64_t sum = 0;
    int64_t empty_cnt = 0;
    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst *)que->front();
        if (p) {
            ++c;
            uint64_t pre = p->v;
            const uint64_t now = UtilsCycles::rdtsc();
            if (now < pre) {
                fprintf(stderr, "ERROR: now[%ld] < pre[%ld] \n", now, pre);
            }
            else {
                sum += (now - pre);
            }
            que->pop();
        }
        else {
            ++empty_cnt;
        }
    }

    const uint64_t end = UtilsCycles::rdtsc();
    int64_t ret = UtilsCycles::cycles_to_nanosecond(end-pre) / CNTS;
    int64_t avg = UtilsCycles::cycles_to_nanosecond(sum) / CNTS;

    fprintf(stdout, "speed_test__read_thread [%ld ns] queue empty_cnt[%ld]; in->out speed [%ld ns] . \n", ret, empty_cnt, avg);
}

void speed_for_SPSCQueue() {
    SPSCQueue que;
    que.init(sizeof(ErrTst), 1024*1024);

    UtilsCycles::init();

    std::thread th1(speed_test_write_thread, &que);
    std::thread th2(speed_test_read_thread, &que);
    th1.join();
    th2.join();

    fprintf(stdout, "speed_test_for_SPSCQueue finish. \n");
}

} // SpeedTest


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC test

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace ErrTest {
void producer(MPSCQueue *que, int32_t cpuid) {
    bind_thread_to_cpu(cpuid);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst*)que->alloc();
        if (p) {
            p->v = ++c;
            que->push();
        }
    }
    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "producer [%ld ns].\n", ret);
}

void consumer(MPSCQueue *que, int32_t cpuid, int32_t producer_th_cnt) {
    bind_thread_to_cpu(cpuid);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t sum = 0;
    uint64_t c = 0;
    while (c < CNTS*producer_th_cnt) {
        ErrTst *p = (ErrTst*)que->front();
        if (p) {
            ++c;
            sum += p->v;
            que->pop();
        }
    }
    int64_t ret = ut.stop_ns() / (CNTS*producer_th_cnt);

    if (sum != producer_th_cnt*(CNTS+1)*CNTS/2) {
        fprintf(stderr, "error. sum[%ld] \n", sum);
    }
    fprintf(stdout, "consumer [%ld ns].\n", ret);
}

void test_for_MPSCQueue() {
    MPSCQueue que(sizeof(ErrTst), 32);
    que.init();

    std::thread th2(producer, &que, 2);
    std::thread th3(producer, &que, 3);
    std::thread th4(producer, &que, 4);
    std::thread th5(producer, &que, 5);
    std::thread th6(consumer, &que, 6, 4);

    th2.join();
    th3.join();
    th4.join();
    th5.join();
    th6.join();

    fprintf(stdout, "test_for_MPSCQueue finish. \n");
}

} // namespace ErrTest

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测试 test name
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void utils_queue_test() {
    CycleQueueTest::test();
    ErrTest::test_for_SPSCQueue();
    SpeedTest::speed_for_SPSCQueue();
    ErrTest::test_for_MPSCQueue();
}
