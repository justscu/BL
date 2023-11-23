#include <arpa/inet.h>
#include <assert.h>
#include "commx/fmt/format.h"
#include "commx/fmt/color.h"
#include "commx/utils_cpu.h"
#include "commx/utils_socket.h"
#include "commx/utils_times.h"
#include "commx/utils_net_hardware.h"
#include "commx/utils_net_packet_func.h"
#include "udp_pg_efvi.h"


bool EvfiUdpRecv::init(const char *eth_name) {
    int32_t rc = ef_driver_open(&driver_hdl_);
    fmt::print("ef_driver_open[{}] \n", driver_hdl_);
    if (rc != 0) {
        fmt::print("ef_driver_open failed. {}. \n", strerror(errno));
        return false;
    }

    rc = ef_pd_alloc_by_name(&pd_, driver_hdl_, eth_name, pd_flags_);
    if (rc != 0) {
        fmt::print("ef_pd_alloc_by_name[{}] failed. {}. \n", eth_name,  strerror(errno));
        return false;
    }

    rc = ef_vi_alloc_from_pd(&vi_, driver_hdl_, &pd_, driver_hdl_, -1, -1, -1, nullptr, -1, (enum ef_vi_flags)vi_flags_);
    if (rc < 0) {
        fmt::print("ef_vi_alloc_from_pd failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

bool EvfiUdpRecv::set_filter(const char *eth_name, uint16_t port) {
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
        nic.get_ip(eth_name, ip);

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
        if (!nic.get_mac(eth_name, mac)) {
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

bool EvfiUdpRecv::set_rx_buffer() {
    int32_t rx_cap = ef_vi_receive_capacity(&vi_);
    if (rx_cap < rx_cnt_) {
        rx_cnt_ = rx_cap;
    }
    fmt::print("ef_vi_receive_capacity [{}], rx_cnt_[{}]. \n", rx_cap, rx_cnt_);

    int32_t rc = posix_memalign(&rx_buf_, 4096, 2048*rx_cnt_);
    if (0 != rc) {
        fmt::print("posix_memalign failed: {}. \n", strerror(errno));
        return false;
    }

    rc = ef_memreg_alloc(&rx_memreg_, driver_hdl_, &pd_, driver_hdl_, rx_buf_, 2048*rx_cnt_);
    if (0 != rc) {
        fmt::print("ef_memreg_alloc failed: {}. \n", strerror(errno));
        return false;
    }

    //
    memset(&rx_dma_buffer_, 0, sizeof(rx_dma_buffer_));
    for (int32_t i = 0; i < rx_cnt_; ++i) {
        rx_dma_buffer_[i] = ef_memreg_dma_addr(&rx_memreg_, i*2048);
    }

    return true;
}

void EvfiUdpRecv::recv(const char *eth_name, uint16_t port) {
    fmt::print("bind_thread_to_cpu(3). \n");
    bind_thread_to_cpu(3);

    if (!init(eth_name) || !set_rx_buffer()) { return; }

    const int32_t prelen = ef_vi_receive_prefix_len(&vi_);
    fmt::print("ef_vi_receive_capacity  : {}. \n", ef_vi_receive_capacity(&vi_));
    fmt::print("ef_vi_transmit_capacity : {}. \n", ef_vi_transmit_capacity(&vi_));
    fmt::print("ef_vi_receive_prefix_len: {}. \n", prelen);

    //
    for (int32_t i = 0; i < rx_cnt_; ++i) {
        ef_vi_receive_init(&vi_, rx_dma_buffer_[i], i);
    }
    ef_vi_receive_push(&vi_);

    if (!set_filter(eth_name, port)) { return; }

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
                    char *src = (char*)rx_buf_ + id*2048 + prelen;
                    {
                        MakeUdpPkt udp;
                        uint64_t now = UtilsClock::get_ns();
                        uint64_t pre = *(uint64_t*)(src+udp.udp_payload_offset());
                        uint32_t cost = now - pre;
                        avg_cost   += cost;
                        avg_cnt    += 1;
                        fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                                "                 round-trip time: {} ns. pkt_num: {}. avg: {}. \n",
                                cost, 1, avg_cost/avg_cnt);
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
                    assert(it->value >= 0 && it->value < rx_cnt_);
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





bool EfviUdpSend::init(const char *eth_name) {
    int32_t rc = ef_driver_open(&driver_hdl_);
    fmt::print("ef_driver_open[{}] \n", driver_hdl_);
    if (rc != 0) {
        fmt::print("ef_driver_open failed. {}. \n", strerror(errno));
        return false;
    }

    rc = ef_pd_alloc_by_name(&pd_, driver_hdl_, eth_name, pd_flags_);
    if (rc != 0) {
        fmt::print("ef_pd_alloc_by_name[{}] failed. {}. \n", eth_name,  strerror(errno));
        return false;
    }

    rc = ef_vi_alloc_from_pd(&vi_, driver_hdl_, &pd_, driver_hdl_, -1, 0, -1, nullptr, -1, (enum ef_vi_flags)vi_flags_);
    if (rc < 0) {
        fmt::print("ef_vi_alloc_from_pd failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

bool EfviUdpSend::set_filter(const char *eth_name, uint16_t port) {
    ef_filter_spec filter;
    ef_filter_spec_init(&filter, EF_FILTER_FLAG_NONE);

    int32_t rc = 0;
    // filter by ip and port.
    if (1) {
        char ip[32] = {0};
        Nic nic;
        nic.get_ip(eth_name, ip);

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

void EfviUdpSend::send(const char *eth_name, const char *dip, uint16_t dport, int32_t payload) {
    fmt::print("bind_thread_to_cpu(5). \n");
    bind_thread_to_cpu(5);

    // if (!init(eth_name) || !set_filter(eth_name, dport) || !set_tx_buffer()) { return; }
    if (!init(eth_name) || !set_tx_buffer()) { return; }

    uint8_t smac[8];
    char sip[16];
    {
        Nic nic;
        if (!nic.get_mac(eth_name, smac) || !nic.get_ip(eth_name, sip)) {
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
    for (uint32_t i = 1; true; ++i) {
        char *d = (char*)tx_buf_+udp.udp_payload_offset();
        *((uint64_t*)d) = UtilsClock::get_ns();

        int32_t vlen = udp.set_hdr_finish((char*)tx_buf_, payload, i);

        ef_vi_transmit(&vi_, tx_dma_buffer_, vlen, 0);
        int32_t n_ev = ef_eventq_poll(&vi_, evs, sizeof(evs) / sizeof(evs[0]));
        for (int32_t c = 0; c < n_ev; ++c) {
            uint32_t type = EF_EVENT_TYPE(evs[c]);
            fmt::print("ef_eventq_poll send type[{}] len[{}] \n", type, vlen);
            switch (type) {
                case EF_EVENT_TYPE_RX: {

                } break;
                case EF_EVENT_TYPE_TX: {
                    ef_vi_transmit_unbundle(&vi_, &evs[c], ids);
                } break;
            } // switch
        }

        UtilsCycles::sleep(1000*500); // 1 second.
    }
}
