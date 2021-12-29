#pragma once
#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CAS: 适用于一写多读
// 适用于一线程写，多线程读. 需要写和读快速完成
// 使用该方法，可以避免加锁
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsCasModel {
public:
    enum State : uint16_t {
        kState_free = 0, // 没有更新
        kState_busy = 1, // 正在更新
    };

    union CasHeader {
        struct {
            State    state;
            uint16_t index; // 每次更新, index增1
        } s;
        volatile int32_t dummy; // sizeof(dummy) = sizeof(s)
    };

public:
    UtilsCasModel() {
        static_assert(sizeof(CasHeader)==4, "");
        header_.dummy = State::kState_free;
    }

    UtilsCasModel(const UtilsCasModel&) = delete;
    UtilsCasModel& operator=(const UtilsCasModel&) = delete;

    // begin_update & end_update 需要配对使用
    void begin_update() { header_.s.state = State::kState_busy; }
    void end_update() {
        header_.s.index += 1;
        header_.s.state  = State::kState_free;
    }

    // if return false, 说明正在更新
    bool get_no_update_header(CasHeader &hd) {
        hd.dummy = header_.dummy;
        return (State::kState_free == hd.s.state);
    }

    bool check_equal(const CasHeader &hd) { return hd.dummy == header_.dummy; }

private:
    volatile CasHeader header_;
};
