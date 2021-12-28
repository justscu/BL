#include <thread>
#include <functional>
#include <unistd.h>
#include "log.h"


void th1(volatile uint64_t *c) {
    int64_t idx = 0;
    while (true) {
        char buf[256];
        snprintf(buf, sizeof(buf)-1, "th1: %ld, c[%lu]", ++idx, (*c)++);
        log_info("%s", buf);

        usleep(1);

        if (idx == 10000) { break; }
    }
}

void th2(volatile uint64_t *c) {
    int64_t idx = 0;
    while (true) {
        char buf[256];
        snprintf(buf, sizeof(buf)-1, "th2: %ld, c[%lu]", ++idx, (*c)++);
        log_info("%s", buf);

        usleep(1);

        if (idx == 10000) { break; }
    }
}

void th3(volatile uint64_t *c) {
    int64_t idx = 0;
    while (true) {
        char buf[256];
        snprintf(buf, sizeof(buf)-1, "th3: %ld, c[%lu]", ++idx, (*c)++);
        log_info("%s", buf);

        usleep(1);

        if (idx == 10000) { break; }
    }
}

void th4(volatile uint64_t *c) {
    int64_t idx = 0;
    while (true) {
        char buf[256];
        snprintf(buf, sizeof(buf)-1, "th4: %ld, c[%lu]", ++idx, (*c)++);
        log_info("%s", buf);

        usleep(1);

        if (idx == 10000) { break; }
    }
}

void th5(volatile uint64_t *c) {
    int64_t idx = 0;
    while (true) {
        char buf[256];
        snprintf(buf, sizeof(buf)-1, "th5: %ld, c[%lu]", ++idx, (*c)++);
        log_info("%s", buf);

        usleep(1);

        if (idx == 10000) { break; }
    }
}

void log_test() {
    log_init(LOG::kTrace, "/tmp/test.log");

    volatile uint64_t c = 0;

    std::thread *t1 = new std::thread(std::bind(th1, &c));
    std::thread *t2 = new std::thread(std::bind(th2, &c));
    std::thread *t3 = new std::thread(std::bind(th3, &c));
    std::thread *t4 = new std::thread(std::bind(th4, &c));
    std::thread *t5 = new std::thread(std::bind(th5, &c));

    t1->join();
    t2->join();
    t3->join();
    t4->join();
    t5->join();
}
