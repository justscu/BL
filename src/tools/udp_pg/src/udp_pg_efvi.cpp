#include <arpa/inet.h>
#include <assert.h>
#include <sys/mman.h>
#include "fmt/format.h"
#include "fmt/color.h"
#include "commx/utils.h"
#include "udp_pg_efvi.h"


// interface: nic name.
bool EfviUdpRecv::init(const char *interface) {
    // print version info
    fmt::print("ef_vi_version_str[{}] \n", ef_vi_version_str());
    fmt::print("ef_vi_driver_interface_str[{}] \n", ef_vi_driver_interface_str());

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

    return true;
}

bool EfviUdpRecv::alloc_rx_buffer() {
    // each size 2K.
    const uint32_t pkt_buf_size = 2*1024;
    const uint32_t alloc_size = rx_q_capacity * pkt_buf_size;
    // alloc memory for DMA transfers.
    rx_bufs_ = mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE,
                            MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                            -1, 0);
    if (rx_bufs_ == MAP_FAILED) {
        snprintf(err_, sizeof(err_)-1, "mmap failed. size[%d] err[%s].", alloc_size, strerror(errno));
        // if mmap failed, use posix_memalign.
        if (0 != posix_memalign((void**)&rx_bufs_, 4*1024*1024, alloc_size)) {
            snprintf(err_, sizeof(err_)-1, "posix_memalign failed. size[%d] err[%s].", alloc_size, strerror(errno));
            return false;
        }
    }

    // Register a memory region for use with ef_vi.
    int32_t rc = ef_memreg_alloc(&rx_memreg_, driver_hdl_, &pd_, driver_hdl_, rx_bufs_, alloc_size);
    if (0 != rc) {
        snprintf(err_, sizeof(err_)-1, "ef_memreg_alloc failed. err[%s].", strerror(errno));
        return false;
    }

    //
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

    return true;
}

