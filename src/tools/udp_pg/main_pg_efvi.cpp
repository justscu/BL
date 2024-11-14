#include <csignal>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <unistd.h>
#include <functional>
#include <arpa/inet.h>
#include "udp_raw.h"
#include "udp_pg.h"
#include "udp_pg_efvi.h"
#include "fmt/format.h"
#include "fmt/color.h"
#include "commx/utils.h"


void handle(int32_t s) {
    fmt::print("recv signal: {}. \n", s);
    exit(0);
}

void usage() {
    fmt::print("测量单向，需要 发送端 + 接收端. \n");
    fmt::print("./udp_pg_efvi -send DMA|CTPIO eth dest_ip udp_size (udp_size >= 24). \n");
    fmt::print("./udp_pg_efvi -recv eth local_ip \n");

    exit(0);
}

// str: 包含mac头的数据
// len: 数据长度
static void recv_cb_func(const char *str, int32_t len) {
    const udp_hdr *udp_hd = (udp_hdr*)(str + sizeof(mac_hdr) + sizeof(ip_hdr));
    if (udp_hd->dst_port != htons(1577)) {
        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "port {}, unwant data.. \n", ntohs(udp_hd->dst_port));
        return;
    }

    static Sta sta;
    static std::vector<int64_t> delay_vec;

    static int32_t recv_cnt = 0;
    ++recv_cnt;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    MakeUdpPkt udp;
    const uint64_t  idx = *(uint64_t*)(str+udp.udp_payload_offset());
    const timespec *pre =  (timespec*)(str+udp.udp_payload_offset()+8);

    int64_t cost = (now.tv_sec - pre->tv_sec) * 1000000000 + (now.tv_nsec - pre->tv_nsec);

    delay_vec.push_back(cost);

    if (idx % SEND_CNT == 0) {
        const Sta::Rst &rst = sta(delay_vec);
        fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                "                 idx[{}] recv_cnt[{}], 1/2RTT time(ns): {} {} {} {}, mid[{}]. \n",
                idx, recv_cnt,
                rst.min, rst.max, rst.avg, rst.stddev, rst.m50);

        delay_vec.clear();
        delay_vec.reserve(128);
    }
}

static int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);
    const char *type = argv[1];

    if (0 == strcmp(type, "-send")) {
        if (argc < 5) { usage(); }

        fmt::print("bind_thread_to_cpu(20). \n");
        bind_thread_to_cpu(20);

        const char *mode= argv[2];
        const char *eth = argv[3];
        const char *dip = argv[4];
        uint16_t  dport = 1577;
        int32_t    size = atoi(argv[5]);
        if (size < 24) { usage(); }

        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {} {} -> {}:{} {}. \n", type, mode, eth, dip, dport, size);

        //
        uint32_t send_mode = 0;
        if (0 == strncasecmp(mode, "DMA", 3)) {
            send_mode = 1;
        }
        else if (0 == strncasecmp(mode, "CTPIO", 5)) {
            send_mode = 2;
        }
        else {
            usage();
        }

        // get mac
        uint8_t smac[8];
        char sip[16];
        {
            Nic nic;
            if (!nic.get_mac(eth, smac) || !nic.get_ip(eth, sip)) {
                fmt::print("{}. \n", nic.err());
                return 0;
            }
        }

        EfviUdpSend tx;

        if (!tx.init(eth)) {
            fmt::print("{} \n", tx.err());
            return 0;
        }

        fmt::print("{} \n", tx.efvi_version());
        fmt::print("{} \n", tx.efvi_driver_interface());
        fmt::print("{} \n", tx.efvi_nic_arch());
        fmt::print("efvi_support_ctpio: {} \n", tx.efvi_support_ctpio(eth));

        uint32_t dma_id = 0;
        PKT_BUF *cell = nullptr;

        MakeUdpPkt udp;

        //
        UtilsCycles::init();
        for (int64_t i = 1; true;) {
            tx.poll();
            if (!tx.get_usable_send_buf(cell, dma_id)) {
                // fmt::print(fg(fmt::rgb(250, 200, 0)) | fmt::emphasis::bold, "error: get_usable_send_buf failed. \n");
                continue;
            }

            char *addr = cell->user_buf;
            {
                udp.init_mac_hdr(addr, (const char*)smac);
                udp.init_ip_hdr_partial(addr, sip, dip);
                udp.init_udp_hdr_partial(addr, 1127, dport);
            }

            char *d = addr + 14 + 20 + 8;
            *((uint64_t*)(d)) = i;
            struct timespec *ts = (timespec*)(d + 8);
            clock_gettime(CLOCK_REALTIME, ts);

            const int32_t vlen = udp.set_hdr_finish(addr, size, i);

            // DMA
            if (send_mode == 1) {
                if (!tx.dma_send(vlen, dma_id)) {
                    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "{} \n", tx.err());
                    getchar();
                }
            }
            // CTPIO
            else if (send_mode == 2) {
                if (!tx.ctpio_send(vlen, dma_id)) {
                    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "{} \n", tx.err());
                    usleep(1000);
                    getchar();
                }
            }

            if (i % SEND_CNT == 0)
            {
                char tm[32] = {0};
                UtilsTimefmt::get_now2(tm);
                fmt::print("{}: {} ef_vi {} send len[{}] \n", tm, i, mode, vlen);
                UtilsCycles::sleep(1000*500); // 500ms
            }
            ++i;
        }
    }
    else if (0 == strcmp(type, "-recv")) {
        if (argc < 4) { usage(); }

        fmt::print("bind_thread_to_cpu(21). \n");
        bind_thread_to_cpu(21);

        const char *eth = argv[2];
        const char *local_ip = argv[3];
        uint16_t      port   = 1577;


        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {}, {} port {}. \n", type, eth, local_ip, port);

        EfviUdpRecv rx;
        fmt::print("{} \n", rx.efvi_version());
        fmt::print("{} \n", rx.efvi_driver_interface());

        if (!rx.init(eth) || !rx.add_filter(local_ip, port)) {
            fmt::print("{} \n", rx.err());
            return 0;
        }

        rx.recv(recv_cb_func);
    }
    else {
        usage();
    }

    return 0;
}


int32_t main(int32_t argc, char **argv) {
    udp_ping_pong(argc, argv);
    return 0;
}

