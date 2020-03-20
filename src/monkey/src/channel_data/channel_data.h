#ifndef __CHANNEL_DATA_H__
#define __CHANNEL_DATA_H__

#include <mutex>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <iomanip>
#include <string.h>

typedef uint16_t ChnnIDType; // channel-id type

enum class eChnnData : int64_t {
    data_cell_max_size = 100000, // 100w
    data_arr_max_size  = 400,
    chnn_max_size = 32,
};

template<typename CellType>
class DataArr {
public:
    DataArr() = default;
    ~DataArr() { unInit(); }

    DataArr(const DataArr&) = delete;
    DataArr& operator=(const DataArr&) = delete;

    bool init() {
        unInit();

        addr_ = new (std::nothrow) CellType[max_size_];
        if (addr_) {
            memset(addr_, 0x00, sizeof(CellType) * max_size_);
        }
        return (addr_ != nullptr);
    }

    void unInit() {
        if (addr_) {
            delete [] addr_;
            addr_ = nullptr;
        }
        channel_id_ = 0;
        used_ = max_seq_ = 0;
    }

    // get addr
    CellType* operator[] (int64_t idx) const {
        if (idx >= 0 && idx < max_size_) {
            return &(addr_[idx]);
        }
        return nullptr;
    }

    void used_info() {
        int64_t c1 = 0; // used.
        int64_t c2 = 0; // max

        for (int64_t i = 0; i < max_size_; ++i) {
            const CellType & d = addr_[i];
            if (d.trade.seq > 0) {
                ++c1;
                if (d.trade.seq > c2) {
                    c2 = d.trade.seq;
                }
            }
        }

        used_ = c1;
        max_seq_ = c2;
    }

    inline void set_channel_id(ChnnIDType id) { channel_id_ = id; }
    inline ChnnIDType channel_id() const { return channel_id_; }

    // how many cells used.
    inline int64_t used() const { return used_; }
    // max sequence number used.
    inline int64_t max_seq() const { return max_seq_; }

    // max size for each array.
    int64_t max_size() const { return max_size_; }

private:
    ChnnIDType  channel_id_ = 0;
    int64_t     used_       = 0; // how many cells used.
    int64_t     max_seq_    = 0; // max sequence number used.

    CellType*         addr_ = nullptr;
    const int64_t max_size_ = static_cast<const int64_t>(eChnnData::data_cell_max_size);
};


// data-array manager
template<typename CellType>
class DataArrMgr {
public:
    DataArrMgr() = default;

    DataArrMgr(const DataArrMgr&) = delete;
    DataArrMgr& operator=(const DataArrMgr&) = delete;

    // max_sequence: each day's max sequence for "entrust + trade".
    bool init(int64_t max_sequence) {
        unInit();
        // check seq.
        {
            const int64_t cell = static_cast<const int64_t>(eChnnData::data_cell_max_size);
            const int64_t  arr = static_cast<const int64_t>(eChnnData::data_arr_max_size);
            arr_max_ = max_sequence / cell;
            if (max_sequence % cell > 0) { ++arr_max_; }
            if (arr_max_ < 10 || arr_max_ > arr) {
                fprintf(stderr, "max_sequence [%ld] error. \n", max_sequence);
                return false;
            }
        }

        arr_ = new (std::nothrow) DataArr<CellType> [arr_max_];
        if (arr_) {
            for (int64_t i = 0; i < arr_max_; ++i) {
                if (!arr_[i].init()) {
                    return false;
                }
            }
        }

        return (arr_ != nullptr);
    }

    void unInit() {
        if (arr_) {
            delete [] arr_;
            arr_ = nullptr;
        }
        arr_used_ = 0;
        arr_max_ = 0;
    }

    // multi-thread safe.
    DataArr<CellType>* get_new_data_array() {
        const int64_t id = __sync_fetch_and_add(&arr_used_, 1);
        if (id >= arr_max_) {
            __sync_add_and_fetch(&arr_used_, -1);
            return nullptr;
        }
        return &(arr_[id]);
    }

    inline int64_t arr_used() const { return arr_used_; }

    void dump() const {
        fprintf(stdout, "DataArrMgr : used[%ld]. \n", arr_used());
        for (int64_t i = 0; i < arr_used(); ++i) {
            arr_[i].used_info();
            fprintf(stdout, "    DataArr[%ld]. addr[%p] chnn_id[%ld]. used[%ld], max[%ld] \n",
                    i, &(arr_[i]), arr_[i].channel_id(), arr_[i].used(), arr_[i].max_seq());
        }
    }

private:
    DataArr<CellType> *arr_ = nullptr;
    volatile int64_t   arr_used_ = 0;
    int64_t            arr_max_  = 0;
};


// for one channel
template<typename CellType>
class ChnnData {
public:
    ChnnData() = default;

    inline void set_data_array_mgr(DataArrMgr<CellType>* mgr) { mgr_ = mgr; }

