#pragma once

#include <stdint.h>
#include "etherfabric/ef_vi.h"
#include "etherfabric/capabilities.h"
#include "etherfabric/base.h"
#include "etherfabric/pd.h"
#include "etherfabric/vi.h"
#include "etherfabric/memreg.h"

class FreeIDs {
public:
    struct ID {
        int32_t value = 0;
    };

public:
    bool add(int32_t v) {
        if (used_ < capacity()) {
            ids_[used_++].value = v;
            return true;
        }
        return false;
    }

    constexpr int32_t capacity() { return sizeof(ids_)/sizeof(ids_[0]); }
    int32_t used() const { return used_; }
    void clear() { used_ = 0; }

    ID* begin() { return &(ids_[0]); }
    ID* end() { return &(ids_[used_]); }

private:
    ID      ids_[16];
    int32_t used_ = 0;
};


class EvfiUdpRecv {
public:
    void recv(const char *eth_name, uint16_t port);

private:
    bool init(const char *eth_name);
    bool set_filter(const char *ip, uint16_t port);
    bool set_rx_buffer();

private:
    // rx
    void     *rx_buf_ = nullptr;
    int32_t   rx_cnt_ = 512;
    ef_memreg rx_memreg_;
    ef_addr   rx_dma_buffer_[512];

private: // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd             pd_; // protect domain.
    ef_pd_flags pd_flags_ = EF_PD_DEFAULT;

    ef_vi    vi_;
    uint32_t vi_flags_ = EF_VI_FLAGS_DEFAULT | EF_VI_TX_TIMESTAMPS;
};


class EfviUdpSend {
public:
    void send(const char *eth_name, const char *dip, uint16_t dport, int32_t payload);

private:
    bool init(const char *eth_name);
    bool set_filter(const char *eth_name, uint16_t port);
    bool set_tx_buffer();
    int32_t change_hdr(char *str, uint16_t dport, int32_t payload);

private: // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd             pd_; // protect domain.
    ef_pd_flags pd_flags_ = EF_PD_DEFAULT;

    ef_vi    vi_;
    uint32_t vi_flags_ = EF_VI_FLAGS_DEFAULT | EF_VI_TX_TIMESTAMPS;

private:
    // tx
    void     *tx_buf_ = nullptr;
    ef_memreg tx_memreg_;
    ef_addr   tx_dma_buffer_;
};
