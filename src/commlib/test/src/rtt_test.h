#pragma once

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "fmt/format.h"

#define PKT_SIZE 512 // 每次发送的数据包大小为512字节

inline uint64_t now_nanosec() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)(ts.tv_sec) * 1000000000ull) + ts.tv_nsec;
}

inline void print_head() {
    fmt::print("{:^24} {:>5} {:>16} {:>16} {:>16} {:>16} \n", "tips", "count", "min[us]", "max[us]", "avg[us]", "stddev[us]");
    fmt::print("{:-<98}\n", "");
}

inline void print(const char *tips, std::vector<int64_t> &s) {
    Sta sta;
    const Sta::Rst &rst = sta(s);
    fmt::print("{:^24} {:>5} {:>16} {:>16} {:>16} {:>16} \n", tips, rst.cnt, rst.min/1000, rst.max/1000, rst.avg/1000, rst.stddev/1000);
}



struct client_param {
    char ip[32] = {0};
    uint16_t port = 0;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 测量函数，
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void thread_loop_client(const client_param *param) {
    std::vector<int64_t> vec_total; // 从客户端 -> 服务端 -> 客户端
    std::vector<int64_t>  vec_self; // 从网卡到进程

    // send
    char send_buf[PKT_SIZE];
    struct msghdr *smsg = new msghdr;
    {
        smsg->msg_iov    = new iovec;
        smsg->msg_iovlen = 1;

        smsg->msg_iov->iov_base = send_buf;
        smsg->msg_iov->iov_len  = sizeof(send_buf);
    }

    // recv
    char msg_control[512]; // 控制信息
    char recv_buf[PKT_SIZE];
    struct msghdr *rmsg = new msghdr;
    {
        rmsg->msg_iov    = new iovec;
        rmsg->msg_iovlen = 1;
        rmsg->msg_iov->iov_base = recv_buf;
        rmsg->msg_iov->iov_len  = PKT_SIZE;

        rmsg->msg_control    = msg_control;
        rmsg->msg_controllen = sizeof(msg_control);
    }

    while (true) {
        UtilsSocketUdp sock;
        if (!sock.create_socket()
                || !sock.set_sockopt_reuse_addr(true)
                || !sock.set_sockopt_timestampns(true)
                || !sock.connect(param->ip, param->port)) {
            fmt::print("{} \n", sock.err_str());
            sock.close_socket();
            continue ;
        }

        // 设置带超时模式
        struct timeval tv{2, 0};
        if (!sock.set_sockopt_recvtimeout(tv)) {
            fmt::print("{} \n", sock.err_str());
            sock.close_socket();
            continue;
        }

        for (int64_t pkg_cnt = 1; pkg_cnt <= 10000; ++pkg_cnt) {
            snprintf(send_buf, sizeof(send_buf)-1, "%d, %ld, %ld", getpid(), syscall(SYS_gettid), pkg_cnt);

            uint64_t t0 = now_nanosec(); // 当前时间
            const int32_t slen = sendmsg(sock.sockfd(), smsg, 0);
            if (slen <= 0) {
                if (errno == EINTR) { continue; }
                fmt::print("sendmsg failed: {}. \n", strerror(errno));
                break;
            }
            // fmt::print("sendmsg success. \n");

            bool timeout = false;
            // recv message for server.
            uint64_t t1 = 0;
            while (true) {
                const int32_t rlen = recvmsg(sock.sockfd(), rmsg, 0); // block
                t1 = now_nanosec();
                if (t1 - t0 >= 2000000000ull) {
                    fmt::print("recvmsg timeout. \n");
                    timeout = true;
                    break;
                }
                if (-1 == rlen) {
                    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                    }
                    fmt::print("recvmsg return -1. {}. \n", strerror(errno));
                    return;
                }

                // recv msg success.
                // fmt::print("recvmsg success. \n");
                break;
            } // while

            if (!timeout) {
                // lost package.
                if (0 != memcmp(send_buf, recv_buf, sizeof(send_buf))) {
                    fmt::print("package not same. \n");
                    continue;
                }

                struct cmsghdr *cmsg = CMSG_FIRSTHDR(rmsg);
                for (; cmsg; cmsg = CMSG_NXTHDR(rmsg, cmsg)) {
                    if (cmsg->cmsg_level == SOL_SOCKET) {
                        if (cmsg->cmsg_type == SO_TIMESTAMPNS) {
                            struct timespec *ts = (struct timespec*)CMSG_DATA(cmsg);
                            uint64_t tp = ((uint64_t)(ts->tv_sec) * 1000000000ull) + ts->tv_nsec;
                            vec_total.push_back(t1-tp);
                            break;
                        }
                    }
                }

                vec_self.push_back(t1-t0);
            }

            usleep(100);
        } // for

        sock.close_socket();

        if (vec_total.size() > 0) {
            print("client->server->client", vec_total);
        }

        if (vec_self.size() > 0) {
            print("netcard -> client", vec_self);
        }

        break;
    } // while
}




