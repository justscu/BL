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

extern
void efvi_multicast_send_thread(uint8_t cpu_id,
                                const char *mode, // DMA or CTPIO
                                const char *nic, // 发送组播的网卡
                                const uint32_t udp_payload_len // udp载荷的长度
                               );

extern
void efvi_multicast_recv_thread(uint8_t cpu_id,
                                const char *nic,  // 接收组播的网卡
                                const char *dst_ip,     // 组播目的ip
                                const uint16_t dst_port // 组播目的port
                               );


void handle(int32_t s) {
    fmt::print("recv signal: {}. \n", s);
    exit(0);
}

void usage() {
    fmt::print("测量单向，需要 发送端 + 接收端. \n");

    fmt::print("udp_pg_efvi -send -f cfg.ini \n");
    fmt::print("udp_pg_efvi -recv -f cfg.ini \n");

    exit(0);
}


int32_t main(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);

    const char *type = argv[1]; // "-send" or "-recv"

    IniReader ini;
    if (!ini.load_ini(argv[3])) {
        fmt::print("load ini file [{}] failed. \n", argv[3]);
        return 0;
    }

    if (0 == strcmp(type, "-send")) {
        const char *mode = ini["send.mode"]; // dma or ctpio
        const char  *eth = ini["send.nic"];
        uint32_t    size = atoi(ini["send.udp_payload"]);
        uint32_t thd_cnt = atoi(ini["send.thread_cnt"]); // thread count.

        fmt::print("mode[{}] nic[{}] udp_payload[{}] thread_cnt[{}] \n\n", mode, eth, size, thd_cnt);
        sleep(3);

        for (uint32_t i = 0; i < thd_cnt; ++i) {
            std::thread th(std::bind(efvi_multicast_send_thread, 4+i, mode, eth, size));
            th.detach();
            sleep(1);
        }

        while(true) { sleep(1); }

        return 0;
    }

    if (0 == strcmp(type, "-recv")) {
        const char *eth    = ini["recv.nic"];
        const char *dst_ip = ini["recv.dst_ip"];
        uint16_t  dst_port = atoi(ini["recv.dst_port"]);

        std::thread th(std::bind(efvi_multicast_recv_thread, 18, eth, dst_ip, dst_port));
        th.join();

        return 0;
    }

    usage();
}
