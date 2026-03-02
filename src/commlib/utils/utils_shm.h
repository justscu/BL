#pragma once


#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Linux 下（进程间）共享内存
//  支持一个进程写(可以多线程写); 多个进程读
//  对进程的启动顺序没有要求
//      共享内存使用环形数组
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#define SHM_SLOTS_CNT 1024*1024

static_assert(0 == (SHM_SLOTS_CNT & (SHM_SLOTS_CNT-1)), "SHM_SLOTS_CNT must 2^N.");

enum SlotStu : uint32_t {
    kSlotStu_init = 0,
    kSlotStu_writing = 1,
    kSlotStu_ready   = 2
};


template<class TYPE>
struct ShmSlot {
    volatile SlotStu  data_status  = kSlotStu_init;
    volatile uint32_t data_version = 0;

    TYPE              data;

    static constexpr uint32_t pad_size = (64 - ((sizeof(TYPE) + sizeof(SlotStu) + sizeof(uint32_t)) % 64)) % 64;
    uint8_t            pad2[pad_size];
};

struct Data60 { char x[60]; };   // used=68, slot=128
struct Data120 { char x[120]; }; // used=128, slot=128

static_assert(sizeof(ShmSlot<uint8_t>) % 64 == 0, "");
static_assert(sizeof(ShmSlot<Data60>) % 64 == 0, "");
static_assert(sizeof(ShmSlot<Data120>) % 64 == 0, "");


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存区域数据结构
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
struct ShmRegion {
    alignas(64) volatile uint32_t write_idx = 0;
    int32_t padding[15] = {0};

    ShmSlot<TYPE> slots[SHM_SLOTS_CNT];
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存 生产者
// 支持多线程写入
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
class ShmPrd {
public:
    ~ShmPrd() { uninit(); }

    bool init(const char *shm_name) {
        shm_fd_ = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (-1 == shm_fd_) {
            snprintf(err_, sizeof(err_)-1, "shm_open failed: %s.", strerror(errno));
            return false;
        }

        // stat
        struct stat st;
        if (-1 == fstat(shm_fd_, &st)) {
            snprintf(err_, sizeof(err_)-1, "fstat failed: %s.", strerror(errno));
            uninit();
            return false;
        }

        // resize.
        if (st.st_size < sizeof(ShmRegion<TYPE>)) {
            if (-1 == ftruncate(shm_fd_, sizeof(ShmRegion<TYPE>))) {
                snprintf(err_, sizeof(err_)-1, "ftruncate failed: %s.", strerror(errno));
                uninit();
                return false;
            }
        }

        // mmap.
        shm_region_ = (ShmRegion<TYPE>*)mmap(nullptr, sizeof(ShmRegion<TYPE>), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (MAP_FAILED == shm_region_) {
            snprintf(err_, sizeof(err_)-1, "mmap failed: %s.", strerror(errno));
            uninit();
            return false;
        }

        reset_shm_region();
        snprintf(err_, sizeof(err_)-1, "ShmPrd::init success.(New Created)");

        return true;
    }

    const char* err() const { return err_; }

    // support multi-thread.
    void shm_write(const TYPE &data) {
        const uint32_t new_idx = __sync_add_and_fetch(&(shm_region_->write_idx), 1);

        ShmSlot<TYPE> &slot = shm_region_->slots[new_idx & (SHM_SLOTS_CNT-1)];
        slot.data_status = kSlotStu_writing;

        __sync_synchronize();

        {
            // slot.data = data;
            memcpy(&slot.data, &data, sizeof(TYPE));
            slot.data_version = new_idx;
        }

        __sync_synchronize();

        // ready.
        slot.data_status = kSlotStu_ready;
    }

private:
    void uninit() {
        if (shm_region_) {
            munmap(shm_region_, sizeof(ShmRegion<TYPE>));
            shm_region_ = nullptr;
        }

        if (shm_fd_ > 0) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
    }

    void reset_shm_region() {
        memset(shm_region_, 0x00, sizeof(ShmRegion<TYPE>));
    }

private:
    char err_[256];

private:
    int32_t          shm_fd_ = 0;
    ShmRegion<TYPE> *shm_region_ = nullptr;
};



template<class TYPE>
using ProcessDataFunc=void(*)(const TYPE &);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存 消费者
// 支持多进程读
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
class ShmCon {
public:
    ~ShmCon() { uninit(); }

