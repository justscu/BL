#include <new>
#include <stdio.h>
#include <stddef.h>
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
            munmap(rx_bufs_, rx_q_capacity * sizeof(PKT_BUF));
        }
        else {
            free(rx_bufs_);
        }
        rx_bufs_ = nullptr;
    }

    if (driver_hdl_ > 0) {
        ef_driver_close(driver_hdl_);
        driver_hdl_ = 0;
    }
}

bool EfviUdpRecv::alloc_rx_buffer() {
    const uint32_t alloc_size = rx_q_capacity * sizeof(PKT_BUF);

    // alloc memory for DMA transfers.
    rx_bufs_ = (PKT_BUF*)mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE,
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
        snprintf(err_, sizeof(err_)-1, "ef_memreg_alloc failed. rc[%d], [%s].", rc, strerror(errno));
        return false;
    }

    for (int32_t i = 0; i < rx_q_capacity; ++i) {
        rx_bufs_[i].state = 0;
        // Return the DMA address for the given offset within a registered memory region.
        rx_bufs_[i].dma_buf_addr  = ef_memreg_dma_addr(&rx_memreg_, i * sizeof(PKT_BUF));
        rx_bufs_[i].dma_buf_addr += offsetof(struct PKT_BUF, user_buf);
    }

    // Initialize an RX descriptor on the RX descriptor ring
    for (int32_t i = 0; i < rx_q_capacity; ++i) {
        ef_vi_receive_init(&vi_, rx_bufs_[i].dma_buf_addr, i);
    }
    // Submit newly initialized RX descriptors to the NIC
    ef_vi_receive_push(&vi_);

    return true;
}

// local ip & local port
bool EfviUdpRecv::add_filter(const char *ip, uint16_t port) {
    ef_filter_spec flt;

    // EF_FILTER_FLAG_MCAST_LOOP_RECEIVE: 接收机器内组播
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
    if (1) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        rc = inet_pton(AF_INET, ip, &(addr.sin_addr));
        if (rc <= 0) {
            snprintf(err_, sizeof(err_)-1, "inet_pton[%s] failed: rc[%d] [%s].", ip, rc, strerror(errno));
            return false;
        }

        rc = ef_filter_spec_set_ip4_local(&flt, IPPROTO_UDP, addr.sin_addr.s_addr, addr.sin_port);
        if (rc != 0) {
            snprintf(err_, sizeof(err_)-1, "ef_filter_spec_set_ip4_local[%s:%u] failed: rc[%d], [%s].", ip, port, rc, strerror(errno));
            return false;
        }
    }

    rc = ef_vi_filter_add(&vi_, driver_hdl_, &flt, nullptr);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_filter_add[%s:%u] failed: rc[%d], [%s].", ip, port, rc, strerror(errno));
        return false;
    }

    return true;
}

void EfviUdpRecv::recv(RecvCBFunc cb) {
    const int32_t prelen = ef_vi_receive_prefix_len(&vi_);

    // recv
    ef_event      evs[16];
    ef_request_id ids[16];

    while (true) {
        const int32_t nev = ef_eventq_poll(&vi_, evs, sizeof(evs)/sizeof(evs[0]));

        for (int32_t i = 0; i < nev; ++i) {
            const uint32_t type = EF_EVENT_TYPE(evs[i]);
            const uint32_t   id = EF_EVENT_RX_RQ_ID(evs[i]);


            // debug info.
            // fprintf(stdout, "ef_eventq_poll: type[%u] id[%u] len[%u]. \n", type, id, len);

            //
            switch (type) {
                // for X2
                case EF_EVENT_TYPE_RX:
                case EF_EVENT_TYPE_RX_DISCARD:
                {
                    assert(id < rx_q_capacity);
                    if (id < rx_q_capacity) {
                        const int32_t len = EF_EVENT_RX_BYTES(evs[i]);
                        const char  *data = rx_bufs_[id].user_buf + prelen; // 包含mac头的数据.
                        cb(data, len - prelen);
                        ef_vi_receive_post(&vi_, rx_bufs_[id].dma_buf_addr, id);
                    }
                }
                break;

                // for X3
                case EF_EVENT_TYPE_RX_REF: {
                    const int32_t pkt_id = evs[i].rx_ref.pkt_id; // evs[i].rx_ref_discard.pkt_id;
                    const char *frame = (const char *)efct_vi_rxpkt_get(&vi_, pkt_id);
                    int32_t frame_length = evs[i].rx_ref.len; // evs[i].rx_ref_discard.len;
                    cb(frame + prelen, frame_length - prelen);
                    efct_vi_rxpkt_release(&vi_, pkt_id);
                } break;
                case EF_EVENT_TYPE_RX_REF_DISCARD: {
                    // fprintf(stdout, "EF_EVENT_TYPE_RX_REF_DISCARD \n");
                    const int32_t pkt_id = evs[i].rx_ref.pkt_id; // evs[i].rx_ref_discard.pkt_id;
                    efct_vi_rxpkt_get(&vi_, pkt_id);
                    efct_vi_rxpkt_release(&vi_, pkt_id);
                } break;

                default: { } break;
            } // switch.
        }
    }
}



// Return a string that identifies the version of ef_vi
const char* EfviUdpSend::efvi_version() {
    snprintf(err_, sizeof(err_)-1, "efvi_version: %s", ef_vi_version_str());
    return err();
}

// Returns the current version of the drivers that are running
const char* EfviUdpSend::efvi_driver_interface() {
    snprintf(err_, sizeof(err_)-1, "drivers version: %s", ef_vi_driver_interface_str());
    return err();
}

