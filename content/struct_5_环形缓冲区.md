## 环形缓冲区

[环形缓冲区](http://bbs.chinaunix.net/forum.php?mod=viewthread&tid=1950140)

`read/write`一般都可以带缓冲区的，其目的是为了提高效率。试想，当我们向文件中写入数据时，若我们每次写1个字节，但system并不是每次调用write的时候，都去真的写文件，先把数据放入缓冲区中，当缓冲区里面的字节达到一定数量时（上限），才会去真的写文件。

在socket接收数据的时候，常会出现半包的时候。这就要求我们不能够将前面接收的数据丢弃，而是放到一个缓冲区中，等一个完整的包接收完毕后，才能够进行处理。

用数组实现ring-buffer
```cpp
#pragma once
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

class RingBuffer
{
public:
    explicit RingBuffer(size_t max_len = 2048):
        m_buf(NULL), m_max_len(max_len), m_put_pos(0), m_get_pos(0), m_cur_len(0)
    {
        new_buf(m_max_len);
    }
    ~RingBuffer() {
        delete_buf();
    }
    // @return, 本次存放的长度
    size_t put(const char* buf, size_t buf_len) {
        size_t allo_len = m_max_len;
        size_t res_len = allo_len - m_cur_len;// 剩下长度
        // 超过剩下长度
        if (buf_len > res_len)
        {
            while (buf_len > res_len)
            {
                allo_len <<= 1;
                res_len = allo_len - m_cur_len;
            }
            expand_buf(allo_len); //扩展
        }
        if (m_max_len - m_put_pos >= buf_len)
        {
            memcpy(m_buf + m_put_pos, buf, buf_len);
            m_cur_len += buf_len;
            m_put_pos = (m_put_pos + buf_len) % m_max_len;
        }
        else //需要分2段存
        {
            memcpy(m_buf + m_put_pos, buf, m_max_len - m_put_pos);
            memcpy(m_buf, buf + m_max_len - m_put_pos, buf_len - (m_max_len
             - m_put_pos));
            m_cur_len += buf_len;
            m_put_pos = (m_put_pos + buf_len) % m_max_len;
        }
        return buf_len;
    }
    // @return , 本次获取的长度
    size_t get(char* buf, size_t buf_len) {
        // 本次获取的长度
        size_t get_len = (buf_len < m_cur_len) ? buf_len : m_cur_len;
        if (m_get_pos + get_len <= m_max_len)
        {
            memcpy(buf, m_buf + m_get_pos, get_len);
        }
        else
        {
            memcpy(buf, m_buf + m_get_pos, m_max_len - m_get_pos);
            memcpy(buf + m_max_len - m_get_pos, m_buf, get_len - (m_max_len
             - m_get_pos));
        }
        m_cur_len -= get_len;
        m_get_pos = (m_get_pos + buf_len) % m_max_len;
        return get_len;
    }
    friend void RingBuffer_test();
private:
    void delete_buf() {
        if (m_buf)
        {
            delete[] m_buf;
            m_buf = NULL;
        }
    }
    char* new_buf(size_t size) {
        delete_buf();
        m_buf = new char[size];
        assert(m_buf != NULL);
        return m_buf;
    }
    char* expand_buf(size_t new_size) {
        char* buf = new char[new_size];
        assert(buf);
        if (m_put_pos > m_get_pos)
        {
            memcpy(buf, m_buf + m_get_pos, m_cur_len);
        }
        else if (m_put_pos < m_get_pos)
        {
            memcpy(buf, m_buf + m_get_pos, m_max_len - m_get_pos);
            memcpy(buf + m_max_len - m_get_pos, m_buf, m_put_pos);
        }
        delete_buf();
        m_buf = buf;
        m_get_pos = 0;
        m_put_pos = m_cur_len;
        m_max_len = new_size;
        return m_buf;
    }
private:
    RingBuffer(const RingBuffer&);
    const RingBuffer & operator=(const RingBuffer&);
private:
    char* m_buf;
    size_t m_max_len; //缓冲区大小
    size_t m_put_pos;//可以从该位置开始存放
    size_t m_get_pos;//可以取的值[m_get_pos, m_put_pos)
    size_t m_cur_len;//当前存放的长度
};

void RingBuffer_test() {
    std::string s;
    RingBuffer r(10);
    FILE* pf = fopen("/home/ll/work/min_heap/ringbuffer.h", "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    size_t iRand = 0;
    while ((read = getline(&line, &len, pf)) != -1)
    {
        r.put(line, strlen(line));
        char buf[101] =
        { 0 };
        iRand = rand() % 51;
        iRand = (iRand <= r.m_cur_len) ? iRand : r.m_cur_len;
        size_t iGet = r.get(buf, iRand);
        s.append(buf, iGet);
    }

    if (line)
        free(line);

    while (r.m_cur_len > 0)
    {
        size_t get_len =
         (r.m_max_len - r.m_get_pos) < r.m_cur_len ? (r.m_max_len
         - r.m_get_pos) : r.m_cur_len;
        char buf[get_len];
        memset(buf, 0x00, get_len);
        r.get(buf, get_len);
        s.append(buf, get_len);
    }
    std::cout << s << std::endl;
}
```
