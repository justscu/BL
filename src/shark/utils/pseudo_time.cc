#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "pseudo_time.h"

namespace UTILS {

PseudoTime* PseudoTime::instance_ = NULL; // 定义并初始化静态变量
Mutex       PseudoTime::mutex_;

PseudoTime* PseudoTime::getInstance() {
    if (NULL == instance_) {
        mutex_.Acquire();
        if (NULL == instance_) { // double check
            PseudoTime* t = new PseudoTime;
            t->update_time();

            pthread_t tid = 0;
            pthread_create(&tid, NULL, PseudoTime::threadfn, t);
            instance_ = t;
        }
        mutex_.Release();
    }
    return instance_;
}

// 获取毫秒
uint64_t PseudoTime::msec() {
#ifdef __x86_64__
    return curtimeMSec_;
#else
    CasHeader head;
    do {
        while (!casModel_.get_no_update_header(head)) {
            ;
        }
        uint64_t ret = curtimeMSec_;
        // 这段时间，curtimeMSec_没有更新
        if (casModel_.check_equal(head)) {
            return ret;
        }
    } while (true);
    return 0;
#endif
}

// 获取秒
time_t PseudoTime::sec() {
    return msec() / 1000;
}

void PseudoTime::stop() {
    continue_ = false;
}

// 只有时间更新线程会调用该函数
void PseudoTime::update_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#ifdef __x86_64__
    curtimeMSec_ = cur; // 64bit系统，一个指令周期可以完成，是原子的
#else
    casModel_.begin_update();
    curtimeMSec_ = cur;
    casModel_.end_update();
#endif
}

void* PseudoTime::threadfn(void *ptr) {
    PseudoTime* p = static_cast<PseudoTime*>(ptr);
    while (p->continue_) {
        usleep(1000); // 每1ms更新一次
        p->update_time();
    }
    return NULL;
}

} // namespace UTILS

