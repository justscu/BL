#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "socket.h"
#include "log.h"


// 返回值大于0，表示创建listen socket成功
int32_t SocketOps::create_tcp_listen_socket(uint16_t listen_port) {
    if (listen_port == 0  || listen_port >= 65536) {
        log_err("listen_port should between (0, 65535).");
        return -1;
    }

    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        log_err("create socket failed. [%s]", strerror(errno));
        return -1;
    }

    if (!set_reuseaddr(fd)) {
        close(fd);
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(listen_port);

    if (0 != bind(fd, (struct sockaddr*)(&addr), sizeof(sockaddr_in))) {
        log_err("bind failed. [%s]", strerror(errno));
        close(fd);
        return -1;
    }

    if (0 != listen(fd, 16)) {
        log_err("listen failed. [%s]", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int32_t SocketOps::connect_tcp_server(const char *ip, uint16_t port) {
    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        log_err("create socket failed. [%s]", strerror(errno));
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (1 != inet_pton(AF_INET, ip, &addr.sin_addr)) {
        log_err("inet_pton failed. [%s:%u] [%s]", ip, port, strerror(errno));
        close(fd);
        return -1;
    }

    if (0 != connect(fd, (struct sockaddr*)(&addr), sizeof(sockaddr_in))) {
        log_err("connect failed. [%s:%u] [%s]", ip, port, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int32_t SocketOps::accept_new_client(int32_t listen_fd) {
    struct sockaddr_in addr;
    socklen_t          addr_len = sizeof(addr);
    int32_t clientfd = accept(listen_fd, (struct sockaddr*)(&addr), &addr_len);
    if (-1 == clientfd) {
      log_err("accept error: [%s]", strerror(errno));
      return -1;
    }

    // print
    char buf[16];
    if (!inet_ntop(AF_INET, &(addr.sin_addr.s_addr), buf, sizeof(buf))) {
        log_err("inet_ntop [AF_INET] failed. [%s]", strerror(errno));
    }
    else {
        log_info("accept : [%s:%u] success.", buf, addr.sin_port);
    }

    return clientfd;
}

// 返回实际读出的长度
// 若返回值-1， 表示socket失效；需要关闭socket.
// 若返回值为0，可能是INTR
int32_t SocketOps::readn(int32_t fd, char *buf, int32_t n) {
    char *dst = buf;
    int32_t remain = n;
    while (remain > 0) {
        const int32_t rlen = ::recv(fd, dst, remain, MSG_DONTWAIT);
        // peer shutdown.
        if (rlen == 0) {
            break;
        }
        else if (rlen < 0) {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN) {
                break;
            }
            return -1; // ERROR.
        }

        remain -= rlen;
        dst    += rlen;
    } // while

    return (dst - buf);
}

// 返回实际发送的长度
// 返回值-1，表示socket失效；需要关闭socket.
int32_t SocketOps::writen(int32_t fd, const char *buf, int32_t n) {
    const char *src = buf;
    int32_t remain = n;
    while (remain > 0) {
        const int32_t slen = ::send(fd, src, remain, MSG_DONTWAIT);
        if (slen == -1) {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN) {
                break;
            }
            return -1; // send failed.
        }

        remain -= slen;
        src    += slen;
    } // while
    return (src - buf);
}

bool SocketOps::set_reuseaddr(int32_t fd) {
    int32_t reuse = 1;
    int32_t ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret == -1) {
        log_err("setsockopt [SO_REUSEADDR] failed. [%s]", strerror(errno));
        return false;
    }
    return true;
}

bool SocketOps::set_blocking(int32_t fd, bool blocking) {
    int32_t flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        log_err("fcntl [F_GETFL] failed. [%s]", strerror(errno));
        return false;
    }

    if (blocking) {
        flags &= (~O_NONBLOCK);
    }
    else {
        flags |= O_NONBLOCK;
    }

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        log_err("fcntl [F_SETFL] failed. [%s]", strerror(errno));
        return false;
    }

    return true;
}

bool SocketOps::set_keepalive(int32_t fd) {
    bool ret = 0;

    int32_t alive = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive));
    if (ret == -1) {
        log_err("setsockopt [SO_KEEPALIVE] failed. [%s]", strerror(errno));
        return false;
    }

    // 当tcp空闲30秒后，开始检测，发送keepalive数据
    int32_t idletime = 60;
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &idletime, sizeof(idletime));
    if (ret == -1) {
        log_err("setsockopt [TCP_KEEPIDLE] failed. [%s]", strerror(errno));
        return false;
    }

    // 每隔20S,发一次keepalive数据
    int32_t interval = 20;
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    if (ret == -1) {
        log_err("setsockopt [TCP_KEEPINTVL] failed. [%s]", strerror(errno));
        return false;
    }

    // 一共发10个keepalive数据
    int32_t cnt = 10;
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    if (ret == -1) {
        log_err("setsockopt [TCP_KEEPCNT] failed. [%s]", strerror(errno));
        return false;
    }

    return true;
}
