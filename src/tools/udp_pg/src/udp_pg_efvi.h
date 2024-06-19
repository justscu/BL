#pragma once

#include <stdint.h>
#include "etherfabric/ef_vi.h"
#include "etherfabric/capabilities.h"
#include "etherfabric/base.h"
#include "etherfabric/pd.h"
#include "etherfabric/vi.h"
#include "etherfabric/memreg.h"
#include "commx/utils_net_packet_hdr.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// for efvi, record RX RQ id.
// not support multi-thread.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class FreeRxIDs {
public:
    struct ID {
        int32_t value = 0;
    };

public:
    bool add(int32_t v);

    constexpr int32_t capacity() { return sizeof(ids_)/sizeof(ids_[0]); }
    int32_t used() const { return used_; }
    void clear() { used_ = 0; }

    ID* begin() { return &(ids_[0]); }
    ID* end() { return &(ids_[used_]); }

private:
    ID      ids_[16];
    int32_t used_ = 0;
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 功能: efvi收到数据后的回调函数
// str: 包含mac头的数据
// len: 数据长度
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
using RecvCBFunc = void(*)(const char *str, int32_t len);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// recv udp packets by Solorflare-efvi
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class EfviUdpRecv {
public:
    EfviUdpRecv() = default;
    ~EfviUdpRecv() { uninit(); }
    EfviUdpRecv(const EfviUdpRecv&) = delete;
    EfviUdpRecv& operator=(const EfviUdpRecv&) = delete;

    // 返回efvi的版本信息: onload -v
    const char* efvi_version();
    const char* efvi_driver_interface();

    const char *err() const { return err_; } // 出错时，返回错误信息

    bool init(const char *interface);
    void uninit();

    // local ip & local port
    // 如果接收组播的话，为组播地址和端口
    bool add_filter(const char *ip, uint16_t port);

    void recv(RecvCBFunc cb);

private:
    bool alloc_rx_buffer();

private:
    // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd pd_; // protect domain.
    ef_vi vi_; // virtual interface.

private:
    // recv queue cnt
    static const int32_t rx_q_capacity = 1024*4; // must 2^N.
    static const int32_t  pkt_buf_size = 2*1024; // each size 2K.

    bool mmap_flag_ = false;
    // rx
    char *rx_bufs_ = nullptr;
    ef_memreg    rx_memreg_;
    ef_addr *rx_dma_buffer_ = nullptr; // store DMA address.

private:
    char err_[256] = {0};
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// First In, Last Out.
// not support multi-thread.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class FILO {
public:
    struct ID {
        uint32_t value = 0;
    };

public:
    FILO(uint32_t cap);
    ~FILO() { uninit(); }

    bool init();
    void uninit();

    void clear() { used_ = 0; }

    // save value
    bool add(uint32_t v);
    bool get(uint32_t &v);

private:
    const uint32_t capacity = 0;
    ID       *ids_ = nullptr;
    uint32_t used_ = 0;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// efvi 发送数据时，使用的单元
// 每个单元大小 2K.
// 包含: mac_hader|ip_header|udp_header|udp_payload
// 需要 1字节对齐
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma pack(push, 1)
struct EfviSendDataCell {
    mac_hdr   mac;
    ip_hdr     ip;
    udp_hdr   udp;
    char  payload[2*1024-sizeof(mac_hdr)-sizeof(ip_hdr)-sizeof(udp_hdr)];
};
#pragma pack(pop)

static_assert(2*1024==sizeof(EfviSendDataCell), "");


class EfviUdpSend {
public:
    EfviUdpSend() : free_tx_dma_ids_(tx_q_capacity) { }
    ~EfviUdpSend() { uninit(); }
    EfviUdpSend(const EfviUdpSend&) = delete;
    EfviUdpSend& operator=(const EfviUdpSend&) = delete;

    // 返回efvi的版本信息: onload -v
    const char* efvi_version();
    const char* efvi_driver_interface();

    const char *err() const { return err_; } // 出错时，返回错误信息

    bool init(const char *interface);
    void uninit();

    // local ip & local port
    bool add_filter(const char *ip, uint16_t port);

    bool get_usable_send_buf(EfviSendDataCell * &addr, uint32_t &dma_id);

    // 调用该函数发送数据
    // pkt_len, include mac & ip & udp header.
    bool send(int32_t pkt_len, uint32_t dma_id);
    void poll();

private:
    bool alloc_tx_buffer();

private: // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd  pd_; // protect domain.
    ef_vi  vi_; // virtual interface.

private: // tx
    static const uint32_t tx_q_capacity = 16;
    EfviSendDataCell     *tx_buf_ = nullptr; // array
    ef_memreg tx_memreg_; // Memory that has been registered for use with ef_vi

    ef_addr tx_dma_buffer_[tx_q_capacity] = {0}; // array, store DMA address.
    FILO free_tx_dma_ids_; // record free dma ids.

    // for poll
    ef_request_id ids_[16];
    ef_event      evs_[16];

private:
    char err_[256] = {0};
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 加入组播
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class JoinMulticast {
public:
    JoinMulticast() { }
    ~JoinMulticast() { close_socket(); }

    bool create_socket();
    void close_socket();

    int32_t fd() { return fd_; }

    // interface: 从哪个网口接收组播
    // group_ip: 组播地址
    bool join(const char *interface, const char *group_ip, uint16_t group_port);

    const char *err() const { return err_; } // 出错时，返回错误信息

private:
    int32_t fd_ = -1;
private:
    char err_[256] = {0};
};
