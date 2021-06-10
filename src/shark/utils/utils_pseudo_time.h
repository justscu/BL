#pragma once

#include <mutex>

///////////////////////////////
//
//      CAS: 适用于一读多写
//
// 适用于一线程写，多线程读. 需要写和读快速完成
// 使用该方法，可以避免加锁
///////////////////////////////
class UtilsCasModel {
public:
    union CasHeader {
        struct {
            uint16_t state; // 1, 正在更新; 0, 没有更新
            uint16_t index; // 每次更新, index增1
        } s;
        volatile int32_t dummy; // sizeof(dummy) = sizeof(s)
    };

public:
    UtilsCasModel() { header_.dummy = 0; }
    UtilsCasModel(const UtilsCasModel&) = delete;
    UtilsCasModel& operator=(const UtilsCasModel&) = delete;

    // begin_update & end_update 需要配对使用
    void begin_update() { header_.s.state = 0; }
    void end_update() {
        header_.s.index += 1;
        header_.s.state  = 0;
    }

    // if return false, 说明正在更新
    bool get_no_update_header(CasHeader &hd) {
        hd.dummy = header_.dummy;
        if (1 == hd.s.state) {
            return false; // updating.
        }
        return true;
    }

    bool check_equal(const CasHeader &hd) { return hd.dummy == header_.dummy; }

private:
    volatile CasHeader header_;
};


///////////////////////////////
// 一个低精度的获取毫秒的类
// 启动一个线程，每隔100us更新一下时间
// 允许多个线程读取时间
///////////////////////////////
class UtilsPseudoTime {
public:
    ~UtilsPseudoTime();

    static UtilsPseudoTime* get_instance(); // 单例模式
    void stop() { run_ = false; }
    uint64_t get_sec() { return get_msec() / 1000; }
    uint64_t get_msec();

private:
    UtilsPseudoTime() { }
    void update_time();
    // 只有一个线程写
    static void* thread_func(void *ptr);

private:
    bool                run_ = true;
    UtilsCasModel cas_model_;
    uint64_t   current_msec_ = 0;

    static std::mutex          mutex_;
    static UtilsPseudoTime *instance_;
};
