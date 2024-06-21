#include <new>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include "udp_pg_efvi.h"


bool FreeRxIDs::add(int32_t v) {
    if (used_ < capacity()) {
        ids_[used_++].value = v;
        return true;
    }
    return false;
}


// Return a string that identifies the version of ef_vi
const char* EfviUdpRecv::efvi_version() {
    snprintf(err_, sizeof(err_)-1, "efvi_version: %s", ef_vi_version_str());
    return err();
}

// Returns the current version of the drivers that are running
const char* EfviUdpRecv::efvi_driver_interface() {
    snprintf(err_, sizeof(err_)-1, "drivers version: %s", ef_vi_driver_interface_str());
    return err();
}

// interface: nic name.
bool EfviUdpRecv::init(const char *interface) {
    // open driver
    int32_t rc = ef_driver_open(&driver_hdl_);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_driver_open failed. rc[%d], [%s].", rc, strerror(errno));
        return false;
    }

    // allocate protection domain
    const ef_pd_flags pd_flags = EF_PD_DEFAULT;
    rc = ef_pd_alloc_by_name(&pd_, driver_hdl_, interface, pd_flags);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_pd_alloc_by_name failed. rc[%d], [%s].", rc, strerror(errno));
        return false;
    }

    // if support timestamping,  vi_flags |= EF_VI_RX_TIMESTAMPS
    // if support rx_merge    ,  vi_flags |= EF_VI_RX_EVENT_MERGE
    const uint32_t vi_flags_ = EF_VI_FLAGS_DEFAULT;
    // Allocate a virtual interface from protection domain.
    rc = ef_vi_alloc_from_pd(&vi_, driver_hdl_, &pd_, driver_hdl_,
                                -1, // evq_capacity
                     rx_q_capacity, // rxq_capacity
                                 0, // txq_capacity
                           nullptr, // event queue to use if evq_capacity=0
                                -1, // The ef_driver_handle of the evq_opt event queue.
                                (enum ef_vi_flags)vi_flags_);
    if (rc < 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_alloc_from_pd failed. rc[%d], [%s].", rc, strerror(errno));
        return false;
    }

    return alloc_rx_buffer();
}

void EfviUdpRecv::uninit() {
    if (rx_bufs_) {
        if (mmap_flag_) {
            mmap_flag_ = false;
            munmap(rx_bufs_, rx_q_capacity * pkt_buf_size);
        }
        else {
            free(rx_bufs_);
        }
        rx_bufs_ = nullptr;
    }

    if (rx_dma_buffer_) {
        delete [] rx_dma_buffer_;
        rx_dma_buffer_ = nullptr;
    }

    if (driver_hdl_ > 0) {
        ef_driver_close(driver_hdl_);
        driver_hdl_ = 0;
    }
}

bool EfviUdpRecv::alloc_rx_buffer() {
    const uint32_t alloc_size = rx_q_capacity * pkt_buf_size;

    // alloc memory for DMA transfers.
    rx_bufs_ = (char*)mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                                -1, 0);

    mmap_flag_ = (rx_bufs_ != MAP_FAILED);// if mmap success, set true.

    if (rx_bufs_ == MAP_FAILED) {
        snprintf(err_, sizeof(err_)-1, "mmap failed. size[%d] err[%s].", alloc_size, strerror(errno));
        // if mmap failed, use posix_memalign.
        if (0 != posix_memalign((void**)&rx_bufs_, 4*1024*1024, alloc_size)) {
            snprintf(err_, sizeof(err_)-1, "posix_memalign failed. size[%d] [%s].", alloc_size, strerror(errno));
            return false;
        }
    }

    // Register a memory region for use with ef_vi.
    int32_t rc = ef_memreg_alloc(&rx_memreg_, driver_hdl_, &pd_, driver_hdl_, rx_bufs_, alloc_size);
    if (0 != rc) {
        snprintf(err_, sizeof(err_)-1, "ef_memreg_alloc failed. rc[%d], [%s].", strerror(errno));
        return false;
    }

    // store DMA address.
    rx_dma_buffer_ = new (std::nothrow) ef_addr[rx_q_capacity];
    if (!rx_dma_buffer_) {
        snprintf(err_, sizeof(err_)-1, "new ef_addr failed. size[%d].", rx_q_capacity);
        return false;
    }
    memset(rx_dma_buffer_, 0, sizeof(ef_addr) * rx_q_capacity);
    for (int32_t i = 0; i < rx_q_capacity; ++i) {
        // Return the DMA address for the given offset within a registered memory region.
        rx_dma_buffer_[i] = ef_memreg_dma_addr(&rx_memreg_, i*pkt_buf_size);
    }

    // Initialize an RX descriptor on the RX descriptor ring
    for (int32_t i = 0; i < rx_q_capacity; ++i) {
        ef_vi_receive_init(&vi_, rx_dma_buffer_[i], i);
    }
    // Submit newly initialized RX descriptors to the NIC
    ef_vi_receive_push(&vi_);

    return true;
}

