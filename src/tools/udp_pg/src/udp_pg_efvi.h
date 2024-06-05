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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// recv udp packets by Solorflare-efvi
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class EfviUdpRecv {
public:
    void recv(const char *interface, uint16_t port);

    const char *err() const { return err_; }

private:
    bool init(const char *interface);
    bool alloc_rx_buffer();
    bool set_filter(const char *ip, uint16_t port);

private:
    // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd pd_; // protect domain.
    ef_vi vi_; // virtual interface.

private:
    // recv queue cnt
    static const int32_t rx_q_capacity = 1024*4; // must 2^N.
    // rx
    void *rx_buf_ = nullptr;
    ef_memreg rx_memreg_;
    ef_addr   *rx_dma_buffer_ = nullptr;

private:
    char err_[256] = {0};
};


class EfviUdpSend {
public:
    void send(const char *interface, const char *dip, uint16_t dport, int32_t payload);

private:
    bool init(const char *interface);
    bool set_tx_buffer();
    bool set_filter(const char *interface, uint16_t port);

    int32_t change_hdr(char *str, uint16_t dport, int32_t payload);

private: // ef-vi
    ef_driver_handle driver_hdl_ = 0;
    ef_pd             pd_; // protect domain.
    ef_pd_flags pd_flags_ = EF_PD_DEFAULT;

    ef_vi    vi_;

private:
    // tx
    void     *tx_buf_ = nullptr;
    ef_memreg tx_memreg_;
    ef_addr   tx_dma_buffer_;
};
