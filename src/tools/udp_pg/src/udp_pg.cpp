#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "fmt/color.h"
#include "commx/utils.h"
#include "udp_pg.h"



#define PKT_CNT 1
void UdpPG::udp_pong(uint16_t port) {
    fmt::print("bind_thread_to_cpu(6). \n");
    bind_thread_to_cpu(6);

    UtilsSocketUdp udp;
    if (!udp.create_socket() || !udp.bind(port)) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    port += 1;
    char buf[4096];
    struct sockaddr_in cli;
    while (true) {
        socklen_t socklen = sizeof(sockaddr_in);
        // MSG_DONTWAIT !!!.
        int32_t rlen = ::recvfrom(udp.sockfd(), buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&cli, &socklen);
        if (rlen > 0) {
            cli.sin_port = htons(port); // dest port.
            ::sendto(udp.sockfd(), buf, rlen, 0, (struct sockaddr*)&cli, sizeof(struct sockaddr));
        }
    }
}

void UdpPG::send_udp(const char *dip, uint16_t dport, int32_t pkt_len) {
    fmt::print("bind_thread_to_cpu(2). \n");
    bind_thread_to_cpu(2);

    if (pkt_len > 4096) { pkt_len = 4096; }
    fmt::print("pkt_len : {}. \n", pkt_len);

    UtilsSocketUdp udp;
    if (!udp.create_socket()) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    UtilsCycles::init();

    char buf[4096];
    struct sockaddr_in svr;
    svr.sin_family = AF_INET;
    svr.sin_port = htons(dport);
    inet_pton(AF_INET, dip, &(svr.sin_addr.s_addr));

    // warm up.
    state_ = 1;
    for (int32_t i = 0; i < 1000; ++i) {
        uint64_t *tm = (uint64_t*)buf;
        *tm = UtilsClock::get_ns();
        int32_t slen = sendto(udp.sockfd(), buf, pkt_len, 0, (struct sockaddr*)&svr, sizeof(struct sockaddr));
        if (slen != pkt_len) {
            fmt::print("sendto failed: {}. \n", strerror(errno));
            exit(0);
        }
    }

    fmt::print("warmup over. \n");
    UtilsCycles::sleep(1000*1000*2); // 2 second.

    while (true) {
        state_ = 2; // sending.
        for (int32_t i = 1; i <= PKT_CNT; ++i) {
            *((uint64_t*)buf) = UtilsClock::get_ns();
            int32_t slen = sendto(udp.sockfd(), buf, pkt_len, 0, (struct sockaddr*)&svr, sizeof(struct sockaddr));
            if (slen != pkt_len) {
                fmt::print("sendto failed: {}. \n", strerror(errno));
                exit(0);
            }
        }

        char tm[32] = {0};
        UtilsTimefmt::get_now3(tm);
        fmt::print("{}: send {} packets to {}:{} success, UDP packet size: {}. \n",
                tm, PKT_CNT, dip, dport, pkt_len);

        state_ = 3;
        UtilsCycles::sleep(1000*500); // 0.5 second.
    }
}

void UdpPG::recv_udp(uint16_t port) {
    fmt::print("bind_thread_to_cpu(4). \n");
    bind_thread_to_cpu(4);

    UtilsCycles::init();

    UtilsSocketUdp udp;
    if (!udp.create_socket() || !udp.bind(port)) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    uint64_t cost_total = 0;
    uint64_t cost_cnt = 0;

    uint64_t avg_cost = 0;
    uint64_t avg_cnt  = 0;

    char buf[8192];
    struct sockaddr_in cli;
    memset(&cli, 0, sizeof(sockaddr_in));
    socklen_t socklen = sizeof(sockaddr_in);
    while (true) {
        int32_t rlen = ::recvfrom(udp.sockfd(), buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&cli, &socklen);
        if (rlen > 0) {
            uint64_t now = UtilsClock::get_ns();
            uint64_t pre = *(uint64_t*)buf;

            cost_total += (now - pre);
            cost_cnt   += 1;
            avg_cost   += (now - pre);
            avg_cnt    += 1;
        }
        else {
            if (state_ == 1) {
                cost_total = cost_cnt = 0;
                avg_cost   = avg_cnt  = 0;
            }
            else if (state_ == 3) {
                if (cost_cnt >= PKT_CNT) {
                    fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                            "                 round-trip time: {} ns. pkt_num: {}. avg: {}. \n",
                            cost_total/cost_cnt, cost_cnt, avg_cost/avg_cnt);
                    cost_total = cost_cnt = 0;
                    cost_cnt = 0;
                }
            }
        }
    } // while
}
