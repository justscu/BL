#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

void UtilsSocketBase::close_socket() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool UtilsSocketBase::connect(const char *ip, const uint16_t port) {
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (-1 == ::connect(sockfd(), (struct sockaddr*)(&addr), sizeof(sockaddr_in))) {
        snprintf(err_, sizeof(err_)-1, "connect failed. [%s:%u] %s.", ip, port, strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketBase::connect(const char *str) {
    const char *cl = strrchr(str, ':');
    if (!cl) {
        snprintf(err_, sizeof(err_)-1, "param have no ':'");
        return false;
    }

    std::vector<std::string> vec;
    UtilsStr::split(str, ':', vec);
    UtilsStr::trim(vec, " ");

    if (vec.size() != 2) {
        snprintf(err_, sizeof(err_)-1, "param str must: 'ip:port'.");
        return false;
    }

    const int32_t port = atoi(vec[1].c_str());
    if (port < 0 || port > 65535) {
        snprintf(err_, sizeof(err_)-1, "port[%d] error.", port);
        return false;
    }

    return connect(vec[0].c_str(), (uint16_t)port);
}

bool UtilsSocketBase::bind(uint16_t port) {
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    const int32_t ret = ::bind(sockfd(), (struct sockaddr*)&addr, sizeof(sockaddr));
    if (ret != 0) {
        snprintf(err_, sizeof(err_)-1, "bind port[%u] failed. %s.", port, strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::bind(const char *ip, uint16_t port) {
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    const int32_t ret = ::bind(sockfd(), (struct sockaddr*)&addr, sizeof(sockaddr));
    if (ret != 0) {
        snprintf(err_, sizeof(err_)-1, "bind port[%s:%u] failed. %s.", ip, port, strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::listen(int32_t cnt) {
    const int32_t ret = ::listen(sockfd(), cnt);
    if (ret != 0) {
        snprintf(err_, sizeof(err_)-1, "listen failed. %s.", strerror(errno));
        return false;
    }
    return true;
}

int32_t UtilsSocketBase::accept() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);

    const int32_t fd  = ::accept(sockfd(), (sockaddr*)&addr, &len);
    if (fd > 0) {
        snprintf(err_, sizeof(err_)-1, "accept success: %s:%u fd[%d].", inet_ntoa(addr.sin_addr), addr.sin_port, fd);
    }
    else {
        if (errno != EAGAIN && errno != EINTR) {
            snprintf(err_, sizeof(err_)-1, "accept failed: %s.", strerror(errno));
        }
    }
    return fd;
}

bool UtilsSocketBase::set_sockopt_reuse_addr(bool use) {
    int32_t reuse = (use ? 1 : 0);
    if (0 != setsockopt(sockfd(), SOL_SOCKET, SO_REUSEADDR, (char*)(&reuse), sizeof(reuse))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_REUSEADDR] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_reuse_port(bool use) {
    int32_t reuse = (use ? 1 : 0);
    if (0 != setsockopt(sockfd(), SOL_SOCKET, SO_REUSEPORT, (char*)(&reuse), sizeof(reuse))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_REUSEPORT] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_sendbuf(const int32_t size) {
    int32_t gval = 0; // get value.
    int32_t  len = sizeof(gval);
    // set SO_SNDBUF
    if (0 != setsockopt(sockfd(), SOL_SOCKET, SO_SNDBUF, (void*)&size, len)) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_SNDBUF] failed: %s.", strerror(errno));
        return false;
    }

    if (0 != getsockopt(sockfd(), SOL_SOCKET, SO_SNDBUF, (void*)&gval, (unsigned int *)&len)) {
        snprintf(err_, sizeof(err_)-1, "getsockopt[SO_SNDBUF] failed: %s.", strerror(errno));
        return false;
    }

    const bool ret = (size == gval);
    snprintf(err_, sizeof(err_)-1, "setsockopt[SO_SNDBUF] %s, size[%d M].", ret ? "success" : "failed", size/1024/1024);
    return ret;
}

bool UtilsSocketBase::set_sockopt_recvbuf(const int32_t size) {
    int32_t gval = 0; // get value.
    int32_t  len = sizeof(gval);
    // set SO_RCVBUF
    if (0 != setsockopt(sockfd(), SOL_SOCKET, SO_RCVBUF, (void*)&size, len)) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_RCVBUF] failed: %s.", strerror(errno));
        return false;
    }

    if (0 != getsockopt(sockfd(), SOL_SOCKET, SO_RCVBUF, (void*)&gval, (unsigned int *)&len)) {
        snprintf(err_, sizeof(err_)-1, "getsockopt[SO_RCVBUF] failed: %s.", strerror(errno));
        return false;
    }

    const bool ret = (size == gval);
    snprintf(err_, sizeof(err_)-1, "setsockopt[SO_RCVBUF] %s: size[%d M].", ret ? "success" : "failed", size/1024/1024);
    return ret;
}

bool UtilsSocketBase::set_sockopt_nonblocking(bool value) {
    int32_t flag = fcntl(sockfd(), F_GETFL, 0);
    if (flag == -1) {
        snprintf(err_, sizeof(err_)-1, "fcntl[F_GETFL] failed: %s.", strerror(errno));
        return false;
    }
    flag = value ? (flag | O_NONBLOCK) : (flag & ~O_NONBLOCK);

    const int32_t ret = fcntl(sockfd(), F_SETFL, flag);
    if (ret != 0) {
        snprintf(err_, sizeof(err_)-1, "fcntl[F_SETFL] failed: %s.", strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketBase::set_sockopt_keepalive(bool use) {
    int32_t alive = (use ? 1 : 0);
    int32_t ret = setsockopt(sockfd(), SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_KEEPALIVE] failed: %s.", strerror(errno));
        return false;
    }

    // 当tcp空闲30秒后，开始检测，发送keepalive数据
    int32_t idletime = 60;
    ret = setsockopt(sockfd(), SOL_TCP, TCP_KEEPIDLE, &idletime, sizeof(idletime));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[TCP_KEEPIDLE] failed: %s.", strerror(errno));
        return false;
    }

    // 每隔20S,发一次keepalive数据
    int32_t interval = 20;
    ret = setsockopt(sockfd(), SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[TCP_KEEPINTVL] failed: %s.", strerror(errno));
        return false;
    }

    // 一共发10个keepalive数据
    int32_t cnt = 10;
    ret = setsockopt(sockfd(), SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[TCP_KEEPCNT] failed: %s.", strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketBase::set_sockopt_nodelay(bool use) {
    int32_t enable = (use ? 1 : 0);
    int32_t ret = setsockopt(sockfd(), IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[TCP_NODELAY] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_busypoll(bool use) {
    int32_t enable = (use ? 1 : 0);
    int32_t ret = setsockopt(sockfd(), SOL_SOCKET, SO_BUSY_POLL, (void*)&enable, sizeof(enable));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_BUSY_POLL] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_timestampns(bool use) {
    int32_t enable = (use ? 1 : 0);
    int32_t ret = setsockopt(sockfd(), SOL_SOCKET, SO_TIMESTAMPNS, (void*)&enable, sizeof(enable));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_TIMESTAMPNS] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_recvtimeout(const struct timeval &tv) {
    int32_t ret = setsockopt(sockfd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_RCVTIMEO] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_pkginfo(bool use) {
    int32_t v = (use ? 1 : 0);
    int32_t ret = setsockopt(sockfd(), SOL_IP, IP_PKTINFO, &v, sizeof(v));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_PKTINFO] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketBase::set_sockopt_bindtodev(const char *eth) {
    struct ifreq  nic;
    memset(&nic, 0, sizeof(struct ifreq));
    strncpy(nic.ifr_name, eth, sizeof(nic.ifr_name));

    int32_t ret = setsockopt(sockfd(), SOL_SOCKET, SO_BINDTODEVICE, &nic, sizeof(nic));
    if (ret == -1) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[SO_BINDTODEVICE] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketTcp::create_socket() {
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        snprintf(err_, sizeof(err_)-1, "create socket failed: %s.", strerror(errno));
        return false;
    }

    return true;
}


bool UtilsSocketUdp::create_socket() {
    if ((fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0) {
        snprintf(err_, sizeof(err_)-1, "create socket failed: %s.", strerror(errno));
        return false;
    }

    return true;
}


void UtilsSocketMulticast::set_multicast_addr(const MultiCastAddr &addr) {
    memcpy(&multicast_addr_, &addr, sizeof(MultiCastAddr));
}

bool UtilsSocketMulticast::bind_socket_multicast() {
    struct sockaddr_in laddr;
    memset(&laddr, 0, sizeof(struct sockaddr_in));
    laddr.sin_family      = AF_INET;
    laddr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(udp_group_.local_ip);
    laddr.sin_port        = htons(multicast_addr_.local_port);

    if (0 != ::bind(sockfd(), (struct sockaddr*)&laddr, sizeof(sockaddr))) {
        snprintf(err_, sizeof(err_)-1, "bind[INADDR_ANY:%u] failed: %s.", multicast_addr_.local_port, strerror(errno));
        return false;
    }

    // bind card.
    struct in_addr oaddr = {0};
    oaddr.s_addr = inet_addr(multicast_addr_.local_ip);
    if (0 != setsockopt(sockfd(), IPPROTO_IP, IP_MULTICAST_IF, (char*)&oaddr, sizeof(struct in_addr))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_MULTICAST_IF] failed: %s.", strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketMulticast::set_sockopt_multicast_addmembership() {
    struct ip_mreq group_addr;
    memset(&group_addr, 0, sizeof(ip_mreq));
    group_addr.imr_multiaddr.s_addr = inet_addr(multicast_addr_.group_ip);
    group_addr.imr_interface.s_addr = inet_addr(multicast_addr_.local_ip);
    if (0 != setsockopt(sockfd(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group_addr, sizeof(group_addr))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_ADD_MEMBERSHIP] failed: %s.", strerror(errno));
        return false;
    }

    return true;
}

bool UtilsSocketMulticast::get_sockopt_multicast_ttl() {
    int32_t   ttl = 0;
    uint32_t size = sizeof(ttl);
    if (0 != getsockopt(sockfd(), IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, &size)) {
        snprintf(err_, sizeof(err_)-1, "getsockopt[IP_MULTICAST_TTL] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

bool UtilsSocketMulticast::set_sockopt_multicast_ttl() {
    const int32_t ttl = 16;
    if (0 != setsockopt(sockfd(), IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_MULTICAST_TTL] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

// loop = 0, not set.
bool UtilsSocketMulticast::set_sockopt_multicast_loop(int32_t loop) {
    if (0 != setsockopt(sockfd(), IPPROTO_IP, IP_MULTICAST_LOOP, (void*)&loop, sizeof(loop))) {
        snprintf(err_, sizeof(err_)-1, "setsockopt[IP_MULTICAST_LOOP] failed: %s.", strerror(errno));
        return false;
    }
    return true;
}

int32_t UtilsSocketMulticast::sendmsg_example() {
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

    const int32_t slen = sendmsg(sockfd(), &msg, MSG_DONTWAIT);
    // fmt::print("send size [{}]. \n", slen);
    return slen;
}
