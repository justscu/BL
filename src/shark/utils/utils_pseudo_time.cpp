#include <unistd.h>
#include <sys/time.h>
#include "utils_pseudo_time.h"

UtilsPseudoTime* UtilsPseudoTime::instance_ = nullptr;
std::mutex UtilsPseudoTime::mutex_;

UtilsPseudoTime::~UtilsPseudoTime() {
    if (instance_) {
        delete instance_;
        instance_ = nullptr;
    }
}

UtilsPseudoTime* UtilsPseudoTime::get_instance() {
    if (!instance_) {
        mutex_.lock();
        if (!instance_) {
            instance_ = new (std::nothrow) UtilsPseudoTime;
            if (instance_) {
                instance_->update_time();
                pthread_t tid = 0;
                pthread_create(&tid, nullptr, UtilsPseudoTime::thread_func, instance_);
            }
        }
        mutex_.unlock();
    }

    return instance_;
}

uint64_t UtilsPseudoTime::get_msec() {
    uint64_t ret = 0;

    UtilsCasModel::CasHeader hd;
    while (true) {
        while (!cas_model_.get_no_update_header(hd)) {
            // usleep(1);
        }
        ret = current_msec_;
        // same, 说明没有更新
        if (cas_model_.check_equal(hd)) {
            break;
        }
    }

    return ret;
}

void UtilsPseudoTime::update_time() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    const uint64_t cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    //
    cas_model_.begin_update();
    current_msec_ = cur;
    cas_model_.end_update();
}

// 写线程
void* UtilsPseudoTime::thread_func(void *ptr) {
    UtilsPseudoTime *p = (UtilsPseudoTime*)ptr;
    while (p->run_) {
        usleep(100); // 100us更新一次
        p->update_time();
    }
    return nullptr;
}
