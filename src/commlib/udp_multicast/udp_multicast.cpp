#include <stdio.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include "utils/utils_times.h"
#include "udp_multicast.h"


void UdpMulticast::set_udpgroup(const UdpGroup &v) {
    memcpy(&udp_group_, &v, sizeof(UdpGroup));
}

bool UdpMulticast::create_udp_socket() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd_ < 0) {
        fprintf(stderr, "create socket failed. [%s] \n", strerror(errno));
        return false;
    }
    //
    int32_t reuse = 1;
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (char*)(&reuse), sizeof(reuse))) {
        fprintf(stderr, "setsockopt[SO_REUSEADDR] failed. err[%s] \n", strerror(errno));
        return false;
    }
    return true;
}

bool UdpMulticast::bind_udp_socket() {
    struct sockaddr_in laddr;
    memset(&laddr, 0, sizeof(struct sockaddr_in));
    laddr.sin_family      = AF_INET;
    laddr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(udp_group_.local_ip);
    laddr.sin_port        = htons(udp_group_.local_port);

    if (0 != bind(sockfd_, (struct sockaddr*)&laddr, sizeof(sockaddr))) {
        fprintf(stderr, "bind[INADDR_ANY:%d] failed. err[%s]\n", udp_group_.local_port, strerror(errno));
        return false;
    }
    fprintf(stdout, "bind[INADDR_ANY:%d] success.\n", udp_group_.local_port);

    // 绑定组播网卡地址，若不设置，会走默认路由
    struct in_addr oaddr = {0};
    oaddr.s_addr = inet_addr(udp_group_.local_ip);
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&oaddr, sizeof(struct in_addr))) {
        fprintf(stdout, "setsockopt[IP_MULTICAST_IF] failed. err[%s].\n", strerror(errno));
        return false;
    }

    return true;
}

bool UdpMulticast::set_sockopt_addmembership() {
    struct ip_mreq group_addr;
    memset(&group_addr, 0, sizeof(ip_mreq));
    group_addr.imr_multiaddr.s_addr = inet_addr(udp_group_.group_ip);
    group_addr.imr_interface.s_addr = inet_addr(udp_group_.local_ip);
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group_addr, sizeof(group_addr))) {
        fprintf(stderr, "setsockopt[IP_ADD_MEMBERSHIP] failed[%s] \n", strerror(errno));
        return false;
    }
    fprintf(stdout, "setsockopt[IP_ADD_MEMBERSHIP] success.\n");
    return true;
}

// set send & recv buffer size
bool UdpMulticast::set_sockopt_sendbuf() {
    // get default SO_SNDBUF
    int32_t val = 0;
    int32_t val_len = sizeof(val);
    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (void*)&val, (unsigned int *)&val_len)) {
        fprintf(stdout, "getsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "getsockopt[SO_SNDBUF] default bufsize[%d] \n", val);
    // set SO_SNDBUF
    val = 16*1024*1024;
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (void*)&val, val_len)) {
        fprintf(stdout, "setsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "setsockopt[SO_SNDBUF] bufsize[16M] success \n");
    // get SO_SNDBUF again
    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (void*)&val, (unsigned int *)&val_len)) {
        fprintf(stdout, "getsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "getsockopt[SO_SNDBUF] bufsize[%d] \n", val);
    return true;
}

bool UdpMulticast::set_sockopt_recvbuf() {
    // get default SO_RCVBUF
    int32_t val = 0;
    int32_t val_len = sizeof(val);
    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (void*)&val, (unsigned int *)&val_len)) {
        fprintf(stdout, "getsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "getsockopt[SO_RCVBUF] default bufsize[%d] \n", val);
    // set SO_RCVBUF
    val = 16*1024*1024;
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (void*)&val, val_len)) {
        fprintf(stdout, "setsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "setsockopt[SO_RCVBUF] bufsize[16M] success \n");
    // get SO_RCVBUF again
    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (void*)&val, (unsigned int *)&val_len)) {
        fprintf(stdout, "getsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "getsockopt[SO_RCVBUF] bufsize[%d] \n", val);
    return true;
}

bool UdpMulticast::set_sockopt_ttl() {
    // set ttl
    int32_t ttl = 128;
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))) {
        fprintf(stdout, "setsockopt[IP_MULTICAST_TTL] failed. err[%s]. \n", strerror(errno));
        return false;
    }
    fprintf(stdout, "setsockopt[IP_MULTICAST_TTL] 128 \n");
    return true;
}

// 设置loop属性，若不设置，本机的环回收不到组播
// loop = 0, 不设置
bool UdpMulticast::set_sockopt_loop(int32_t loop) {
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, (void*)&loop, sizeof(loop))) {
        fprintf(stdout, "setsockopt[IP_MULTICAST_LOOP] failed. err[%s].\n", strerror(errno));
        return false;
    }
    fprintf(stdout, "setsockopt[IP_MULTICAST_LOOP] loop[%d] \n", loop);
    return true;
}

void UdpMulticast::close_socket() {
    close(sockfd_);
    sockfd_ = -1;
}

void UdpMulticast::sendmsg_example() {
    char a[] = "123";
    char b[] = "4567890";
    // data need send
    struct iovec vec[2];
    vec[0].iov_base = a;
    vec[0].iov_len  = 4;
    vec[1].iov_base = b;
    vec[1].iov_len  = 8;

    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(udp_group_.group_ip);
    addr.sin_port        = htons(udp_group_.group_port);

    msghdr msg;
    msg.msg_name       = &addr; // 目的地址
    msg.msg_namelen    = sizeof(addr);
    msg.msg_iov        = vec; // 需要发送的多缓冲区
    msg.msg_iovlen     = sizeof(vec) / sizeof(vec[0]);
    msg.msg_control    = nullptr;
    msg.msg_controllen = 0;
    msg.msg_flags      = 0;

    int32_t slen = sendmsg(sockfd_, &msg, MSG_DONTWAIT);
    fprintf(stdout, "send length[%d]. \n", slen);
}

void UdpMulticast::test_server(uint32_t sleep_ms) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(udp_group_.group_ip);
    addr.sin_port        = htons(udp_group_.group_port);

    UtilsCycles::init();
    //
    uint64_t i = 0;
    char buf[8*1024]; // 8K.
    while (true) {
        *(uint64_t*)buf = ++i;
        int32_t slen = sendto(sockfd_, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(sockaddr_in));
        if (slen == -1) {
            fprintf(stdout, "sendto error. [%s] \n", strerror(errno));
            break;
        }

        if (i % 100 == 0) {
            fprintf(stdout, "send_%ld OK. size[%ld] \n", i, sizeof(buf));
            UtilsCycles::sleep(sleep_ms * 1000);
        }
    }
}


void UdpMulticast::test_client() {
    uint64_t i = 0;
    uint64_t lost_cnt = 0;
    char buf[16*1024];
    while (true) {
        int32_t rlen = read(sockfd_, buf, sizeof(buf));
        if (rlen < 0) {
            fprintf(stderr, "recv_%ld err[%s] \n", i, strerror(errno));
        }
        else {
            uint64_t idx = *(uint64_t*)(&buf);
            if (i == 0 || idx == 1) { i = idx-1; }

            if (i+1 == idx) {
                fprintf(stdout, "recv package idx[%ld] len[%d], lost_cnt[%ld] \n", idx, rlen, lost_cnt);
            }
            else {
                lost_cnt += (idx-i);
                i = idx;
                fprintf(stdout, "recv package idx[%ld] len[%d], lost_cnt[%ld] \n", idx, rlen, lost_cnt);
            }

            ++i;
        }
    }
}
