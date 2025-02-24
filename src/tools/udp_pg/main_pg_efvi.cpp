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
    fmt::print("./udp_pg_efvi -send DMA|CTPIO eth udp_size (udp_size >= 24). \n");
    fmt::print("./udp_pg_efvi -recv eth dest_ip dest_port \n");

    exit(0);
}


int32_t main(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);
    const char *type = argv[1];

    if (0 == strcmp(type, "-send")) {
        if (argc < 4) { usage(); }

        const char *mode = argv[2];
        const char  *eth = argv[3];
        uint32_t    size = atoi(argv[4]);
        if (size < 24) { usage(); }

        for (uint32_t i = 0; i < 8; ++i) {
            std::thread th(std::bind(efvi_multicast_send_thread, 4+i, mode, eth, size));
            th.detach();
            sleep(1);

        }

        while(true) { sleep(1); }

        return 0;
    }

    if (0 == strcmp(type, "-recv")) {
        if (argc < 5) { usage(); }

        const char *eth    = argv[2];
        const char *dst_ip = argv[3];
        uint16_t  dst_port = atoi(argv[4]);

        std::thread th(std::bind(efvi_multicast_recv_thread, 18, eth, dst_ip, dst_port));
        th.join();

        return 0;
    }

    usage();
}
