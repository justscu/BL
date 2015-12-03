#include <thread>
#include "log.h"

static
void thread_cb(int id) {
    for (int i = 0; i < 100; ++i) {
        DEBUG("identify[%d] [%d]  000--- OK, debug, info", id, i);
    }
    sleep(10);
}

int log_test() {
    INFO("fdsfew");
    DEBUG("[%s %d %f]", "fsdfd", 1232, 12.343);
    WARN("here is warning!");
    ERROR("here is errror ");

    LOG::FileLogger *pDebug = new LOG::FileLogger("log_test.debug");
    //LOG::Logger *pDebug = new LOG::OStreamLogger(std::cout);
    SET_LOGGER(LOG::LOGLEVEL::kDebug, pDebug);

    std::thread* th[200];
    for (size_t i = 0; i < sizeof(th) / sizeof(th[0]); ++i) {
        th[i] = new std::thread(std::bind(thread_cb, i));
        th[i]->detach();
    }

    sleep(20);
    delete pDebug;
    for (size_t i = 0; i < sizeof(th) / sizeof(th[0]); ++i) {
        delete th[i];
    }
    return 0;
}