// local ip & local port
bool EfviUdpRecv::add_filter(const char *ip, uint16_t port) {
    ef_filter_spec flt;
    ef_filter_spec_init(&flt, EF_FILTER_FLAG_NONE);

    int32_t rc = 0;

    // set promiscuous mode
    if (0) {
        rc = ef_filter_spec_set_port_sniff(&flt, 1);
        if (rc != 0) {
            snprintf(err_, sizeof(err_)-1, "ef_filter_spec_set_port_sniff failed: rc[%d], [%s]. ", rc, strerror(errno));
            return false;
        }
    }

    // filter multicast, 慎用  !!! 推荐只有在镜像时使用
    if (0) {
        // Set a Multicast All filter on the filter specification
        rc = ef_filter_spec_set_multicast_all(&flt);
        if (rc != 0) {
            snprintf(err_, sizeof(err_)-1, "ef_filter_spec_set_multicast_all failed: rc[%d], [%s].", rc, strerror(errno));
            return false;
        }
    }

    // filter by ip and port.
    if (0) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        rc = inet_pton(AF_INET, ip, &(addr.sin_addr));
        if (rc <= 0) {
            snprintf(err_, sizeof(err_)-1, "inet_pton failed: rc[%d], ip[%s] [%s].", rc, ip, strerror(errno));
            return false;
        }

        rc = ef_filter_spec_set_ip4_local(&flt, IPPROTO_UDP, addr.sin_addr.s_addr, addr.sin_port);
        if (rc != 0) {
            snprintf(err_, sizeof(err_)-1, "ef_filter_spec_set_ip4_local failed: rc[%d], [%s].", rc, strerror(errno));
            return false;
        }
    }

    rc = ef_vi_filter_add(&vi_, driver_hdl_, &flt, nullptr);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_filter_add failed: rc[%d], [%s].", rc, strerror(errno));
        return false;
    }

    return true;
}

void EfviUdpRecv::recv(RecvCBFunc cb) {
    const int32_t prelen = ef_vi_receive_prefix_len(&vi_);

    FreeRxIDs free_ids;
    // recv
    ef_event      evs[16];
    ef_request_id ids[16];

    while (true) {
        const int32_t nev = ef_eventq_poll(&vi_, evs, sizeof(evs)/sizeof(evs[0]));

        for (int32_t i = 0; i < nev; ++i) {
            const uint32_t type = EF_EVENT_TYPE(evs[i]);
            const uint32_t   id = EF_EVENT_RX_RQ_ID(evs[i]);
            const int32_t   len = EF_EVENT_RX_BYTES(evs[i]);

            // debug info.
            // fprintf(stdout, "ef_eventq_poll: type[%u] id[%u] len[%u]. \n", type, id, len);

            if (type == EF_EVENT_TYPE_RX) {
                // 包含mac头的数据.
                const char *data = (char*)rx_bufs_ + id *2048 + prelen;
                cb(data, len-prelen);
            }
            free_ids.add(id);
        }

        // refill rx ring.
        if (free_ids.used() >= free_ids.capacity()/2) {
            for (FreeRxIDs::ID *it = free_ids.begin(); it != free_ids.end(); ++it) {
                assert(it->value >= 0 && it->value < rx_q_capacity);
                const int32_t rc = ef_vi_receive_init(&vi_, rx_dma_buffer_[it->value], it->value);
                if (rc != 0) {
                    snprintf(err_, sizeof(err_)-1, "EfviUdpRecv::recv: ef_vi_receive_init failed: rc[%d], [%s].", rc, strerror(errno));
                }
            } // for.
            ef_vi_receive_push(&vi_);
            free_ids.clear();
        }
    }
}



