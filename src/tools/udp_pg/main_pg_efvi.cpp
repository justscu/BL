#include <csignal>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <unistd.h>
#include <functional>
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
    fmt::print("./udp_pg_efvi -send eth dest_ip udp_size, size >= 24. \n");
    fmt::print("./udp_pg_efvi -recv eth local_ip \n");

    exit(0);
}

// str: 包含mac头的数据
// len: 数据长度
static void recv_cb_func(const char *str, int32_t len) {
    static int32_t total = 1;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    MakeUdpPkt udp;
    uint64_t  idx = *(uint64_t*)(str+udp.udp_payload_offset());
    timespec *pre =  (timespec*)(str+udp.udp_payload_offset()+8);

    int64_t cost = (now.tv_sec - pre->tv_sec) * 1000000000 + (now.tv_nsec - pre->tv_nsec);

    fmt::print("                 send {} {} \n", pre->tv_sec, pre->tv_nsec);
    fmt::print("                 recv {} {} \n", now.tv_sec,  now.tv_nsec);

    char tm[32] = {0};
    UtilsTimefmt::get_now2(tm);
    fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
            "                 {}: {} {} len[{}] RTT/2 time: {} ns. \n",
            tm, idx, total++, len, cost);
}

static int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);
    const char *type = argv[1];

    if (0 == strcmp(type, "-send")) {
        if (argc < 5) { usage(); }

        fmt::print("bind_thread_to_cpu(20). \n");
        bind_thread_to_cpu(20);

        const char *eth = argv[2];
        const char *dip = argv[3];
        uint16_t  dport = 1577;
        int32_t    size = atoi(argv[4]);
        if (size < 24) { usage(); }

        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {} -> {}:{} {}. \n", type, eth, dip, dport, size);

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
        fmt::print("{} \n", tx.efvi_version());
        fmt::print("{} \n", tx.efvi_driver_interface());

        if (!tx.init(eth)) {
            fmt::print("{} \n", tx.err());
            return 0;
        }

        EfviSendDataCell *cell = tx.get_send_buf();
        MakeUdpPkt udp;
        udp.init_mac_hdr((char*)cell, (const char*)smac);
        udp.init_ip_hdr_partial((char*)cell, sip, dip);
        udp.init_udp_hdr_partial((char*)cell, 1127, dport);

        //
        char *d = cell->payload;
        UtilsCycles::init();
        for (int64_t i = 1; true; ++i) {
            UtilsCycles::sleep(1000*1); // 500ms

            *((uint64_t*)(d)) = i;
            struct timespec *ts = (timespec*)(d + 8);
            clock_gettime(CLOCK_REALTIME, ts);

            const int32_t vlen = udp.set_hdr_finish((char*)cell, size, i);
            tx.send(vlen);

            char tm[32] = {0};
            UtilsTimefmt::get_now2(tm);
            fmt::print("{}: {} ef_vi_transmit send len[{}] \n", tm, i, vlen);
        }
    }
    else if (0 == strcmp(type, "-recv")) {
        if (argc < 4) { usage(); }

        fmt::print("bind_thread_to_cpu(21). \n");
        bind_thread_to_cpu(21);

        const char *eth = argv[2];
        const char *src_ip = argv[3];
        uint16_t      port = 1577;


        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {}, {} port {}. \n", type, eth, src_ip, port);

        EfviUdpRecv rx;
        fmt::print("{} \n", rx.efvi_version());
        fmt::print("{} \n", rx.efvi_driver_interface());

        if (!rx.init(eth) || !rx.add_filter(src_ip, port)) {
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

