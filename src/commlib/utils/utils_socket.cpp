#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include "fmt/format.h"
#include "utils_socket.h"

// true : tcp; false : udp.
bool UtilsSocket::create_socket_ipv4(bool tcp) {
    if (tcp) {
        sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    // udp.
    else {
        sockfd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (sockfd_ <= 0) {
        fmt::print("ERR: create socket failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

void UtilsSocket::close_socket() {
    close(sockfd_);
    sockfd_ = -1;
}

bool UtilsSocket::connect(const char *ip, const uint16_t port) {
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (1 != inet_pton(AF_INET, ip, &addr.sin_addr)) {
        fmt::print("ERR: inet_pton failed: [{}:{}] {} \n", ip, port, strerror(errno));
        return false;
    }

    if (0 != ::connect(sockfd(), (struct sockaddr*)(&addr), sizeof(sockaddr_in))) {
        fmt::print("ERR: connect failed. [%s:%u] [%s] \n", ip, port, strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocket::set_sockopt_reuse_addr() {
    int32_t reuse = 1;
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (char*)(&reuse), sizeof(reuse))) {
        fmt::print("ERR: setsockopt[SO_REUSEADDR] failed: {}. \n", strerror(errno));
        return false;
    }
    fmt::print("setsockopt[SO_REUSEADDR] success. \n");
    return true;
}

bool UtilsSocket::set_sockopt_sendbuf(const int32_t size) {
    int32_t gval = 0; // get value.
    int32_t  len = sizeof(gval);
    // set SO_SNDBUF
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (void*)&size, len)) {
        fmt::print("ERR: setsockopt[SO_SNDBUF] failed: {}. \n", strerror(errno));
        return false;
    }

    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (void*)&gval, (unsigned int *)&len)) {
        fmt::print("ERR: getsockopt[SO_SNDBUF] failed: {}. \n", strerror(errno));
        return false;
    }

    const bool ret = (size == gval);
    fmt::print("setsockopt[SO_SNDBUF] {}, size[{}M] \n", ret ? "success" : "failed", size/1024/1024);
    return ret;
}

bool UtilsSocket::set_sockopt_recvbuf(const int32_t size) {
    int32_t gval = 0; // get value.
    int32_t  len = sizeof(gval);
    // set SO_RCVBUF
    if (0 != setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (void*)&size, len)) {
        fmt::print("ERR: setsockopt[SO_RCVBUF] failed: {}. \n", strerror(errno));
        return false;
    }

    if (0 != getsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (void*)&gval, (unsigned int *)&len)) {
        fmt::print("ERR: getsockopt[SO_RCVBUF] failed: {}. \n", strerror(errno));
        return false;
    }

    const bool ret = (size == gval);
    fmt::print("setsockopt[SO_RCVBUF] {}, size[{}M] \n", ret ? "success" : "failed", size/1024/1024);
    return ret;
}

bool UtilsSocket::set_sockopt_nonblocking(bool value) {
    int32_t flag = fcntl(sockfd_, F_GETFL, 0);
    if (flag == -1) {
        fmt::print("ERR: fcntl[F_GETFL] failed: {}. \n", strerror(errno));
        return false;
    }
    flag = value ? (flag | O_NONBLOCK) : (flag & ~O_NONBLOCK);

    const int32_t ret = fcntl(sockfd_, F_SETFL, flag);
    if (ret != 0) {
        fmt::print("ERR: fcntl[F_SETFL] failed: {}. \n", strerror(errno));
        return false;
    }

    fmt::print("fcntl[F_SETFL] nonblocking[{}] success. \n", value);
    return true;
}

