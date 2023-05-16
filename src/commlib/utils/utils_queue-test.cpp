#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include "utils.h"
#include "fmt/format.h"

#define CNTS (32*1024*1024ul)

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
                    throw "ErrTest::read_thread: ERR";
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

        fmt::print("{} ThroughputTest: finish. \n", typeid(que_).name());


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
                throw "ThroughputTest::read_thread: value err.";
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
                throw "LatencyTest::read_thread: ERR";
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


/////////////////////////////////////////////

class MPSCQueueErrTest {
private:
    struct Info {
        int64_t cnt;
        char    buf[16];
        int64_t num;
    };

public:
    MPSCQueueErrTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPSCQueueErrTest::producer, this, 2));
        std::thread th3(std::bind(&MPSCQueueErrTest::producer, this, 3));
        std::thread th4(std::bind(&MPSCQueueErrTest::producer, this, 4));
        std::thread th5(std::bind(&MPSCQueueErrTest::producer, this, 5));
        std::thread th6(std::bind(&MPSCQueueErrTest::consumer, this, 6, 4));


        th2.join();
        th3.join();
        th4.join();
        th5.join();
        th6.join();

        fmt::print("MPSCQueueErrTest::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = c+1;
            snprintf(info.buf, sizeof(info.buf)-1, "%ld", info.num);
            if (que_.try_push(info)) {
                ++c;
            }
        }

        fmt::print("MPSCQueueErrTest::producer: finish. \n");
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        uint64_t cnt = 0;
        uint64_t sum = 0;

        Info info;
        while (cnt < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt;

                sum += info.num;
                char buf[32];
                snprintf(buf, sizeof(buf)-1, "%ld", info.num);
                if (0 != strcmp(buf, info.buf)) {
                    fmt::print("MPSCQueueErrTest::consumer err. [{}] {{}}", buf, info.buf);
                    throw "MPSCQueueErrTest::consumer err";
                }
            }
        }

        const uint64_t need = pro_cnt*(CNTS+1)*CNTS/2;

        if (need != sum) {
            throw "MPSCQueueErrTest::consumer error.";
        }
        fmt::print("MPSCQueueErrTest::consumer finish. need[{}], sum[{}]. \n", need, sum);
    }

private:
    MPSCQueue<Info>       que_;
};


class MPSCQueueThroughputTest {
private:
    struct Info {
        int64_t cnt;
        char    buf[16];
        int64_t num;
    };

public:
    MPSCQueueThroughputTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPSCQueueThroughputTest::producer, this, 2));
        std::thread th3(std::bind(&MPSCQueueThroughputTest::producer, this, 3));
        std::thread th4(std::bind(&MPSCQueueThroughputTest::producer, this, 4));
        std::thread th6(std::bind(&MPSCQueueThroughputTest::consumer, this, 6, 3));

        th2.join();
        th3.join();
        th4.join();
        th6.join();

        fmt::print("MPSCQueueThroughputTest::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t full_cnt = 0;
        const uint64_t pre = UtilsClock::get_ns();
        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = c+1;
            if (que_.try_push(info)) {
                ++c;
            }
            else {
                ++full_cnt;
            }
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / CNTS;

        fmt::print("MPSCQueueThroughputTest::producer: each write cost[{} ns], queue full cnt[{}]. \n",
                ret, full_cnt);
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        int64_t m = 0;
        Info info;
        uint64_t empty_cnt = 0;

        uint64_t cnt = 0;
        const uint64_t pre = UtilsClock::get_ns();
        while (cnt < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt;
                ++m;
            }
            else {
                ++empty_cnt;
            }
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / m;

        fmt::print("MPSCQueueThroughputTest::consumer finish. each read cost[{} ns], queue empty cnt[{}]. \n",
                ret, empty_cnt);
    }

private:
    MPSCQueue<Info>       que_;
};

