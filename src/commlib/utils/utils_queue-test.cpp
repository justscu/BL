#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include "utils.h"
#include "fmt/format.h"

#define CNTS (8*1024*1024*1024ul)

static
void discard_value(void *value) {
    int64_t x = *(int64_t*)value;
    if (x == 0x43924776) {
        fprintf(stdout, "value is 0x%lx \n", x);
    }
}

struct Info {
    uint64_t v;
    char str[16];
    uint64_t mul;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CycleQueue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CycleQueueTest {
public:
    CycleQueueTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        bind_thread_to_cpu(2);
        assert(que_.init());

        UtilsCycles::init();

        const uint64_t pre = UtilsCycles::rdtsc();
        Info *p = nullptr;
        for (uint64_t i = 0; i < CNTS; ++i) {
            p = que_.alloc();
            p->v = i;
        }

        const uint64_t end = UtilsCycles::rdtsc();

        int64_t ret = UtilsCycles::cycles_to_nanosecond(end-pre) / CNTS;
        discard_value(p);

        fmt::print("CycleQueueTest: {} total cost[{} ns], each cost[{} ns]. \n",
                typeid(que_).name(), end-pre, ret);
    }

private:
    CycleQueue<Info> que_;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ErrTest
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class QueType>
class ErrTest {
public:
    ErrTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th1(std::bind(&ErrTest::read_thread, this));
        sleep(1);
        std::thread th2(std::bind(&ErrTest::write_thread, this));

        th1.join();
        th2.join();

        fmt::print("{} ErrTest: test finish. \n", typeid(que_).name());
    }

private:
    void write_thread() {
        bind_thread_to_cpu(2);

        int64_t full_cnt = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info *p = que_.alloc();
            if (p) {
                p->v = c;
                snprintf(p->str, 16, "%lu", c);
                p->mul = c*10;

                ++c;
                que_.push();
            }
            else {
                ++full_cnt; // queue is full.
            }
        }

        fmt::print("{} ErrTest::write_thread: finish. \n", typeid(que_).name());
    }

    void read_thread() {
        bind_thread_to_cpu(6);

        uint64_t c = 0;
        while (c < CNTS) {
            Info *p = que_.front();
            if (p) {
                char buf[16];
                snprintf(buf, 16, "%lu", c);

                if (p->v != c || 0 != strncmp(buf, p->str, 16) || p->mul != c*10) {
                    fmt::print("{} ErrTest::read_thread: ERR, data in queue[{}, {}, {}], need[{}, {}, {}]. \n",
                            typeid(que_).name(), p->v, p->str, p->mul, c, buf, c*10);
                    exit(0);
                }

                ++c;
                que_.pop();
            }
        }

        fmt::print("{} ErrTest::read_thread: finish. \n", typeid(que_).name());
    }

private:
    QueType que_;
};


template<class QueType>
class ThroughputTest {
public:
    ThroughputTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        que_.init();

        std::thread th1(std::bind(&ThroughputTest::read_thread, this));
        sleep(1);
        std::thread th2(std::bind(&ThroughputTest::write_thread, this));
        th1.join();
        th2.join();

        fmt::print("{} ThroughputTest::SPSCQueue1: finish. \n", typeid(que_).name());
    }

private:
    void write_thread() {
        bind_thread_to_cpu(2);

        const uint64_t pre = UtilsClock::get_ns();

        uint64_t full_cnt = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info *p = que_.alloc();
            if (!p) {
                ++full_cnt;
                continue;
            }

            p->v = c++;
            que_.push();
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / CNTS;
        fmt::print("{} ThroughputTest::write_thread: each cost[{} ns], queue full cnt[{}]. \n",
                typeid(que_).name(), ret, full_cnt);
    }

    void read_thread() {
        bind_thread_to_cpu(6);

        uint64_t pre = 0;
        int64_t empty_cnt = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info *p = que_.front();
            if (!p) {
                ++empty_cnt;
                continue;
            }

            if (c == 0) { pre = UtilsClock::get_ns(); }
            if (p->v != c) {
                fmt::print("{} ThroughputTest::read_thread: value err. in[{}] out[{}]. \n",
                        typeid(que_).name(), p->v, c);
                exit(0);
            }

            ++c;
            que_.pop();
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / CNTS;

        fmt::print("{} ThroughputTest::read_thread: each read cost[{} ns], queue empty cnt[{}]. \n",
                typeid(que_).name(), ret, empty_cnt);
    }

private:
    QueType que_;
};


template<class Queue>
class LatencyTest {
public:
    LatencyTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th1(std::bind(&LatencyTest::read_thread, this));
        sleep(1);
        std::thread th2(std::bind(&LatencyTest::write_thread, this));
        th1.join();
        th2.join();

        fmt::print("{} LatencyTest: finish. \n", typeid(que_).name());
    }

private:
    void write_thread() {
        bind_thread_to_cpu(2);

        uint64_t full_cnt = 0;
        uint64_t c = 0;

        while (c < CNTS) {
            Info *p = que_.alloc();
            if (!p) {
                ++full_cnt;
                continue;
            }

            p->mul = UtilsClock::get_ns();
            que_.push();
            ++c;

            cpu_delay(10);
        }

        fmt::print("{} LatencyTest::write_thread: finish. \n", typeid(que_).name());
    }

    void read_thread() {
        bind_thread_to_cpu(4);

        uint64_t sum = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info *p = que_.front();
            if (!p) {
                continue;
            }

            const uint64_t pre = p->mul;
            const uint64_t now = UtilsClock::get_ns();
            if (now < pre) {
                fmt::print("{} LatencyTest::read_thread: ERR, now[{}] < pre[{}] \n",
                        typeid(que_).name(), now, pre);
                exit(0);
            }
            else {
                sum += (now - pre);
            }
            ++c;
            que_.pop();
        }

        int64_t avg = sum/CNTS;

        fmt::print("{} LatencyTest::read_thread: each in->out cost[{} ns]. \n", typeid(que_).name(), avg);
    }

private:
    Queue que_;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MPSC test

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if 0
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
    fmt::print("ErrTest::MPSCQueue::producer: each cost[{}]. \n", ret);
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

    fmt::print("ErrTest::MPSCQueue::consumer: c[{}]. \n", c);
    const uint64_t need = producer_th_cnt*(CNTS+1)*CNTS/2;
    if (sum != need) {
        fmt::print("ErrTest::MPSCQueue::consumer: ERR sum[{}], need[{}]. \n", sum, need);
    }

    fmt::print("ErrTest::MPSCQueue::consumer: each cost[{}]. \n", ret);
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

#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测试 test name
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void utils_queue_test() {
    CycleQueueTest ct(1024*1024); ct.test();

    ErrTest<SPSCQueue1<Info>> err1(1024*1024); err1.test();
    ErrTest<SPSCQueue2<Info>> err2(1024*1024); err2.test();

    ThroughputTest<SPSCQueue1<Info>> tp1(1024*1024); tp1.test();
    ThroughputTest<SPSCQueue2<Info>> tp2(1024*1024); tp2.test();

    LatencyTest<SPSCQueue1<Info>> la1(1024*1024); la1.test();
    LatencyTest<SPSCQueue2<Info>> la2(1024*1024); la2.test();
}