FILO::FILO(uint32_t cap) : capacity(cap) { }

bool FILO::init() {
    uninit();
    ids_ = new (std::nothrow) ID[capacity];
    used_ = 0;
    return (ids_ != nullptr);
}

void FILO::uninit() {
    if (ids_) {
        delete [] ids_;
        ids_ = nullptr;
    }
    used_ = 0;
}

bool FILO::add(uint32_t v) {
    if (used_ < capacity) {
        ids_[used_].value = v;
        used_++;
        return true;
    }

    return false;
}

bool FILO::get(uint32_t &v) {
    if (used_ > 0) {
        v = ids_[used_-1].value;
        used_ -= 1;
        return true;
    }

    return false;
}

// Return a string that identifies the version of ef_vi
const char* EfviDMAUdpSend::efvi_version() {
    snprintf(err_, sizeof(err_)-1, "efvi_version: %s", ef_vi_version_str());
    return err();
}

// Returns the current version of the drivers that are running
const char* EfviDMAUdpSend::efvi_driver_interface() {
    snprintf(err_, sizeof(err_)-1, "drivers version: %s", ef_vi_driver_interface_str());
    return err();
}

bool EfviDMAUdpSend::init(const char *interface) {
    if (!free_tx_dma_ids_.init()) {
        snprintf(err_, sizeof(err_)-1, "free_tx_dma_ids.init failed.");
        return false;
    }

    // open driver
    int32_t rc = ef_driver_open(&driver_hdl_);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_driver_open failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    // Allocate a protection domain
    const ef_pd_flags pd_flags = EF_PD_DEFAULT;
    rc = ef_pd_alloc_by_name(&pd_, driver_hdl_, interface, pd_flags);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_pd_alloc_by_name failed: [%s] rc[%d] [%s].", interface, rc, strerror(errno));
        return false;
    }

    // if support timestamping,  vi_flags |= EF_VI_TX_TIMESTAMPS
    const uint32_t vi_flags = EF_VI_FLAGS_DEFAULT;
    // Allocate a virtual interface from a protection domain.
    rc = ef_vi_alloc_from_pd(&vi_, driver_hdl_, &pd_, driver_hdl_,
                                -1, // evq_capacity
                                 0, // rxq_capacity
                                -1, // txq_capacity
                           nullptr, // evq_opt, event queue to use if evq_capacity=0.
                                -1, // evq_dh, The ef_driver_handle of the evq_opt event queue.
                                (enum ef_vi_flags)vi_flags);
    if (rc < 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_alloc_from_pd failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    return alloc_tx_buffer();
}

void EfviDMAUdpSend::uninit() {
    if (tx_buf_) {
        free(tx_buf_);
        tx_buf_ = nullptr;
    }

    if (driver_hdl_ > 0) {
        ef_driver_close(driver_hdl_);
        driver_hdl_ = 0;
    }
}

bool EfviDMAUdpSend::alloc_tx_buffer() {
    const uint32_t alloc_size = sizeof(EfviSendDataCell) * tx_q_capacity;

    // if mmap failed, use posix_memalign.
    if (0 != posix_memalign((void**)&tx_buf_, 4*1024, alloc_size)) {
        snprintf(err_, sizeof(err_)-1, "posix_memalign failed. size[%d] [%s].", alloc_size, strerror(errno));
        return false;
    }

    // Register memory for use with ef_vi.
    int32_t rc = ef_memreg_alloc(&tx_memreg_, driver_hdl_, &pd_, driver_hdl_, (void*)tx_buf_, alloc_size);
    if (0 != rc) {
        snprintf(err_, sizeof(err_)-1, "ef_memreg_alloc failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    for (uint32_t i = 0; i < tx_q_capacity; ++i) {
        // Return the DMA address for the given offset within a registered memory region.
        tx_dma_buffer_[i] = ef_memreg_dma_addr(&tx_memreg_, i * sizeof(EfviSendDataCell));
        free_tx_dma_ids_.add(i);
    }

    return true;
}

// local ip & local port
bool EfviDMAUdpSend::add_filter(const char *ip, uint16_t port) {
    ef_filter_spec flt;
    ef_filter_spec_init(&flt, EF_FILTER_FLAG_NONE);

    int32_t rc = 0;
    // filter by ip and port.
    if (1) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        rc = inet_pton(AF_INET, ip, &(addr.sin_addr));
        if (rc <= 0) {
            snprintf(err_, sizeof(err_)-1, "inet_pton failed: rc[%d], ip[%s] [%s].", rc, ip, strerror(errno));
            return false;
        }

        rc = ef_filter_spec_set_ip4_local(&flt, IPPROTO_UDP, addr.sin_addr.s_addr, addr.sin_port);
        if (rc != 0) {
            snprintf(err_, sizeof(err_)-1, "ef_filter_spec_set_ip4_local failed: rc[%d], [%s].", rc, strerror(errno));
            return false;
        }
    }

    // Add a filter to a virtual interface.
    rc = ef_vi_filter_add(&vi_, driver_hdl_, &flt, nullptr);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_filter_add failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    return true;
}

