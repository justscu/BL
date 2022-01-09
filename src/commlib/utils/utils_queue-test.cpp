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

struct ErrTst {
    volatile uint64_t v;
    char str[16];
    volatile uint64_t mul;
};

void err_test_write_thread(SPSCQueue *que) {
    bind_thread_to_cpu(2);

    UtilsTimeElapse ut;
    ut.start();

    uint64_t c = 0;
    while (c < CNTS) {
        ErrTst *p = (ErrTst*)que->alloc();
        if (p) {
            // fprintf(stdout, " IN[%p] [%lu] \n", p, c);
//            p->v = c;
//            snprintf(p->str, 16, "%lu", c);
//            p->mul = c*10;

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
            // fprintf(stdout, "OUT[%p] [%lu]\n", p, c);
//            char buf[16];
//            snprintf(buf, 16, "%lu", c);
//
//            if (p->v != c || 0 != strncmp(buf, p->str, 16) || p->mul != c*10) {
//                fprintf(stdout, "[%lu, %s, %lu] Queue[%lu, %s, %lu] ", c, buf, c*10, p->v, p->str, p->mul);
//                assert(0);
//            }

            ++c;
            que->pop();
        }
    }

    int64_t ret = ut.stop_ns() / CNTS;
    fprintf(stdout, "err_test__read_thread [%ld ns]. \n", ret);
}

void err_test_for_SPSCQueue() {
    SPSCQueue que(sizeof(ErrTst), 16);
    que.init();

    std::thread *th1 = new std::thread(err_test_write_thread, &que);
    std::thread *th2 = new std::thread(err_test_read_thread, &que);
    th1->join();
    th2->join();

    fprintf(stdout, "err_test_for_SPSCQueue finish. \n");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测试 memory_order_relaxed
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace Test_memory_order_relaxed {

void thread1(std::atomic<int32_t> &a, std::atomic<int32_t> &b) {
	a.store(5, std::memory_order_relaxed);  // step 1
	b.store(10, std::memory_order_relaxed); // step 2

	usleep(1);
}

void thread2(std::atomic<int32_t> &a, std::atomic<int32_t> &b) {
	while (b.load(std::memory_order_relaxed) != 10) { ; } // step 3

	if (a.load(std::memory_order_relaxed) != 5) { // step 4
		assert(0);
	}
	usleep(1);
}

void test() {
	for (int64_t i = 0; i < 100*10000ul; ++i) {
		std::atomic<int32_t> a{0};
		std::atomic<int32_t> b{0};
		std::thread *t1 = new (std::nothrow) std::thread(thread1, std::ref(a), std::ref(b));
		std::thread *t2 = new (std::nothrow) std::thread(thread2, std::ref(a), std::ref(b));
		t1->join();
		t2->join();
	}
}

} // namespace Test_memory_order_relaxed



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测试 test name
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void utils_queue_test() {
	Test_memory_order_relaxed::test();
}
