#pragma once

#include <stdint.h>
#include "etherfabric/ef_vi.h"
#include "etherfabric/capabilities.h"
#include "etherfabric/base.h"
#include "etherfabric/pd.h"
#include "etherfabric/vi.h"
#include "etherfabric/efct_vi.h"
#include "etherfabric/memreg.h"
#include "commx/utils_net_packet_hdr.h"


// efvi 收发数据时，使用的单元
// 每个单元大小 2K.
// 包含: mac_hader|ip_header|udp_header|udp_payload
// 需要 1字节对齐
#pragma pack(push, 1)

struct PKT_BUF {
    uint64_t state = 0; // 0, useable.
    ef_addr  dma_buf_addr; // dma相关函数，使用该地址
    char  user_buf[2048-16]; // 用户空间，使用该地址, 16字节对齐
};

#pragma pack(pop)

static_assert(sizeof(PKT_BUF)==2048, "sizeof(PKT_BUF) != 2048");

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
    static const int32_t rx_q_capacity = 128; // must 2^N.

    bool mmap_flag_ = false;

    // rx
    ef_memreg rx_memreg_;
    PKT_BUF  *rx_bufs_ = nullptr;

private:
    char err_[256] = {0};
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// efvi 发送模式: DMA or CTPIO.
// 适用于X2/X3.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class EfviUdpSend {
public:
    EfviUdpSend() { }
    ~EfviUdpSend() { uninit(); }
    EfviUdpSend(const EfviUdpSend&) = delete;
    EfviUdpSend& operator=(const EfviUdpSend&) = delete;

    // 返回efvi的版本信息: onload -v
    const char* efvi_version();
    const char* efvi_driver_interface();
    const char* efvi_nic_arch();
    const char* efvi_support_ctpio(const char *interface);

    const char *err() const { return err_; } // 出错时，返回错误信息

    bool init(const char *interface);
    void uninit();

    // local ip & local port
    bool add_filter(const char *ip, uint16_t port);

    bool get_usable_send_buf(PKT_BUF * &buf, uint32_t &dma_id);

    // 使用DMA模式发送数据
    // DMA模式，efvi驱动会填充 ip.checksum & udp.checksum.
    // pkt_len, include mac & ip & udp header.
    bool dma_send(int32_t pkt_len, uint32_t dma_id);

    // 使用CTPIO模式发送数据, 需要调用者保证包头的正确性
    // CTPIO模式，需要调用者自己保证ip.checksum & udp.checksum的正确性
    bool ctpio_send(int32_t pkt_len, uint32_t dma_id);

    void poll();

private:
    bool alloc_tx_buffer();

private: // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd  pd_; // protect domain.
    ef_vi  vi_; // virtual interface.

private: // tx
    static const uint32_t tx_q_capacity = 16;

    ef_memreg tx_memreg_; // Memory that has been registered for use with ef_vi
    PKT_BUF  *tx_bufs_ = nullptr;

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
