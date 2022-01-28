#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include "utils_cpu.h"
#include "utils_times.h"
#include "utils_queue.h"

#define CNTS (10000*10000ul)

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
    CycleQueue que;
    assert(que.init(sizeof(Info), 1024));

    void *p = nullptr;
    UtilsTimeElapse ut;
    ut.start();

    for (uint64_t i = 0; i < CNTS; ++i) {
        p = que.alloc();
    }

    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "CycleQueueTest [%ld ns/each].\n", ret);
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

    UtilsTimeElapse ut;
    ut.start();

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
    }
    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "err_test_write_thread [%ld ns].\n", ret);
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
                fprintf(stdout, "[%lu, %s, %lu] Queue[%lu, %s, %lu] ", c, buf, c*10, p->v, p->str, p->mul);
                assert(0);
            }

            ++c;
            que->pop();
        }
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
void err_test_write_thread(SPSCQueue *que) {
    bind_thread_to_cpu(2);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst*)que->alloc();
        if (p) {
            ++c;
            que->push();
        }
    }
    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "err_test_write_thread [%ld ns].\n", ret);
}

void err_test_read_thread(SPSCQueue *que) {
    bind_thread_to_cpu(3);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst *)que->front();
        if (p) {
            ++c;
            que->pop();
        }
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
            ++c;
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

    uint64_t c = 0;
    while (c < CNTS*producer_th_cnt) {
        ErrTst *p = (ErrTst*)que->front();
        if (p) {
            ++c;
            que->pop();
        }
    }
    int64_t ret = ut.stop_ns() / (CNTS*producer_th_cnt);
    fprintf(stdout, "consumer [%ld ns].\n", ret);
}

void test_for_MPSCQueue() {
    MPSCQueue que(sizeof(ErrTst), 32);
    que.init();

    std::thread th2(producer, &que, 2);
    std::thread th3(producer, &que, 3);
    std::thread th4(producer, &que, 4);
    std::thread th5(producer, &que, 5);
    std::thread th6(consumer, &que, 6, 1);

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
    // ErrTest::test_for_SPSCQueue();
    // SpeedTest::test_for_SPSCQueue();
    // ErrTest::test_for_MPSCQueue();
    CycleQueueTest::test();
}
