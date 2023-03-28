#pragma once

#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocket {
public:
    UtilsSocket()  { }
    ~UtilsSocket() { close_socket(); }
    UtilsSocket(const UtilsSocket&) = delete;
    void operator=(const UtilsSocket&) = delete;

public:
    bool create_socket_ipv4(bool btcp);
    void close_socket();
    int32_t sockfd() const { return sockfd_; }
    const char* err_str() const { return err_; }

public:
    bool connect(const char *ip, const uint16_t port);
    bool connect(const char *ipport); // str: "127.0.0.1:6618"

    bool bind(uint16_t port);
    bool listen(int32_t cnt);
    int32_t accept();

public:
    bool set_sockopt_reuse_addr(bool use);
    bool set_sockopt_reuse_port(bool use);
    bool set_sockopt_sendbuf(const int32_t size); // size = 8*1024*1024; // 8M
    bool set_sockopt_recvbuf(const int32_t size); // size =16*1024*1024; // 16M
    bool set_sockopt_nonblocking(bool value);
    bool set_sockopt_keepalive(bool use);
    bool set_sockopt_nodelay(bool use);
    bool set_sockopt_busypoll(bool use);
    bool set_sockopt_timestampns(bool use);
    bool set_sockopt_recvtimeout(const struct timeval &tv);
    bool set_sockopt_pkginfo(bool use);

protected:
    int32_t sockfd_ = -1;
    char  err_[256] = {0};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocketMulticast : public UtilsSocket {
public:
    struct MultiCastAddr {
        char     group_ip[32] = {0};
        uint16_t group_port   =  0;
        char     local_ip[32] = {0};
        uint16_t local_port   = 0;
    };

public:
    void set_multicast_addr(const MultiCastAddr &addr);
    bool bind_socket_multicast();

    bool set_sockopt_multicast_addmembership();
    bool get_sockopt_multicast_ttl();
    bool set_sockopt_multicast_ttl();
    bool set_sockopt_multicast_loop(int32_t loop); // loop = 0, not set.

public:
    void sendmsg_example();

protected:
    MultiCastAddr multicast_addr_;
};

