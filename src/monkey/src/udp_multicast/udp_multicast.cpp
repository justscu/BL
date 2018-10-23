

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
#include <errno.h>
#include <string.h>
#include <unistd.h>


void multicast_usage() {
    fprintf(stdout, "2018-07-17\n");
    fprintf(stdout, "Usage: test c|s group_ip group_port local_ip local_port usleepTime \n");
    fprintf(stdout, "usleepTime: 指每发送100个包，usleep的时间 \n");
    fprintf(stderr, "\n");
}

// server
void multicast_server(int argc, char** argv) {
    if (argc < 5) {
        multicast_usage();
        exit(0);
    }

    //
    const char* group_ip = argv[2];
    const int16_t group_port = atoi(argv[3]);
    const char* local_ip = argv[4];
    int16_t local_port = atoi(argv[5]);
    int32_t usleeptime = 0;
    if (argc == 7) { usleeptime = atoi(argv[6]); }

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

    // SO_SNDBUF
    {
        // get default SO_SNDBUF
        int32_t val = 0;
        int32_t val_len = sizeof(val);
        ret = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&val, (unsigned int *)&val_len);
        if (ret == -1) {
            fprintf(stdout, "getsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "getsockopt[SO_SNDBUF] default bufsize[%d] \n", val);
        // set SO_SNDBUF
        val = 16*1024*1024;
        ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&val, val_len);
        if (ret == -1) {
            fprintf(stdout, "setsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "setsockopt[SO_SNDBUF] bufsize[16M] success \n");
        // get SO_SNDBUF again
        ret = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&val, (unsigned int *)&val_len);
        if (ret == -1) {
            fprintf(stdout, "getsockopt[SO_SNDBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "getsockopt[SO_SNDBUF] bufsize[%d] \n", val);
        // set ttl
        int32_t ttl = 128;
        ret = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
        if (ret == -1) {
            fprintf(stdout, "setsockopt[IP_MULTICAST_TTL] failed. err[%s]. \n", strerror(errno));
            return;
        }
        fprintf(stdout, "setsockopt[IP_MULTICAST_TTL] 128 \n");
    }

    // send.
    struct sockaddr_in mcast_addr;
    memset(&mcast_addr, 0, sizeof(sockaddr_in));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = inet_addr(group_ip);
    mcast_addr.sin_port = htons(group_port);

    const int32_t buf_len = 64*1024-50;
    char * buf = new char[buf_len];
    uint64_t i = 1;
    while (true) {
        snprintf(buf, buf_len, "%llu", i);
        int32_t s = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&mcast_addr, sizeof(sockaddr_in));
        if (s == -1) {
            fprintf(stderr, "sendto error[%s] \n", strerror(errno));
            break;
        }
        ++i;
        fprintf(stdout, "%llu send len[%d] \n", i, s);
        if (usleeptime > 0 && i % 100 == 0) {
            usleep(usleeptime);
        }
    }

    close(sockfd);
}


// client
void multicast_client(int argc, char** argv) {
    if (argc < 5) {
        multicast_usage();
        exit(0);
    }
    //
    const char* group_ip = argv[2];
    const int16_t group_port = atoi(argv[3]);
    const char* local_ip = argv[4];
    int16_t local_port = atoi(argv[5]);

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

    // SO_RCVBUF
    {
        // get default SO_RCVBUF
        int32_t val = 0;
        int32_t val_len = sizeof(val);
        int32_t ret = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&val, (unsigned int *)&val_len);
        if (ret == -1) {
            fprintf(stdout, "getsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "getsockopt[SO_RCVBUF] default bufsize[%d] \n", val);
        // set SO_RCVBUF
        val = 16*1024*1024;
        ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&val, val_len);
        if (ret == -1) {
            fprintf(stdout, "setsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "setsockopt[SO_RCVBUF] bufsize[16M] success \n");
        // get SO_RCVBUF again
        ret = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&val, (unsigned int *)&val_len);
        if (ret == -1) {
            fprintf(stdout, "getsockopt[SO_RCVBUF] failed. err[%s].\n", strerror(errno));
            return;
        }
        fprintf(stdout, "getsockopt[SO_RCVBUF] bufsize[%d] \n", val);
    }

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

    int32_t c = 1;
    while (true) {
        char buf[64*1024];
        int32_t rLen = read(sockfd, buf, sizeof(buf));
        if (rLen < 0) {
            fprintf(stderr, "read_%d. err[%s] \n", c++, strerror(errno));
            return;
        } else {
            fprintf(stderr, "read_%d. buf[%s] success. len[%d]. \n", c++, buf, rLen);
        }
    }

    close(sockfd);
}