const char* EfviUdpSend::efvi_nic_arch() {
    switch (vi_.nic_type.arch) {
        case EF_VI_ARCH_FALCON: {
            return "EF_VI_ARCH_FALCON";
        } break;
        // X2
        case EF_VI_ARCH_EF10: {
            return "EF_VI_ARCH_EF10";
        } break;
        case EF_VI_ARCH_EF100: {
            return "EF_VI_ARCH_EF100";
        } break;
        // X3
        case EF_VI_ARCH_EFCT: {
            return "EF_VI_ARCH_EFCT";
        } break;
        case EF_VI_ARCH_AF_XDP: {
            return "EF_VI_ARCH_AF_XDP";
        } break;
        default: {
            return "UnKnown";
        } break;
    }
}

const char* EfviUdpSend::efvi_support_ctpio(const char *interface) {
     const int32_t idx = if_nametoindex(interface);
     if (idx == 0) {
         snprintf(err_, sizeof(err_)-1, "if_nametoindex[%s] failed: [%s]", interface, strerror(errno));
         return err();
     }

    uint64_t value = 0;
    int32_t rc = ef_vi_capabilities_get(driver_hdl_, idx, EF_VI_CAP_CTPIO, &value);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_capabilities_get(EF_VI_CAP_CTPIO) failed: rc[%d] [%s]", rc, strerror(errno));
    }
    else {
        snprintf(err_, sizeof(err_)-1, "EF_VI_CAP_CTPIO: %u", value);
    }

    return err();
}

bool EfviUdpSend::init(const char *interface) {
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
    const uint32_t vi_flags = EF_VI_FLAGS_DEFAULT | EF_VI_TX_CTPIO;
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

void EfviUdpSend::uninit() {
    if (tx_bufs_) {
        free(tx_bufs_);
        tx_bufs_ = nullptr;
    }

    if (driver_hdl_ > 0) {
        ef_driver_close(driver_hdl_);
        driver_hdl_ = 0;
    }
}

bool EfviUdpSend::alloc_tx_buffer() {
    const uint32_t alloc_size = sizeof(PKT_BUF) * tx_q_capacity;

    // if mmap failed, use posix_memalign.
    if (0 != posix_memalign((void**)&tx_bufs_, 4*1024, alloc_size)) {
        snprintf(err_, sizeof(err_)-1, "posix_memalign failed. size[%d] [%s].", alloc_size, strerror(errno));
        return false;
    }

    // Register memory for use with ef_vi.
    int32_t rc = ef_memreg_alloc(&tx_memreg_, driver_hdl_, &pd_, driver_hdl_, (void*)tx_bufs_, alloc_size);
    if (0 != rc) {
        snprintf(err_, sizeof(err_)-1, "ef_memreg_alloc failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    for (uint32_t i = 0; i < tx_q_capacity; ++i) {
        tx_bufs_[i].state = 0;
        // Return the DMA address for the given offset within a registered memory region.
        tx_bufs_[i].dma_buf_addr  = ef_memreg_dma_addr(&tx_memreg_, i * sizeof(PKT_BUF));
        tx_bufs_[i].dma_buf_addr += offsetof(struct PKT_BUF, user_buf);
    }

    return true;
}

// local ip & local port
bool EfviUdpSend::add_filter(const char *ip, uint16_t port) {
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

bool EfviUdpSend::get_usable_send_buf(PKT_BUF * &buf, uint32_t &dma_id) {
    for (uint32_t i = 0; i < tx_q_capacity; ++i) {
        if (tx_bufs_[i].state == 0) {
            tx_bufs_[i].state = 1;
            buf = &(tx_bufs_[i]);
            dma_id = i;
            return true;
        }
    }

    return false;
}

bool EfviUdpSend::dma_send(int32_t pkt_len, uint32_t dma_id) {
    assert(dma_id < tx_q_capacity);

    // ef_vi_transmit_init, ef_vi_transmit_push
    // Transmit a packet from a single packet buffer.
    // send data now
    int32_t rc = ef_vi_transmit(&vi_, tx_bufs_[dma_id].dma_buf_addr, pkt_len, dma_id);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_transmit failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    return true;
}

bool EfviUdpSend::ctpio_send(int32_t pkt_len, uint32_t dma_id) {
    // ctpio传入的地址，是用户空间的地址，不是DMA的地址.
    ef_vi_transmit_ctpio(&vi_, tx_bufs_[dma_id].user_buf, pkt_len, 14);

    // 直接用 ctpio发送，可能会遇到失败的情况. 此时，需要用回退函数(ef_vi_transmit_ctpio_fallback)，从DMA发送.
    // 所以仍然需要DMA的地址，该地址和ctpio中用户空间的地址对应
    int32_t rc = ef_vi_transmit_ctpio_fallback(&vi_, tx_bufs_[dma_id].dma_buf_addr, pkt_len, dma_id);
    if (rc != 0) {
        snprintf(err_, sizeof(err_)-1, "ef_vi_transmit_ctpio failed: rc[%d] [%s].", rc, strerror(errno));
        return false;
    }

    return true;
}

void EfviUdpSend::poll() {
    int32_t n_ev = ef_eventq_poll(&vi_, evs_, sizeof(evs_) / sizeof(evs_[0]));
    for (int32_t c = 0; c < n_ev; ++c) {
        const uint32_t type = EF_EVENT_TYPE(evs_[c]);
        switch (type) {
            case EF_EVENT_TYPE_TX:
            case EF_EVENT_TYPE_TX_ERROR: {
                int32_t cnt = ef_vi_transmit_unbundle(&vi_, &evs_[c], ids_);
                for (int32_t i = 0; i < cnt; ++i) {
                    assert(ids_[i] < tx_q_capacity);
                    tx_bufs_[ids_[i]].state = 0; // set state usable.
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
