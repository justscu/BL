#include <stdio.h>
#include <iomanip>
#include <thread>
#include <unistd.h>


void add(volatile int64_t* d, int32_t thid) {
    int64_t rst = 0;
    for (int64_t i = 0; i < 10000; ++i) {
        const int64_t v1 = *d;
        int64_t v = __sync_fetch_and_add(d, 1);
        rst += v;
        if (thid == 1 && v1+1 != v) {
            fprintf(stdout, "v1[%ld] v2[%ld]. \n", v1, v);
        }
    }

    fprintf(stdout, "thread[%d] rst: %ld. \n", thid, rst);
}


void tst_sync_fetch_and_add() {
    volatile int64_t value = 1;

    std::thread* th1 = new std::thread(std::bind(add, &value, 0));
    std::thread* th2 = new std::thread(std::bind(add, &value, 1));
    std::thread* th3 = new std::thread(std::bind(add, &value, 2));
    std::thread* th4 = new std::thread(std::bind(add, &value, 3));

    th1->join();
    th2->join();
    th3->join();
    th4->join();

    getchar();

    fprintf(stdout, "value: %ld. \n", value);
    // result : value = 40001
}