bool EfviUdpRecv::set_filter(const char *interface, uint16_t port) {
    ef_filter_spec filter;
    ef_filter_spec_init(&filter, EF_FILTER_FLAG_NONE);

    int32_t rc = 0;
    // set promiscuous mode
    if (0) {
        rc = ef_filter_spec_set_port_sniff(&filter, 1);
        if (rc != 0) {
            fmt::print("ef_filter_spec_set_port_sniff failed: {}. \n", strerror(errno));
            return false;
        }
    }

    // filter by ip and port.
    if (1) {
        char ip[32] = {0};
        Nic nic;
        nic.get_ip(interface, ip);

        in_addr addr;
        if (0 == inet_aton(ip, &addr)) {
            fmt::print("inet_aton failed: {}. \n", strerror(errno));
            return false;
        }

        rc = ef_filter_spec_set_ip4_local(&filter, IPPROTO_UDP, addr.s_addr, htons(port));
        if (rc != 0) {
            fmt::print("ef_filter_spec_set_ip4_local failed: {}. \n", strerror(errno));
            return false;
        }
        fmt::print("ef_filter_spec_set_ip4_local success, rx {}:{} \n", ip, port);
    }

    // filter by mac. !!!
    if (0) {
        uint8_t mac[16] = {0};
        Nic nic;
        if (!nic.get_mac(interface, mac)) {
            fmt::print("get_mac failed: {}. \n", nic.err());
            return false;
        }
        rc = ef_filter_spec_set_eth_local(&filter, EF_FILTER_VLAN_ID_ANY, mac);
        if (rc != 0) {
            fmt::print("ef_filter_spec_set_eth_local failed: {}. \n", strerror(errno));
            return false;
        }
    }

    rc = ef_vi_filter_add(&vi_, driver_hdl_, &filter, nullptr);
    if (rc != 0) {
        fmt::print("ef_vi_filter_add failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}



void EfviUdpRecv::recv(const char *interface, uint16_t port) {
    fmt::print("bind_thread_to_cpu(3). \n");
    bind_thread_to_cpu(3);

    if (!init(interface) || !alloc_rx_buffer()) {
        fmt::print("{} \n", err());
        return;
    }
    fmt::print("init ok. \n");

    const int32_t prelen = ef_vi_receive_prefix_len(&vi_);
    fmt::print("ef_vi_receive_capacity  : {}. \n", ef_vi_receive_capacity(&vi_));
    fmt::print("ef_vi_transmit_capacity : {}. \n", ef_vi_transmit_capacity(&vi_));
    fmt::print("ef_vi_receive_prefix_len: {}. \n", prelen);

    // Initialize an RX descriptor on the RX descriptor ring
    for (int32_t i = 0; i < rx_q_capacity; ++i) {
        ef_vi_receive_init(&vi_, rx_dma_buffer_[i], i);
    }
    // Submit newly initialized RX descriptors to the NIC
    ef_vi_receive_push(&vi_);

    if (!set_filter(interface, port)) { return; }

    MakeUdpPkt udp;

    uint64_t avg_cost = 0;
    uint64_t avg_cnt  = 0;

    FreeIDs free_ids;
    // recv
    ef_event      evs[EF_VI_TRANSMIT_BATCH];
    ef_request_id ids[EF_VI_TRANSMIT_BATCH];
    while (true) {
        const int32_t nev = ef_eventq_poll(&vi_, evs, sizeof(evs)/sizeof(evs[0]));
        for (int32_t i = 0; i < nev; ++i) {
            uint32_t type = EF_EVENT_TYPE(evs[i]);
            uint32_t   id = EF_EVENT_RX_RQ_ID(evs[i]);
            uint32_t  len = EF_EVENT_RX_BYTES(evs[i]);
            // fmt::print("rx new event: type[{}] id[{}] len[{}]. \n", type, id, len);
            switch (type) {
                case EF_EVENT_TYPE_RX_MULTI:
                case EF_EVENT_TYPE_RX_NO_DESC_TRUNC:
                case EF_EVENT_TYPE_RX: {
                    char *src = (char*)rx_bufs_ + id*2048 + prelen;
                    {
                        struct timespec now;
                        clock_gettime(CLOCK_REALTIME, &now);

                        uint16_t  idx = *(uint16_t*)(src+udp.udp_payload_offset());
                        timespec *pre =  (timespec*)(src+udp.udp_payload_offset()+8);

                        int64_t cost = (now.tv_sec - pre->tv_sec) * 1000000000 + (now.tv_nsec - pre->tv_nsec);
                        avg_cost += cost;
                        avg_cnt  += 1;

                        fmt::print("                 send {} {} \n", pre->tv_sec, pre->tv_nsec);
                        fmt::print("                 recv {} {} \n", now.tv_sec,  now.tv_nsec);

                        char tm[32] = {0};
                        UtilsTimefmt::get_now2(tm);
                        fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                                "                 {}: {} RTT/2 time: {} ns. pkt_num: {}. avg: {}. \n",
                                tm, idx, cost, 1, avg_cost/avg_cnt);
                    }

                    free_ids.add(id);
                } break;

                case EF_EVENT_TYPE_TX:
                case EF_EVENT_TYPE_TX_WITH_TIMESTAMP: {
                    ef_vi_transmit_unbundle(&vi_, &evs[i], ids);
                } break;

            } // switch
        } // for

        //
        if (free_ids.used() == free_ids.capacity()) {
            for (int32_t k = 0; k < free_ids.used(); ++k) {
                for (FreeIDs::ID *it = free_ids.begin(); it != free_ids.end(); ++it) {
                    assert(it->value >= 0 && it->value < rx_q_capacity);
                    int32_t rc = ef_vi_receive_init(&vi_, rx_dma_buffer_[it->value], it->value);
                    if (rc != 0) {
                        fmt::print("ef_vi_receive_init failed: rx {}. \n", strerror(errno));
                    }
                }
                ef_vi_receive_push(&vi_);
                free_ids.clear();
                fmt::print("ef_vi_receive_push \n");
            }
        }

    }
}


bool EfviUdpSend::init(const char *interface) {
    // print version info
    fmt::print("ef_vi_version_str[{}] \n", ef_vi_version_str());
    fmt::print("ef_vi_driver_interface_str[{}] \n", ef_vi_driver_interface_str());

    int32_t rc = ef_driver_open(&driver_hdl_);
    fmt::print("ef_driver_open[{}] \n", driver_hdl_);
    if (rc != 0) {
        fmt::print("ef_driver_open failed. {}. \n", strerror(errno));
        return false;
    }

    rc = ef_pd_alloc_by_name(&pd_, driver_hdl_, interface, pd_flags_);
    if (rc != 0) {
        fmt::print("ef_pd_alloc_by_name[{}] failed. {}. \n", interface,  strerror(errno));
        return false;
    }

    // | EF_VI_TX_TIMESTAMPS
    uint32_t vi_flags = EF_VI_FLAGS_DEFAULT;
    rc = ef_vi_alloc_from_pd(&vi_, driver_hdl_, &pd_, driver_hdl_,
                                -1, // evq_capacity
                                 0, // rxq_capacity
                                -1, // txq_capacity
                           nullptr, // evq_opt, event queue to use if evq_capacity=0.
                                -1, // evq_dh, The ef_driver_handle of the evq_opt event queue.
                                (enum ef_vi_flags)vi_flags);
    if (rc < 0) {
        fmt::print("ef_vi_alloc_from_pd failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

bool EfviUdpSend::set_tx_buffer() {
    int32_t tx_cap = ef_vi_transmit_capacity(&vi_);
    fmt::print("ef_vi_transmit_capacity tx_cap[{}]. \n", tx_cap);

    int32_t rc = posix_memalign(&tx_buf_, 4096, 2048);
    if (0 != rc) {
        fmt::print("posix_memalign failed: {}. \n", strerror(errno));
        return false;
    }

    rc = ef_memreg_alloc(&tx_memreg_, driver_hdl_, &pd_, driver_hdl_, tx_buf_, 2048);
    if (0 != rc) {
        fmt::print("ef_memreg_alloc failed: {}. \n", strerror(errno));
        return false;
    }

    //
    tx_dma_buffer_ = ef_memreg_dma_addr(&tx_memreg_, 0);

    return true;
}

bool EfviUdpSend::set_filter(const char *interface, uint16_t port) {
    ef_filter_spec filter;
    ef_filter_spec_init(&filter, EF_FILTER_FLAG_NONE);

    int32_t rc = 0;
    // filter by ip and port.
    if (1) {
        char ip[32] = {0};
        Nic nic;
        nic.get_ip(interface, ip);

        in_addr addr;
        if (0 == inet_aton(ip, &addr)) {
            fmt::print("inet_aton failed: {}. \n", strerror(errno));
            return false;
        }

        rc = ef_filter_spec_set_ip4_local(&filter, IPPROTO_UDP, addr.s_addr, htons(port));
        if (rc != 0) {
            fmt::print("ef_filter_spec_set_ip4_local failed: {}. \n", strerror(errno));
            return false;
        }
    }

    rc = ef_vi_filter_add(&vi_, driver_hdl_, &filter, nullptr);
    if (rc != 0) {
        fmt::print("ef_vi_filter_add failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

int32_t EfviUdpSend::change_hdr(char *str, uint16_t dport, int32_t payload) {
    mac_hdr *mac = (mac_hdr*)str;
    {
        char smac[8];
        memcpy(smac, mac->dstaddr, 6);
        memcpy(mac->dstaddr, mac->srcaddr, 6);
        memcpy(mac->srcaddr, smac, 6);
    }

    ip_hdr *ip = (ip_hdr*)(str+sizeof(mac_hdr));
    {
        uint32_t sip = ip->dst_ip;
        ip->dst_ip = ip->src_ip;
        ip->src_ip = sip;
    }

    udp_hdr *udp = (udp_hdr*)(str+sizeof(mac_hdr)+sizeof(ip_hdr));
    {
        udp->src_port = udp->dst_port;
        udp->dst_port = ntohs(dport);
    }

    MakeUdpPkt v;
    return v.set_hdr_finish(str, payload, ntohs(ip->id));
}

void EfviUdpSend::send(const char *interface, const char *dip, uint16_t dport, int32_t payload) {
    fmt::print("bind_thread_to_cpu(5). \n");
    bind_thread_to_cpu(5);

    if (!init(interface) || !set_tx_buffer()) { return; }

    uint8_t smac[8];
    char sip[16];
    {
        Nic nic;
        if (!nic.get_mac(interface, smac) || !nic.get_ip(interface, sip)) {
            fmt::print("{}. \n", nic.err());
            return;
        }
    }

    UtilsCycles::init();
    //
    MakeUdpPkt udp;
    udp.init_mac_hdr((char*)tx_buf_, (const char*)smac);
    udp.init_ip_hdr_partial((char*)tx_buf_, sip, dip);
    udp.init_udp_hdr_partial((char*)tx_buf_, 1127, dport);

    ef_request_id ids[EF_VI_TRANSMIT_BATCH];
    ef_event      evs[EF_VI_TRANSMIT_BATCH];
    for (uint64_t i = 1; true; ++i) {
        UtilsCycles::sleep(1000*500); // 500ms

        char *d = (char*)tx_buf_+udp.udp_payload_offset();
        *((uint16_t*)(d)) = i;
        struct timespec *ts = (timespec*)(d + 8);
        clock_gettime(CLOCK_REALTIME, ts);

        int32_t vlen = udp.set_hdr_finish((char*)tx_buf_, payload, i);
        // send data now.
        ef_vi_transmit(&vi_, tx_dma_buffer_, vlen, 0);

        char tm[32] = {0};
        UtilsTimefmt::get_now2(tm);
        fmt::print("{}: {} ef_vi_transmit send len[{}] \n", tm, i, vlen);

        int32_t n_ev = ef_eventq_poll(&vi_, evs, sizeof(evs) / sizeof(evs[0]));
        for (int32_t c = 0; c < n_ev; ++c) {
            const uint32_t type = EF_EVENT_TYPE(evs[c]);
            switch (type) {
                case EF_EVENT_TYPE_RX: {

                } break;
                case EF_EVENT_TYPE_TX:
                case EF_EVENT_TYPE_TX_ERROR: {
                    ef_vi_transmit_unbundle(&vi_, &evs[c], ids);
                } break;
            } // switch
        }
    }
}