    bool init(const char *shm_name) {
        shm_fd_ = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (-1 == shm_fd_) {
            snprintf(err_, sizeof(err_)-1, "shm_open failed: %s.", strerror(errno));
            return false;
        }

        // stat
        struct stat st;
        if (-1 == fstat(shm_fd_, &st)) {
            snprintf(err_, sizeof(err_)-1, "fstat failed: %s.", strerror(errno));
            uninit();
            return false;
        }

        bool need_reset = false;
        // resize.
        if (st.st_size < sizeof(ShmRegion<TYPE>)) {
            need_reset = true;
            if (-1 == ftruncate(shm_fd_, sizeof(ShmRegion<TYPE>))) {
                snprintf(err_, sizeof(err_)-1, "ftruncate failed: %s.", strerror(errno));
                uninit();
                return false;
            }
        }

        // mmap.
        shm_region_ = (ShmRegion<TYPE>*)mmap(nullptr, sizeof(ShmRegion<TYPE>), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (MAP_FAILED == shm_region_) {
            snprintf(err_, sizeof(err_)-1, "mmap failed: %s.", strerror(errno));
            uninit();
            return false;
        }

        if (need_reset) {
            reset_shm_region();
            snprintf(err_, sizeof(err_)-1, "ShmPrd::init success.(New Created)");
        }
        else {
            snprintf(err_, sizeof(err_)-1, "ShmPrd::init success.(New Attached)");
        }

        return true;
    }

    void uninit() {
        if (shm_region_) {
            munmap(shm_region_, sizeof(ShmRegion<TYPE>));
            shm_region_ = nullptr;
        }

        if (shm_fd_ > 0) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
    }

    const char* err() const { return err_; }

    // support multi-process
    void shm_read_thread(ProcessDataFunc<TYPE> process_data) {
        thread_local TYPE data;
        uint32_t need_idx = shm_region_->write_idx;

        uint32_t retry_cnt = 0;

        while(true) {
            if (read_data(need_idx, data)) {
                process_data(data);
                ++need_idx;
            }
            else {
                const uint32_t idx = shm_region_->write_idx;
                // next data not ready.
                if (idx < need_idx) {
                    cpu_delay(10);
                    if (++retry_cnt >= 50) {
                        need_idx = shm_region_->write_idx + 1;
                        retry_cnt = 0;
                    }
                }
                // read too slowly.
                else if (idx >= need_idx + SHM_SLOTS_CNT - 1) {
                    need_idx = shm_region_->write_idx + 1;
                    // TODO err log.
                    fprintf(stdout, "read reset. %u. \n", need_idx);
                }
            }
        }
    }

private:
    bool read_data(const uint32_t need_idx, TYPE &out) {
        ShmSlot<TYPE> &slot = shm_region_->slots[need_idx & (SHM_SLOTS_CNT-1)];

        uint32_t cnt = 0;
        while (true) {
            if (slot.data_status == kSlotStu_ready) {
                __sync_synchronize();

                // out = slot.data;
                memcpy(&out, &slot.data, sizeof(TYPE));

                const uint32_t ver = slot.data_version;

                __sync_synchronize();
                if (slot.data_status == kSlotStu_ready) {
                    return need_idx == ver;
                }

                continue; // need read again.
            }
            else if (slot.data_status == kSlotStu_writing) {
                if (cnt++ <= 50) {
                    cpu_delay(50);
                    continue;
                }
            }

            return false;
        }
    }

    void cpu_delay(uint64_t delay) {
        for (uint64_t i = 0; i < delay; i++) {
            __asm__ __volatile__("pause" ::: "memory");
        }
    }

    void reset_shm_region() {
        memset(shm_region_, 0x00, sizeof(ShmRegion<TYPE>));
    }

private:
    char err_[256];

private:
    int32_t          shm_fd_ = 0;
    ShmRegion<TYPE> *shm_region_ = nullptr;
};