void thread_loop_server(const client_param *param) {
    struct msghdr msg;
    msg.msg_name    = new char[64]; // 网卡
    msg.msg_namelen = 64;
    msg.msg_iov    = new iovec;
    msg.msg_iovlen = 1;
    msg.msg_iov->iov_base = new char[2048]; // 收到的消息
    msg.msg_iov->iov_len  = 2048;
    msg.msg_control    = new char[4096]; // 控制信息
    msg.msg_controllen = 4096;

    UtilsSocketUdp sock;
    if (!sock.create_socket()
            || !sock.set_sockopt_reuse_addr(true)
            || !sock.set_sockopt_reuse_port(true)
            || !sock.set_sockopt_pkginfo(true)
            || !sock.bind(param->port)
            || !sock.set_sockopt_busypoll(true)) {
        fmt::print("thread[{}] {} \n", syscall(SYS_gettid), sock.err_str());
        sock.close_socket();
        return;
    }

    while (true) {
        msg.msg_iov->iov_len = 2048;
        msg.msg_controllen = 4096;

        const int32_t rlen = recvmsg(sock.sockfd(), &msg, MSG_DONTWAIT);
        if (rlen == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }

            fmt::print("thread[{}] recvmsg failed: {} \n", syscall(SYS_gettid), strerror(errno));
            break;
        }

        //
        msg.msg_iov->iov_len = rlen;
        const int32_t slen = sendmsg(sock.sockfd(), &msg, 0);
        if (slen != rlen) {
            fmt::print("thread[{}] sendlen[{}] != recvlen[{}]. \n", syscall(SYS_gettid), slen, rlen);
            break;
        }
    } // while
}


/*    测试结果
           tips           count          min[us]          max[us]          avg[us]         mdev[us]
--------------------------------------------------------------------------------------------------
 client->server->client  10000                4               28             7.27             1.10
   netcard -> client     10000                1               11             3.63             0.56
 client->server->client  10000                4               15             6.93             0.37
   netcard -> client     10000                1                6             3.44             0.13
 client->server->client  10000                4               14             6.97             0.39

 */

int32_t ttl_test(int32_t argc, char **argv) {
    if (1) {
        if (argc < 2) {
            fmt::print("Usage: ./client ip port \n");
            return 0;
        }

        client_param cp;
        strcpy(cp.ip, argv[1]);;
        cp.port = atoi(argv[2]);

        fmt::print("echo client pkg_size[{}].\n", PKT_SIZE);
        print_head();
        for (int32_t i = 0; i < 1000; ++i) {
            bind_thread_to_cpu(2);
            thread_loop_client(&cp);
        }
    }
    else {
        client_param cp;
        cp.port = 1859;

        fmt::print("udp echo server, listen port[{}].\n", cp.port);
        bind_thread_to_cpu(4);
        thread_loop_server(&cp);
    }

    return 0;
}
