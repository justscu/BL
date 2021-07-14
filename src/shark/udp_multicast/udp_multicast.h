#pragma once

#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct UdpGroup {
    char     group_ip[32];
    uint16_t group_port;
    char     local_ip[32];
    uint16_t local_port;
};

class UdpMulticast {
public:
    void test_server(uint32_t sleep_ms = 1);
    void test_client();

    void set_udpgroup(const UdpGroup &v);

    bool create_udp_socket();
    bool bind_udp_socket();
    bool set_sockopt_addmembership();
    // set send & recv buffer size
    bool set_sockopt_sendbuf();
    bool set_sockopt_recvbuf();
    bool set_sockopt_ttl();

    // 设置loop属性，若不设置，本机的环回收不到组播
    // loop = 0, 不设置
    bool set_sockopt_loop(int32_t loop=0);

    void close_socket();

    // 多缓冲区同时发送的例子
    void sendmsg_example();

private:
    UdpGroup udp_group_;
    int32_t     sockfd_ = -1;
};
