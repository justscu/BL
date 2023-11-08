#pragma once

#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UtilsSocketBase
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocketBase {
public:
    UtilsSocketBase() = default;
    ~UtilsSocketBase() { close_socket(); }

public:
    void close_socket();
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

    int32_t sockfd() const { return fd_; }
     const char* err_str() const { return err_; }

protected:
    char err_[256];
    int32_t fd_ = -1;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UtilsSockTcp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocketTcp : public UtilsSocketBase {
public:
    UtilsSocketTcp() = default;

public:
    bool create_socket();
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UtilsSocketUdp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocketUdp : public UtilsSocketBase {
public:
    UtilsSocketUdp() = default;

public:
    bool create_socket();
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UtilsSocketMulticast
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsSocketMulticast : public UtilsSocketUdp {
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

private:
    MultiCastAddr multicast_addr_;
};