bool UtilsSocket::set_sockopt_keepalive() {
    bool ret = 0;
    int32_t alive = 1;
    ret = setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive));
    if (ret == -1) {
        fmt::print("ERR: setsockopt [SO_KEEPALIVE] failed: {}. \n", strerror(errno));
        return false;
    }

    // 当tcp空闲30秒后，开始检测，发送keepalive数据
    int32_t idletime = 60;
    ret = setsockopt(sockfd_, SOL_TCP, TCP_KEEPIDLE, &idletime, sizeof(idletime));
    if (ret == -1) {
        fmt::print("ERR: setsockopt [TCP_KEEPIDLE] failed: {}. \n", strerror(errno));
        return false;
    }

    // 每隔20S,发一次keepalive数据
    int32_t interval = 20;
    ret = setsockopt(sockfd_, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    if (ret == -1) {
        fmt::print("ERR: setsockopt [TCP_KEEPINTVL] failed: {}. \n", strerror(errno));
        return false;
    }

    // 一共发10个keepalive数据
    int32_t cnt = 10;
    ret = setsockopt(sockfd_, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    if (ret == -1) {
        fmt::print("ERR: setsockopt [TCP_KEEPCNT] failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void UtilsSocketUDP::set_multicast_addr(const MultiCastAddr &addr) {
    memcpy(&multicast_addr_, &addr, sizeof(MultiCastAddr));
}

bool UtilsSocketUDP::bind_socket_multicast() {
    struct sockaddr_in laddr;
    memset(&laddr, 0, sizeof(struct sockaddr_in));
    laddr.sin_family      = AF_INET;
    laddr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(udp_group_.local_ip);
    laddr.sin_port        = htons(multicast_addr_.local_port);

    if (0 != bind(sockfd_, (struct sockaddr*)&laddr, sizeof(sockaddr))) {
        fmt::print("ERR: bind[INADDR_ANY:{}] failed: {}. \n", multicast_addr_.local_port, strerror(errno));
        return false;
    }
    fmt::print("bind[INADDR_ANY:{}] success. \n", multicast_addr_.local_port);

    // bind card.
    struct in_addr oaddr = {0};
    oaddr.s_addr = inet_addr(multicast_addr_.local_ip);
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&oaddr, sizeof(struct in_addr))) {
        fmt::print("ERR: setsockopt[IP_MULTICAST_IF] failed: {}. \n", strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketUDP::set_sockopt_multicast_addmembership() {
    struct ip_mreq group_addr;
    memset(&group_addr, 0, sizeof(ip_mreq));
    group_addr.imr_multiaddr.s_addr = inet_addr(multicast_addr_.group_ip);
    group_addr.imr_interface.s_addr = inet_addr(multicast_addr_.local_ip);
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group_addr, sizeof(group_addr))) {
        fmt::print("ERR: setsockopt[IP_ADD_MEMBERSHIP] failed: {}. \n", strerror(errno));
        return false;
    }
    fmt::print("setsockopt[IP_ADD_MEMBERSHIP] success. \n");
    return true;
}

bool UtilsSocketUDP::set_sockopt_multicast_ttl() {
    const int32_t ttl = 128;
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))) {
        fmt::print("ERR: setsockopt[IP_MULTICAST_TTL] failed: {}. \n", strerror(errno));
        return false;
    }
    fmt::print("setsockopt[IP_MULTICAST_TTL] ttl[128] success. \n");
    return true;
}

// loop = 0, not set.
bool UtilsSocketUDP::set_sockopt_multicast_loop(int32_t loop) {
    if (0 != setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, (void*)&loop, sizeof(loop))) {
        fmt::print("ERR: setsockopt[IP_MULTICAST_LOOP] failed: {}. \n", strerror(errno));
        return false;
    }
    fmt::print("setsockopt[IP_MULTICAST_LOOP] loop[{}] success. \n", loop);
    return true;
}

void UtilsSocketUDP::sendmsg_example() {
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
    addr.sin_addr.s_addr = inet_addr(multicast_addr_.group_ip);
    addr.sin_port        = htons(multicast_addr_.group_port);

    msghdr msg;
    msg.msg_name       = &addr; // dest addr
    msg.msg_namelen    = sizeof(addr);
    msg.msg_iov        = vec; // send buffers.
    msg.msg_iovlen     = sizeof(vec) / sizeof(vec[0]);
    msg.msg_control    = nullptr;
    msg.msg_controllen = 0;
    msg.msg_flags      = 0;

    const int32_t slen = sendmsg(sockfd_, &msg, MSG_DONTWAIT);
    fmt::print("send size [{}]. \n", slen);
}