class MPSCQueueLatencyTest {
private:
    struct Info {
        uint64_t cnt;
        char     buf[16];
        uint64_t num;
    };

public:
    MPSCQueueLatencyTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPSCQueueLatencyTest::producer, this, 2));
        std::thread th3(std::bind(&MPSCQueueLatencyTest::producer, this, 3));
        std::thread th4(std::bind(&MPSCQueueLatencyTest::producer, this, 4));
        std::thread th6(std::bind(&MPSCQueueLatencyTest::consumer, this, 6, 3));

        th2.join();
        th3.join();
        th4.join();
        th6.join();

        fmt::print("MPSCQueueLatencyTest::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t full_cnt = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = UtilsClock::get_ns();
            if (que_.try_push(info)) {
                ++c;
            }
            else {
                ++full_cnt;
            }
            cpu_delay(18);
        }

        fmt::print("MPSCQueueLatencyTest::producer: queue full cnt[{}]. \n", full_cnt);
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        uint64_t cost = 0;
        int64_t m = 0;
        Info info;
        uint64_t empty_cnt = 0;

        uint64_t cnt = 0;
        while (cnt < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt;
                ++m;
                uint64_t now = UtilsClock::get_ns();
                if (now <= info.num) {
                    throw "now <= info.num";
                }
                cost += (now - info.num);
            }
            else {
                ++empty_cnt;
            }
        }

        int64_t ret = (cost) / m;

        fmt::print("MPSCQueueLatencyTest::consumer finish. each read cost[{} ns], queue empty cnt[{}]. \n",
                ret, empty_cnt);
    }

private:
    MPSCQueue<Info>       que_;
};


/////////////////////////////////////////////

class MPMCQueueErrTest {
private:
    struct Info {
        int64_t cnt;
        char    buf[16];
        int64_t num;
    };

public:
    MPMCQueueErrTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPMCQueueErrTest::producer, this, 2));
        std::thread th3(std::bind(&MPMCQueueErrTest::producer, this, 3));
        std::thread th4(std::bind(&MPMCQueueErrTest::producer, this, 4));
        std::thread th5(std::bind(&MPMCQueueErrTest::producer, this, 5));
        std::thread th6(std::bind(&MPMCQueueErrTest::consumer, this, 6, 4));
        std::thread th7(std::bind(&MPMCQueueErrTest::consumer, this, 7, 4));
        std::thread th8(std::bind(&MPMCQueueErrTest::consumer, this, 8, 4));

        th2.join();
        th3.join();
        th4.join();
        th5.join();
        th6.join();
        th7.join();
        th8.join();

        fmt::print("MPMCQueueErrTest::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = c+1;
            snprintf(info.buf, sizeof(info.buf)-1, "%ld", info.num);
            if (que_.try_push(info)) {
                ++c;
            }
        }

        fmt::print("MPMCQueueErrTest::producer: finish. \n");
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        Info info;
        while (cnt_ < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt_;

                sum_ += info.num;
                char buf[32];
                snprintf(buf, sizeof(buf)-1, "%ld", info.num);
                if (0 != strcmp(buf, info.buf)) {
                    fmt::print("MPMCQueueErrTest::consumer err. [{}] {{}}", buf, info.buf);
                    throw "MPMCQueueErrTest::consumer err";
                }
            }
        }

        const uint64_t need = pro_cnt*(CNTS+1)*CNTS/2;

        if (need != sum_) {
            throw "MPMCQueueErrTest::consumer error.";
        }
        fmt::print("MPMCQueueErrTest::consumer finish. need[{}], sum[{}]. \n", need, sum_);
    }

private:
    MPMCQueue<Info>       que_;
    std::atomic<uint64_t> cnt_{0};
    std::atomic<uint64_t> sum_{0};
};


class MPMCQueueThroughputTest {
private:
    struct Info {
        int64_t cnt;
        char    buf[16];
        int64_t num;
    };

public:
    MPMCQueueThroughputTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPMCQueueThroughputTest::producer, this, 2));
        std::thread th3(std::bind(&MPMCQueueThroughputTest::producer, this, 3));
        std::thread th4(std::bind(&MPMCQueueThroughputTest::producer, this, 4));
        std::thread th5(std::bind(&MPMCQueueThroughputTest::producer, this, 5));
        std::thread th6(std::bind(&MPMCQueueThroughputTest::consumer, this, 6, 4));
        std::thread th7(std::bind(&MPMCQueueThroughputTest::consumer, this, 7, 4));
        std::thread th8(std::bind(&MPMCQueueThroughputTest::consumer, this, 8, 4));

        th2.join();
        th3.join();
        th4.join();
        th5.join();
        th6.join();
        th7.join();
        th8.join();

        fmt::print("MPMCQueueThroughput::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t full_cnt = 0;
        const uint64_t pre = UtilsClock::get_ns();
        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = c+1;
            if (que_.try_push(info)) {
                ++c;
            }
            else {
                ++full_cnt;
            }
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / CNTS;

        fmt::print("MPMCQueueThroughput::producer: each write cost[{} ns], queue full cnt[{}]. \n",
                ret, full_cnt);
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        int64_t m = 0;
        Info info;
        uint64_t empty_cnt = 0;
        const uint64_t pre = UtilsClock::get_ns();
        while (cnt_ < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt_;
                ++m;
            }
            else {
                ++empty_cnt;
            }
        }

        const uint64_t end = UtilsClock::get_ns();
        int64_t ret = (end-pre) / m;

        fmt::print("MPMCQueueThroughput::consumer finish. each read cost[{} ns], queue empty cnt[{}]. \n",
                ret, empty_cnt);
    }

