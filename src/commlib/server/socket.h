#pragma once

#include <stdint.h>

class SocketOps {
public:
    static int32_t create_tcp_listen_socket(uint16_t listen_port);
    static int32_t connect_tcp_server(const char *op, uint16_t port);
    static int32_t accept_new_client(int32_t listen_fd);

    static int32_t readn(int32_t fd, char *buf, int32_t n);
    static int32_t writen(int32_t fd, const char *buf, int32_t n);

    static bool set_reuseaddr(int32_t fd);
    static bool set_blocking(int32_t fd, bool block);
    static bool set_keepalive(int32_t fd);
};
