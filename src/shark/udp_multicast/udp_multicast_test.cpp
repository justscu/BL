#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "udp_multicast.h"

void udp_server(const UdpGroup &g, int32_t sleep_ms) {
    UdpMulticast s;
    s.set_udpgroup(g);

    if (s.create_udp_socket() && s.bind_udp_socket()
            && s.set_sockopt_sendbuf() && s.set_sockopt_ttl()) {
        s.test_server(sleep_ms);
    }

    s.close_socket();
}

void udp_client(const UdpGroup &g) {
    UdpMulticast s;
    s.set_udpgroup(g);

    if (s.create_udp_socket() && s.set_sockopt_recvbuf()
            && s.bind_udp_socket() && s.set_sockopt_addmembership()) {
        s.test_client();
    }

    s.close_socket();
}


void mulicast_test(int32_t argc, char **argv) {
    if (argc < 5) {
        fprintf(stdout, "2031-05-10\n");
        fprintf(stdout, "Usage: multicast_test c|s group_ip group_port local_ip local_port sleep(ms) \n");
        fprintf(stdout, "       sleep: 指每发送100个包，sleep 多少 ms. \n");
        fprintf(stderr, "\n");
        exit(0);
    }

    UdpGroup g;
    memset(&g, 0, sizeof(g));
    strcpy(g.group_ip, argv[2]);
    g.group_port = atoi(argv[3]);
    strcpy(g.local_ip, argv[4]);
    g.local_port = atoi(argv[5]);

    int32_t s = 1;
    if (argc > 6) { s = atoi(argv[6]); }

    if (argv[1][0] == 's' || argv[1][0] == 'S') {
        return udp_server(g, s);
    }

    if (argv[1][0] == 'c' || argv[1][0] == 'C') {
        return udp_client(g);
    }
}
