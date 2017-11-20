

//         udp多播
// （1）设置 SO_REUSEADDR 需要在bind之前，否则第二个程序绑定，会失败
// （2）server - 多网卡机器，需要设置组播的出口网卡地址，否则会走默认路由
// 
// （2）client - 多网卡的机器，需要bind到特定的网卡，否则可能会收不到数据
// （3）client - 在绑定时，必须使用 INADDR_ANY 或 group_ip，否则会收不到数据
// （4）在接收数据时，client的端口必须和server的端口一致，否则也会收不到数据

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void multicast_usage() {
    fprintf(stderr, "Usage: test group_ip group_port local_ip local_port \n");
    fprintf(stderr, "Usage: test group_ip group_port \n");
    fprintf(stderr, "\n");
}

// server
void multicast_server(int argc, char** argv) {
    if (argc < 4) {
        multicast_usage();
        exit(0);
    }

    //
    const char* group_ip = argv[1];
    const int16_t group_port = atoi(argv[2]);
    const char* local_ip = argv[3];
    int16_t local_port = atoi(argv[4]);

    // create socket.
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "create socket failed. err[%s]\n", strerror(errno));
        return ;
    }
    fprintf(stdout, "create socket success. \n");

    // set reuse
    int32_t reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) != 0) {
        fprintf(stderr, "setsockopt[SO_REUSEADDR] failed. err[%s] \n", strerror(errno));
        return;
    }
    fprintf(stdout, "setsockopt[SO_REUSEADDR] success. \n");

    //  bind
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port        = htons(local_port);
    int32_t ret = bind(sockfd, (struct sockaddr*)&local_addr, sizeof(sockaddr));
    if (ret != 0) {
        fprintf(stderr, "bind[INADDR_ANY:%d] failed. err[%s]\n", local_port, strerror(errno));
        return ;
    }
    fprintf(stdout, "bind[INADDR_ANY] success.\n");

    // 设置组播出口网卡地址，若不设置，会走默认路由
    struct in_addr addr = {0};
    addr.s_addr = inet_addr(local_ip);
    ret = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addr, sizeof(struct in_addr));
    if (ret == -1) {
        fprintf(stdout, "setsockopt[IP_MULTICAST_IF] failed. err[%s].\n", strerror(errno));
        return;
    }
    fprintf(stdout, "setsockopt[IP_MULTICAST_IF] success.\n");

    // send.
    struct sockaddr_in mcast_addr;
    memset(&mcast_addr, 0, sizeof(sockaddr_in));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = inet_addr(group_ip);
    mcast_addr.sin_port = htons(group_port);
    while (1) {
        static int32_t i = 100;
        if (i++ < 1000) i = 1000;
        if (i > 1024) i = 1000;
        char buf[1024];
        int32_t s = sendto(sockfd, buf, i, 0, (struct sockaddr*)&mcast_addr, sizeof(sockaddr_in));
        fprintf(stdout, "send len[%d] \n", s);
        sleep(1);
    }

    close(sockfd);
}


// client
void multicast_client(int argc, char** argv) {
    if (argc < 4) {
        multicast_usage();
        exit(0);
    }
    //
    const char* group_ip = argv[1];
    const int16_t group_port = atoi(argv[2]);
    const char* local_ip = argv[3];
    int16_t local_port = atoi(argv[4]);

    // create
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        fprintf(stderr, "create socket failed. err[%s]\n", strerror(errno));
        return ;
    }
    fprintf(stdout, "create socket success. \n");

    // set reuse
    int32_t reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) != 0) {
        fprintf(stderr, "setsockopt[SO_REUSEADDR] failed. err[%s] \n", strerror(errno));
        return;
    }
    fprintf(stdout, "setsockopt[SO_REUSEADDR] success. \n");

    // bind
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(group_ip); inet_addr(local_ip);
    local_addr.sin_port        = htons(local_port);
    int32_t ret = bind(sockfd, (struct sockaddr*)&local_addr, sizeof(sockaddr));
    if (ret != 0) {
        fprintf(stderr, "bind[INADDR_ANY:%d] failed. err[%s]\n", local_port, strerror(errno));
        return ;
    }
    fprintf(stdout, "bind[INADDR_ANY] success.\n");

    // add membership
    struct ip_mreq group_addr;
    memset(&group_addr, 0, sizeof(ip_mreq));
    group_addr.imr_multiaddr.s_addr = inet_addr(group_ip);
    group_addr.imr_interface.s_addr = inet_addr(local_ip);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group_addr, sizeof(group_addr)) != 0) {
        fprintf(stderr, "setsockopt[IP_ADD_MEMBERSHIP] failed[%s] \n", strerror(errno));
        return;
    }
    fprintf(stdout, "setsockopt[IP_ADD_MEMBERSHIP] success.\n");

    // 设置组播的网卡
    struct in_addr addr = {0};
    addr.s_addr = inet_addr(local_ip);
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addr, sizeof(struct in_addr)) != 0) {
        fprintf(stderr, "setsockopt[IP_MULTICAST_IF] failed[%s] \n", strerror(errno));
        return;
    }
    fprintf(stdout, "setsockopt[IP_MULTICAST_IF] success.\n");

    int32_t c = 0;
    while (true) {
        char buf[64*1024];
        int32_t rLen = read(sockfd, buf, sizeof(buf));
        if (rLen < 0) {
            fprintf(stderr, "read_%d. err[%s] \n", c++, strerror(errno));
            return;
        } else {
            fprintf(stderr, "read_%d. success. len[%d]. \n", c++, rLen);
        }
    }

    close(sockfd);
}