private:
    MPMCQueue<Info>       que_;
    std::atomic<uint64_t> cnt_{0};
};


class MPMCQueueLatencyTest {
private:
    struct Info {
        uint64_t cnt;
        char     buf[16];
        uint64_t num;
    };

public:
    MPMCQueueLatencyTest(uint64_t capacity) : que_(capacity) {}

    void test() {
        assert(que_.init());

        std::thread th2(std::bind(&MPMCQueueLatencyTest::producer, this, 2));
        std::thread th3(std::bind(&MPMCQueueLatencyTest::producer, this, 3));
        std::thread th4(std::bind(&MPMCQueueLatencyTest::producer, this, 4));
        std::thread th5(std::bind(&MPMCQueueLatencyTest::producer, this, 5));
        std::thread th6(std::bind(&MPMCQueueLatencyTest::consumer, this, 6, 4));
        std::thread th7(std::bind(&MPMCQueueLatencyTest::consumer, this, 7, 4));

        th2.join();
        th3.join();
        th4.join();
        th5.join();
        th6.join();
        th7.join();

        fmt::print("MPMCQueueLatencyTest::test finish. \n");
    }

private:
    void producer(int32_t cpuid) {
        bind_thread_to_cpu(cpuid);

        uint64_t full_cnt = 0;
        uint64_t c = 0;
        while (c < CNTS) {
            Info info;
            info.num = UtilsClock::get_ns();
            if (que_.try_push(info)) {
                ++c;
            }
            else {
                ++full_cnt;
            }
            cpu_delay(20);
        }

        fmt::print("MPMCQueueLatencyTest::producer: queue full cnt[{}]. \n", full_cnt);
    }

    void consumer(int32_t cpuid, int32_t pro_cnt) {
        bind_thread_to_cpu(cpuid);

        uint64_t cost = 0;
        int64_t m = 0;
        Info info;
        uint64_t empty_cnt = 0;
        while (cnt_ < pro_cnt * CNTS) {
            if (que_.try_pop(info)) {
                ++cnt_;
                ++m;
                uint64_t now = UtilsClock::get_ns();
                if (now <= info.num) {
                    throw "now <= info.num";
                }
                cost += (now - info.num);
            }
            else {
                ++empty_cnt;
            }
        }

        int64_t ret = (cost) / m;

        fmt::print("MPMCQueueLatencyTest::consumer finish. each read cost[{} ns], queue empty cnt[{}]. \n",
                ret, empty_cnt);
    }

private:
    MPMCQueue<Info>       que_;
    std::atomic<uint64_t> cnt_{0};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测试 test name
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void utils_queue_test() {
    CycleQueueTest ct(1024*1024); ct.test();

    ErrTest<SPSCQueue<Info>>  err(1024*1024);  err.test();
    ErrTest<SPSCQueue1<Info>> err1(1024*1024); err1.test();

    ThroughputTest<SPSCQueue<Info>>  tp(1024*1024);  tp.test();
    ThroughputTest<SPSCQueue1<Info>> tp1(1024*1024); tp1.test();

    LatencyTest<SPSCQueue<Info>>  la(1024*1024);  la.test();
    LatencyTest<SPSCQueue1<Info>> la1(1024*1024); la1.test();

    MPSCQueueErrTest        mpsc_errtest(1024*1024); mpsc_errtest.test();
    MPSCQueueThroughputTest mpsc_thtest(1024*1024);  mpsc_thtest.test();
    MPSCQueueLatencyTest    mpsc_latency(1024*1024); mpsc_latency.test();

    MPMCQueueErrTest        mpmc_errtest(1024*1024); mpmc_errtest.test();
    MPMCQueueThroughputTest mpmc_thtest(1024*1024);  mpmc_thtest.test();
    MPMCQueueLatencyTest    mpmc_latency(1024*1024); mpmc_latency.test();
}
