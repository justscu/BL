#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>

namespace DS {

// Caution: 环形缓冲区，该缓冲区不支持多线程操作 !
//          若要在多线程中使用，需要加锁 ！
class RingBuffer {
private:
    char*         buf_;
    uint32_t buf_size_;
    uint32_t     rPos_; // 开始位置
    uint32_t     wPos_; // 结束位置
    bool        bFull_; // 是否满了

public:
    RingBuffer(uint32_t size = 1024*1024);
    ~RingBuffer();

    inline bool is_full()  const { return  bFull_; }
    inline bool is_empty() const { return (bFull_ == false) && (wPos_ == rPos_); }

    uint32_t used_size() const;
    inline uint32_t free_size() const { return buf_size_ - used_size(); }
    inline uint32_t buf_size()  const { return buf_size_; }

    inline void reset() { rPos_ = wPos_ = 0; bFull_ = false; }
    // 若成功，则返回扩展后的size.
    int32_t expand();
    // 返回实际写入的字节数
    uint32_t write(const char* dat, const uint32_t datLen);
    // 返回实际读入的字节数
    uint32_t read(char* dat, const uint32_t datLen);
};

} // namespace DS

#endif /*__RING_BUFFER_H__*/
