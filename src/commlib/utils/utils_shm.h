#pragma once

#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <atomic>


//
// Linux 下（进程间）共享内存, 共享内存使用环形数组
//  支持一个进程写(可以多线程写); 多个进程读(每个进程中只有一个读线程)
//  对进程的启动顺序没有要求
//
// !!! 生产者重启的时候，消费者可能会丢数据

#define SHM_REGION_READY     0x53484D51 // 共享内存初始化完成（魔法数字）
#define UTILS_SHM_SLOTS_CNT  256*1024

static_assert(0 == (UTILS_SHM_SLOTS_CNT & (UTILS_SHM_SLOTS_CNT-1)), "UTILS_SHM_SLOTS_CNT must 2^N.");


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 单条数据
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
struct ShmSlot {
    alignas(64) std::atomic<uint32_t> data_version;

    TYPE              data;

    static constexpr uint32_t pad_size = (64 - ((sizeof(TYPE) + sizeof(std::atomic<uint32_t>)) % 64)) % 64;
    uint8_t            pad2[pad_size];
};

struct Data60 { char x[60]; };   // used=68, slot=128
struct Data120 { char x[120]; }; // used=128, slot=128

static_assert(sizeof(ShmSlot<uint8_t>) % 64 == 0, "");
static_assert(sizeof(ShmSlot<Data60>) % 64 == 0, "");
static_assert(sizeof(ShmSlot<Data120>) % 64 == 0, "");
static_assert(sizeof(std::atomic<uint32_t>) == 4, "sizeof(std::atomic<uint32_t>) must 4.");


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存区域数据结构
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
struct ShmRegion {
    alignas(64) std::atomic<uint32_t> is_ready; // init status.
    std::atomic<uint32_t> start_sec; // start time, second.
    std::atomic<uint32_t> write_idx;
    int32_t padding[13] = {0};

    ShmSlot<TYPE> slots[UTILS_SHM_SLOTS_CNT];
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存 生产者
// 支持多线程写入
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
class ShmPrd {
public:
    ~ShmPrd() { uninit(); }

    // TODO 对于生产者，在一个进程中，这个函数只能被调用一次
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
        const uint32_t new_idx = shm_region_->write_idx.fetch_add(1, std::memory_order_relaxed) + 1;

        ShmSlot<TYPE> &slot = shm_region_->slots[new_idx & (UTILS_SHM_SLOTS_CNT-1)];

        // slot.data = data;
        memcpy(&slot.data, &data, sizeof(TYPE));

        slot.data_version.store(new_idx, std::memory_order_release);
    }

private:
    void uninit() {
        if (shm_region_) {
            munmap(shm_region_, sizeof(ShmRegion<TYPE>));
            shm_region_ = nullptr;
        }

        if (shm_fd_ >= 0) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
    }

    void reset_shm_region() {
        // 初始化槽位版本号
        for (uint32_t i = 0; i < UTILS_SHM_SLOTS_CNT; ++i) {
            shm_region_->slots[i].data_version.store(0, std::memory_order_relaxed);
        }

        std::atomic_thread_fence(std::memory_order_seq_cst);

        // Server 重启的唯一标识
        shm_region_->start_sec.store(get_sec(), std::memory_order_release);
        shm_region_->write_idx.store(0, std::memory_order_release);
        shm_region_->is_ready.store(SHM_REGION_READY, std::memory_order_release);

        std::atomic_thread_fence(std::memory_order_seq_cst);
    }

    uint32_t get_sec() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec;
    }

private:
    char err_[256];

private:
    int32_t          shm_fd_ = -1;
    ShmRegion<TYPE> *shm_region_ = nullptr;
};



template<class TYPE>
using ProcessDataFunc=void(*)(const TYPE &);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 共享内存 消费者
// 支持多进程读（每个进程中，只能有一个读线程）
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class TYPE>
class ShmCon {
public:
    ~ShmCon() { uninit(); }

    // TODO 对于生产者，这个函数在一个进程中，只能被调用一次
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

        return true;
    }

    void uninit() {
        if (shm_region_) {
            munmap(shm_region_, sizeof(ShmRegion<TYPE>));
            shm_region_ = nullptr;
        }

        if (shm_fd_ >= 0) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
    }

    const char* err() const { return err_; }

    // support multi-process
    void shm_read_thread(ProcessDataFunc<TYPE> process_data) {
        thread_local TYPE data;

        uint32_t need_idx = 0;
        while(true) {
            uint32_t curr_w_idx = shm_region_->write_idx.load(std::memory_order_acquire);
            shm_start_sec_ = shm_region_->start_sec.load(std::memory_order_acquire);

            if (shm_start_sec_ != 0 && (SHM_REGION_READY == shm_region_->is_ready.load(std::memory_order_acquire))) {
                need_idx = (curr_w_idx == 0) ? 1 : (curr_w_idx + 1) ;
                break;
            }
            cpu_delay(100);
        }

        uint32_t retry_cnt = 0;
        while(true) {
            if (read_data(need_idx, data)) {
                process_data(data);
                ++need_idx;
            }
            else {
                if (retry_cnt++ <= 50) {
                    cpu_delay(10);
                    continue;
                }

                retry_cnt = 0;

                const uint32_t  tm = shm_region_->start_sec.load(std::memory_order_acquire);
                const uint32_t idx = shm_region_->write_idx.load(std::memory_order_acquire);
                const int32_t diff = (int32_t)(idx - need_idx);

                // server restarted.
                if (shm_start_sec_ != tm) {
                    shm_start_sec_ = tm;
                    need_idx = idx + 1;
                    fprintf(stdout, "shm_start_tm %u changed, need_idx %u. \n", shm_start_sec_, need_idx);
                }
                // read too slowly.
                else if (diff >= UTILS_SHM_SLOTS_CNT - 1) {
                    fprintf(stdout, "shm_read too slowly. need[%u] real[%u] \n", need_idx, idx);
                    need_idx = idx + 1;
                }
            }
        }
    }

private:
    bool read_data(const uint32_t need_idx, TYPE &out) {
        ShmSlot<TYPE> &slot = shm_region_->slots[need_idx & (UTILS_SHM_SLOTS_CNT-1)];

        const uint32_t ver1 = slot.data_version.load(std::memory_order_acquire);
        if (ver1 != need_idx) {
            return false;
        }

        memcpy(&out, &slot.data, sizeof(TYPE));

        const uint32_t ver2 = slot.data_version.load(std::memory_order_acquire);

        return (ver1 == ver2);
    }

    void cpu_delay(uint64_t delay) {
        for (uint64_t i = 0; i < delay; i++) {
            __asm__ __volatile__("pause" ::: "memory");
        }
    }

private:
    char err_[256];

private:
    int32_t          shm_fd_ = -1;
    ShmRegion<TYPE> *shm_region_ = nullptr;
    uint32_t shm_start_sec_ = 0; // start time, second.
};
