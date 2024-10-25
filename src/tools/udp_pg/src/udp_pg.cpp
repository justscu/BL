#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "fmt/color.h"
#include "commx/utils.h"
#include "udp_pg.h"



#define PKT_CNT 1


void UdpPG::udp_pong(uint16_t port) {
    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "bind_thread_to_cpu(6). \n");
    fmt::print("listen port {}, back port {}. \n", port, port + 1);

    bind_thread_to_cpu(6);

    UtilsSocketUdp udp;
    if (!udp.create_socket() || !udp.bind(port)) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    char buf[4096];
    struct sockaddr_in cli;
    while (true) {
        socklen_t socklen = sizeof(sockaddr_in);
        // MSG_DONTWAIT !!!.
        int32_t rlen = ::recvfrom(udp.sockfd(), buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&cli, &socklen);
        if (rlen > 0) {
            cli.sin_port = htons(port+1); // dest port.
            ::sendto(udp.sockfd(), buf, rlen, 0, (struct sockaddr*)&cli, sizeof(struct sockaddr));

            // fmt::print("recv {} data, and send back . \n", rlen);
        }
    }
}

void UdpPG::send_udp(const char *lip, uint16_t lport, const char *dip, uint16_t dport, int32_t pkt_len) {
    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
            "bind_thread_to_cpu(2), send udp, {}:{} => {}:{}. \n", lip, lport, dip, dport);

    bind_thread_to_cpu(2);

    if (pkt_len > 4096) { pkt_len = 4096; }
    fmt::print("pkt_len : {}. \n", pkt_len);

    UtilsSocketUdp udp;
    if (!udp.create_socket() || !udp.bind(lip, lport)) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    UtilsCycles::init();

    char buf[4096];
    struct sockaddr_in svr;
    svr.sin_family = AF_INET;
    svr.sin_port = htons(dport);
    inet_pton(AF_INET, dip, &(svr.sin_addr.s_addr));

    uint32_t k = 1;
    while (true) {
        for (int32_t i = 1; i <= PKT_CNT; ++i) {
            *(((uint64_t*)buf)) = k;
            struct timespec *ts = (timespec*)(buf+8);
            clock_gettime(CLOCK_REALTIME, ts);

            int32_t slen = sendto(udp.sockfd(), buf, pkt_len, 0, (struct sockaddr*)&svr, sizeof(struct sockaddr));
            if (slen != pkt_len) {
                fmt::print("sendto failed: {}. \n", strerror(errno));
                exit(0);
            }
        }

        if (++k % SEND_CNT == 0) {
            char tm[32] = {0};
            UtilsTimefmt::get_now3(tm);
            fmt::print("{}: send {} packets to {}:{} success, UDP packet size: {}. \n",
                    tm, SEND_CNT, dip, dport, pkt_len);

            UtilsCycles::sleep(1000*1000*3); // 3 second.
        }

        UtilsCycles::sleep(1000);
    }
}

void UdpPG::recv_udp(uint16_t port) {
    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "bind_thread_to_cpu(4). \n");

    bind_thread_to_cpu(4);

    UtilsCycles::init();

    UtilsSocketUdp udp;
    if (!udp.create_socket() || !udp.bind(port)) {
        fmt::print("{} \n", udp.err_str());
        return;
    }

    Sta sta;
    std::vector<int64_t> delay_vec;
    delay_vec.reserve(128);

    char buf[8192];
    struct sockaddr_in cli;
    memset(&cli, 0, sizeof(sockaddr_in));
    socklen_t socklen = sizeof(sockaddr_in);

    uint32_t recv_cnt = 0;

    while (true) {
        int32_t rlen = ::recvfrom(udp.sockfd(), buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&cli, &socklen);
        if (rlen > 0) {
            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now);

            const uint64_t  idx = *(uint64_t*)buf;
            const timespec *pre = (timespec*)(buf+8);


            int64_t cost = (now.tv_sec - pre->tv_sec) * 1000000000 + (now.tv_nsec - pre->tv_nsec);

            delay_vec.push_back(cost);

            recv_cnt += 1;

            if (idx % SEND_CNT == 0) {
                const Sta::Rst &rst = sta(delay_vec);
                fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                        "                 idx[{}] recv_cnt[{}], RTT time(ns): {} {} {} {}, mid[{}]. \n",
                        idx, recv_cnt,
                        rst.min, rst.max, rst.avg, rst.stddev, rst.m50);

                delay_vec.clear();
            }
        }
    } // while
}
