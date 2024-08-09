#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "utils_socket.h"
#include "utils_times.h"
#include "fmt/format.h"
#include "fmt/color.h"


#define send_buf_len  1600

void Utils_test_multicast_server(const UtilsSocketMulticast::MultiCastAddr &addr, int32_t interval_ms) {
    fmt::print("发送组播包大小 {} 字节, 每发100个包sleep {} ms. \n", send_buf_len, interval_ms);

    UtilsSocketMulticast so;
    so.set_multicast_addr(addr);

    if (!so.create_socket() || !so.set_sockopt_reuse_addr(true) || !so.bind_socket_multicast()) {
        fmt::print("{} \n", so.err_str());
        return;
    }

    if (!so.set_sockopt_sendbuf(16*1024*1024)) { fmt::print("{} \n", so.err_str()); }
    if (!so.get_sockopt_multicast_ttl()) { fmt::print("{} \n", so.err_str()); }
    if (!so.set_sockopt_multicast_ttl()) { fmt::print("{} \n", so.err_str()); }
    if (!so.set_sockopt_multicast_loop(true)) { fmt::print("{} \n", so.err_str()); }

    UtilsCycles::init();

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(sockaddr_in));
    dest.sin_family      = AF_INET;
    dest.sin_addr.s_addr = inet_addr(addr.group_ip);
    dest.sin_port        = htons(addr.group_port);

    //
    uint64_t i = 0;
    char sendbuf[send_buf_len];
    uint64_t *num = (uint64_t*)sendbuf;
    while (true) {
        *num = ++i;
        // *(uint64_t*)sendbuf = ++i; // begin with 1.
        const int32_t slen = sendto(so.sockfd(), sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&dest, sizeof(sockaddr_in));
        if (slen == -1) {
            fmt::print("ERR: sendto failed. len[{}] err: {}. \n", sizeof(sendbuf), slen);
            break;
        }

        if (i % 100 == 0) {
            fmt::print("send_");
            fmt::print(fg(fmt::rgb(255, 0, 1)) | fmt::emphasis::italic, "{} ", i);
            fmt::print("size[{}] \n", slen);

            UtilsCycles::sleep(interval_ms * 1000);
        }
    } // while

    so.close_socket();
}

void Utils_test_multicast_client(const UtilsSocketMulticast::MultiCastAddr &addr) {
    fmt::print("接收组播包 \n");

    UtilsSocketMulticast so;
    so.set_multicast_addr(addr);
    if (!so.create_socket()
            || !so.bind_socket_multicast()
            || !so.set_sockopt_multicast_addmembership()) {
        fmt::print("{} \n", so.err_str());
        return;
    }

    if (!so.set_sockopt_recvbuf(8*1024*1024)) { fmt::print("{} \n", so.err_str()); }
    if (!so.set_sockopt_multicast_loop(true)) { fmt::print("{} \n", so.err_str()); }

    //
    uint64_t i = 1;
    uint64_t lost_cnt = 0;
    char buf[16*1024];
    uint64_t *num = (uint64_t*)buf;

    while (true) {
        const int32_t rlen = ::recv(so.sockfd(), buf, sizeof(buf), 0);
        if (rlen < 0) {
            fmt::print("ERR: recv failed: {}. \n", strerror(errno));
        }
        else {
            const uint64_t idx = *num;
            // reset
            if (idx == 1) {
                i = 1;
                fmt::print(fg(fmt::rgb(255, 0, 1)) | fmt::emphasis::italic, "server reset. \n");
            }
            if (i == idx) {
                fmt::print("recv pkg idx[{}], size[{}]. \n", idx, rlen);
                ++i;
            }
            else {
                lost_cnt += (idx-i);
                i = idx;
                fmt::print("recv pkg idx[{}], size[{}]. ", idx, rlen);
                fmt::print(fg(fmt::rgb(255, 0, 1)) | fmt::emphasis::italic, "total_lost[{}]. \n", lost_cnt);
            }
        }
    }

    so.close_socket();
}

void mulicast_test(int32_t argc, char **argv) {
    if (argc < 5) {
        fmt::print("VER: \n");
        fmt::print("Usage: multicast_test c|s group_ip group_port local_ip local_port sleep(ms) \n");
        fmt::print("       sleep: 指每发送100个包，sleep 多少 ms. (default=10ms). \n\n");
        exit(0);
    }

    UtilsSocketMulticast::MultiCastAddr g;
    memset(&g, 0, sizeof(g));
    strcpy(g.group_ip, argv[2]);
    g.group_port = atoi(argv[3]);
    strcpy(g.local_ip, argv[4]);
    g.local_port = atoi(argv[5]);

    int32_t s = 10;
    if (argc > 6) { s = atoi(argv[6]); }

    if (argv[1][0] == 's' || argv[1][0] == 'S') {
        return Utils_test_multicast_server(g, s);
    }

    if (argv[1][0] == 'c' || argv[1][0] == 'C') {
        return Utils_test_multicast_client(g);
    }
}
