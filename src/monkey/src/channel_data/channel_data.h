#ifndef __CHANNEL_DATA_H__
#define __CHANNEL_DATA_H__

#include <mutex>
#include <functional>
#include <unordered_map>
#include <mutex>
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

    // max 400.
    bool init(int64_t arr_max_size) {
        unInit();

        arr_max_ = arr_max_size;
        if (arr_max_ < 10 || arr_max_ > static_cast<const int64_t>(eChnnData::data_arr_max_size)) {
            return false;
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

    // support multi-thread.
    DataArr<CellType>* get_new_data_array() {
        int64_t id = -1;
        mutex_.lock();
        if (arr_used_ < arr_max_) {
            id = arr_used_++;
        }
        mutex_.unlock();

        return (id == -1) ? nullptr : &(arr_[id]);
    }

private:
    DataArr<CellType> *arr_ = nullptr;
    int64_t            arr_used_ = 0;
    int64_t            arr_max_  = 0;

private:
    std::mutex         mutex_;
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

                fprintf(stdout, "     %ld: used[%ld] max[%ld]. \n", i, used, max_seq);
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
template<typename CellType>
class ChnnDataMgr {
using ChnnType = ChnnData<CellType>;

public:
    // each day's max sequence for entrust + trade.
    bool init(int64_t max_sequence) {
        unInit();

        if (max_sequence <= 0) { return false; }

        int64_t max_arr = max_sequence / static_cast<const int64_t>(eChnnData::data_cell_max_size);
        if (max_sequence % static_cast<const int64_t>(eChnnData::data_cell_max_size) > 0) {
            ++max_arr;
        }

        if (!data_arr_mgr_.init(max_arr)) { return false; }

        for (int64_t i = 0; i < static_cast<const int64_t>(eChnnData::chnn_max_size); ++i) {
            chnn_arr_[i].set_data_array_mgr(&data_arr_mgr_);
        }

        return true;
    }

    void unInit() {
        data_arr_mgr_.unInit();
        chnn_arr_used_ = 0;
    }

    bool add(ChnnIDType chid, int64_t seq, const CellType* d) {
        typename std::unordered_map<ChnnIDType, ChnnType*>::iterator it = map_.find(chid);

        if (it == map_.end()) {
            int64_t id = -1;
            mutex_.lock();
            if (chnn_arr_used_ < static_cast<const int64_t>(eChnnData::chnn_max_size)) {
                id = chnn_arr_used_++;
            }
            mutex_.unlock();

            if (id == -1) { return false; }
            ChnnType& c = chnn_arr_[id];
            auto rst = map_.insert({chid, &c});
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
        fprintf(stdout, "共有 %ld 个channel. \n", map_.size());
        for (auto it = map_.cbegin(); it != map_.cend(); ++it) {
            it->second->dump();
        }
    }

private:
    DataArrMgr<CellType>             data_arr_mgr_;
    std::unordered_map<ChnnIDType, ChnnType*> map_; // chnn-id & ChnnData

private:
    ChnnType chnn_arr_[static_cast<const int64_t>(eChnnData::chnn_max_size)]; // support max 32 channels.
    int64_t  chnn_arr_used_ = 0;

    std::mutex mutex_;
};

#endif // __CHANNEL_DATA_H__
