#include <csignal>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <unistd.h>
#include "udp_raw.h"
#include "udp_pg.h"
#include "udp_pg_efvi.h"
#include "fmt/format.h"



void handle(int32_t s) {
    fmt::print("recv signal: {}. \n", s);
    exit(0);
}

void usage() {
    fmt::print("./udp_pg -ping dest_ip dest_port udp_size \n");
    fmt::print("./udp_pg -pong listen_port \n");
}

static
int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) {
        usage();
        return 0;
    }

    signal(SIGINT, handle);

    const char *type = argv[1];

    UdpPG ping;

    if (0 == strcmp(type, "-ping")) {
        char      *dip = argv[2];
        uint16_t dport = atoi(argv[3]);
        int32_t   size = atoi(argv[4]);

        fmt::print("udp_pg: {} {}:{} {}. \n", type, dip, dport, size);

        std::thread th(std::bind(&UdpPG::recv_udp, &ping, dport+1));
        ping.send_udp(dip, dport, size);

        th.join();
    }
    else if (0 == strcmp(type, "-pong")) {
        uint16_t port = atoi(argv[2]);

        fmt::print("udp_pg: {} {}. \n", type, port);

        ping.udp_pong(port);
    }
    else {
        usage();
    }

    return 0;
}


int32_t main(int32_t argc, char **argv) {
    return udp_ping_pong(argc, argv);
}

