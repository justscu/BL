#ifndef __UTILS_PSEUDO_TIME__
#define __UTILS_PSEUDO_TIME__

#include <stdint.h>
#include "lock.h"

namespace UTILS {
/*
 *       CAS, compare and swap,
 * 这个地方的CAS模式，适用于只有一个线程写，多个线程读；需要写和读都能够很快完成（不能用指针）
 * 这种方法，可以避免加锁
 *
 *       volatile修饰符
 * (1)在编译时，该变量不会被优化掉；
 * (2)在运行时，CPU每次都从内存中取值，而不会从CPU寄存器中取值
 *
 * */
union CasHeader {
    struct {
        uint16_t state; // 1表示正在更新，０表示没有更新
        uint16_t index; // 每次更新，index变化
    } s;
    volatile int32_t dummy; // sizeof(s) = sizeof(dummy)
};

// CAS 模式
class CasModel {
private:
    volatile CasHeader header_;
public:
    CasModel() {
        header_.s.state = 0;
        header_.s.index = 0;
    }
    // 开始更新，需要和end_update配对使用
    void begin_update() {
        header_.s.state = 1;
    }
    // 更新完成后使用，需要和 begin_update配对使用
    void end_update()   {
        header_.s.index += 1;
        header_.s.state = 0;
    }
    // return false, 正在更新
    bool get_no_update_header(CasHeader & header) {
        header.dummy = header_.dummy;
        if (1 == header.s.state) return false;// 正在更新
        else                     return true;
    }

    bool check_equal(const CasHeader & header) {
        return header.dummy == header_.dummy;
    }
};


/*
 * gettimeofday, 会进行系统调用，当调用次数比较多的情况下，开销是比较大的
 *
 *          PseudoTime, 提供非精确的获取时间的功能，
 * 在精度要求不高的情况下，可以替代 gettimeofday，其做法是:
 * (1)专门启动一个线程，来隔段时间调用一次 gettimeofday
 * (2)其它线程只读
 *
 * */
class PseudoTime {
private:
    bool                continue_; // 为false时，循环线程退出
    static PseudoTime*  instance_; // 声明静态变量
    static Mutex        mutex_;
    uint64_t            curtimeMSec_;//当前时间，毫秒
#ifndef __x86_64__
    CasModel            casModel_;
#endif

private:
    PseudoTime() : continue_(true), curtimeMSec_(0) {
    }
    PseudoTime(const PseudoTime&);
    PseudoTime& operator=(const PseudoTime&);

public:
    static PseudoTime* getInstance(); // 单例模式
    uint64_t msec(); // 获取毫秒
    time_t    sec(); // 获取秒
    void     stop(); // 让取时间的线程退出

private:
    void update_time(); // 只有时间更新线程会调用该函数
    static void* threadfn(void *ptr);
};


} // namespace UTILS

#endif /*__UTILS_PSEUDO_TIME__*/