    bool add(int64_t seq, const CellType* d) {
        if (seq < 0) { return false; }
        const int64_t  idx_id = seq / static_cast<const int64_t>(eChnnData::data_cell_max_size);
        const int64_t cell_id = seq % static_cast<const int64_t>(eChnnData::data_cell_max_size);

        if (idx_id >= idx_max_) { return false; }

        if (!idx_[idx_id]) {
            idx_[idx_id] = mgr_->get_new_data_array();
            if (idx_[idx_id]) {
                idx_[idx_id]->set_channel_id(channel_id());
            }
            else {
                fprintf(stdout, "data array exhausted. \n");
                return false;
            }
        }

        memcpy((*(idx_[idx_id]))[cell_id], d, sizeof(CellType));
        return true;
    }

    CellType* get(int64_t seq) {
        if (seq < 0) { return nullptr; }
        const int64_t  idx_id = seq / static_cast<const int64_t>(eChnnData::data_cell_max_size);
        const int64_t cell_id = seq % static_cast<const int64_t>(eChnnData::data_cell_max_size);

        if (idx_id >= idx_max_ || !idx_[idx_id]) { return nullptr; }

        return (*(idx_[idx_id]))[cell_id];
    }

    void used_info() {
        used_ = 0, max_seq_ = 0;
        for (int64_t i = 0; i < idx_max_; ++i) {
            if (idx_[i]) {
                idx_[i]->used_info();
                int64_t    used = idx_[i]->used();
                int64_t max_seq = idx_[i]->max_seq();

                used_ += used;
                if (max_seq > max_seq_) {
                    max_seq_ = max_seq;
                }

                fprintf(stdout, "     %ld: [%p] used[%ld] max[%ld]. \n", i, idx_[i], used, max_seq);
            }
        }
    }

    inline void set_channel_id(ChnnIDType id) { chnn_id_ = id; }
    inline ChnnIDType channel_id() const { return chnn_id_; }

    inline int64_t used() const { return used_; }
    inline int64_t max_seq() const { return max_seq_; }

    void dump() {
        fprintf(stdout, "= %u = . \n", channel_id());
        used_info();
        fprintf(stdout, "  total: used[%ld] max[%ld]. \n", used(), max_seq());
    }

private:
    ChnnIDType chnn_id_ = 0;

    int64_t     used_ = 0; // how many data received.
    int64_t  max_seq_ = 0; // max sequence number received

    static const int64_t  idx_max_ = 100;
    DataArr<CellType>*    idx_[idx_max_] = { nullptr }; // save the address to DataArr.

private:
    DataArrMgr<CellType>* mgr_ = nullptr;
};


// for all channels.
// not multi-thread safe.
template<typename CellType>
class ChnnDataMgr {
using ChnnType = ChnnData<CellType>;

public:
    // data_arr_mgr: all data array.
    bool init(DataArrMgr<CellType> *data_arr_mgr) {
        if (!(data_arr_mgr_=data_arr_mgr)) { return false; }
        unInit();

        for (int64_t i = 0; i < static_cast<const int64_t>(eChnnData::chnn_max_size); ++i) {
            chnn_arr_[i].set_data_array_mgr(data_arr_mgr_);
        }

        return true;
    }

    void unInit() {
        chnn_arr_used_ = 0;
    }

    // not support multi-threads.
    bool add(ChnnIDType chid, int64_t seq, const CellType* d) {
        typename std::unordered_map<ChnnIDType, ChnnType*>::iterator it = map_.find(chid);

        if (it == map_.end()) {
            if (chnn_arr_used_ >= static_cast<const int64_t>(eChnnData::chnn_max_size)) {
                fprintf(stderr, "chnn arr exhausted. \n");
                return false;
            }

            ChnnType& c = chnn_arr_[chnn_arr_used_++];
            auto rst = map_.insert({chid, &c}); // multi-threads.
            if (rst.second) {
                c.set_channel_id(chid);
                it = rst.first;
            }
        }

        if (it != map_.end()) {
            return it->second->add(seq, d);
        }

        return false;
    }

    inline ChnnType* get(ChnnIDType chid) {
        typename std::unordered_map<ChnnIDType, ChnnType*>::iterator it = map_.find(chid);
        if (it != map_.end()) return it->second;
        return nullptr;
    }

    inline CellType* get(ChnnIDType chid, int64_t seq) {
        if (ChnnType* ch = get(chid)) { return ch->get(seq); }
        return nullptr;
    }

    void dump() const {
        fprintf(stdout, "共有 %ld 个channel. chnn_arr_used[%ld]. \n", map_.size(), chnn_arr_used_);
        for (auto it = map_.cbegin(); it != map_.cend(); ++it) {
            it->second->dump();
        }

        data_arr_mgr_->dump();
    }

private:
    DataArrMgr<CellType>*            data_arr_mgr_;
    std::unordered_map<ChnnIDType, ChnnType*> map_; // chnn-id & ChnnData

private:
    ChnnType chnn_arr_[static_cast<const int64_t>(eChnnData::chnn_max_size)]; // support max 32 channels.
    int64_t  chnn_arr_used_ = 0;
};

#endif // __CHANNEL_DATA_H__
