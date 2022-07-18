#pragma once

#include <stdint.h>

class UtilsSocket {
public:
    struct MultiCastAddr {
        char     group_ip[32] = {0};
        uint16_t group_port   =  0;
        char     local_ip[32] = {0};
        uint16_t local_port   = 0;
    };

public:
    UtilsSocket()  { }
    ~UtilsSocket() { }
    UtilsSocket(const UtilsSocket&) = delete;
    void operator=(const UtilsSocket&) = delete;

public:
    bool create_socket_ipv4(bool btcp);
    void close_socket();
    int32_t sockfd() const { return sockfd_; }

    bool set_sockopt_sendbuf(const int32_t size); // size = 8*1024*1024; // 8M
    bool set_sockopt_recvbuf(const int32_t size); // size =16*1024*1024; // 16M
    bool set_sockopt_nonblocking(bool value);

public:
    void set_multicast_addr(const MultiCastAddr &addr);
    bool bind_socket_multicast();
    bool set_sockopt_multicast_addmembership();
    bool set_sockopt_multicast_ttl();
    bool set_sockopt_multicast_loop(int32_t loop); // loop = 0, not set.

public:
    void sendmsg_example();

protected:
    int32_t               sockfd_ = -1;
    MultiCastAddr multicast_addr_;
};