bool EfviDMAUdpSend::get_usable_send_buf(EfviSendDataCell * &addr, uint32_t &dma_id) {
    if (free_tx_dma_ids_.get(dma_id)) {
        addr = &(tx_buf_[dma_id]);
        return true;
    }

    return false;
}

bool EfviDMAUdpSend::send(int32_t pkt_len, uint32_t dma_id) {
    assert(dma_id < tx_q_capacity);

    // ef_vi_transmit_init, ef_vi_transmit_push
    // Transmit a packet from a single packet buffer.
    // send data now
    int32_t rc = ef_vi_transmit(&vi_, tx_dma_buffer_[dma_id], pkt_len, dma_id);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_transmit failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    return true;
}

void EfviDMAUdpSend::poll() {
    int32_t n_ev = ef_eventq_poll(&vi_, evs_, sizeof(evs_) / sizeof(evs_[0]));
    for (int32_t c = 0; c < n_ev; ++c) {
        const uint32_t type = EF_EVENT_TYPE(evs_[c]);
        switch (type) {
            case EF_EVENT_TYPE_TX:
            case EF_EVENT_TYPE_TX_ERROR: {
                int32_t cnt = ef_vi_transmit_unbundle(&vi_, &evs_[c], ids_);
                for (int32_t i = 0; i < cnt; ++i) {
                    assert(ids_[i] < tx_q_capacity);
                    free_tx_dma_ids_.add(ids_[i]);
                }
            } break;
        } // switch
    } // for
}


bool JoinMulticast::create_socket() {
    // 注意： 如果过滤器是 multcast-all, 需要使用 onload_socket_nonaccel
    // 否则会收不到组播数据，因为IGMP消息会被VI拦截，内核协议栈收不到IGMP
    // #include "onload/extensions.h"
    // 库目录: lib/onload_ext/
    // 库: libonload_ext.a, ly
    // This function creates a socket that is not accelerated by Onload
    // fd_ = onload_socket_nonaccel(AF_INET, SOCK_DGRAM, 0);

    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd() == -1) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_transmit failed: [%s].", strerror(errno));
        return false;
    }

    return true;
}

void JoinMulticast::close_socket() {
    if (fd() != 0) {
        close(fd());
        fd_ = 0;
    }
}

bool JoinMulticast::join(const char *interface, const char *group_ip, uint16_t group_port) {
    char local_ip[32] = {0};

    // get ip by interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, interface);
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl(fd(), SIOCGIFADDR, &ifr) < 0) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFADDR] err: %s.", strerror(errno));
        return false;
    }
    struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);
    sprintf(local_ip, "%s", inet_ntoa(sin->sin_addr));

    // bind NIC.
    struct in_addr oaddr = {0};
    oaddr.s_addr = inet_addr(local_ip);
    if (0 != setsockopt(fd(), IPPROTO_IP, IP_MULTICAST_IF, (char*)&oaddr, sizeof(struct in_addr))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_MULTICAST_IF] failed: %s.", strerror(errno));
        return false;
    }

    // join multicast
    struct ip_mreq group_addr;
    memset(&group_addr, 0, sizeof(ip_mreq));
    group_addr.imr_multiaddr.s_addr = inet_addr(group_ip);
    group_addr.imr_interface.s_addr = inet_addr(local_ip);
    if (0 != setsockopt(fd(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group_addr, sizeof(group_addr))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_ADD_MEMBERSHIP] failed: %s.", strerror(errno));
        return false;
    }

    return true;
}
