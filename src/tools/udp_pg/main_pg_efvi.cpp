#include <csignal>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <unistd.h>
#include <functional>
#include "udp_raw.h"
#include "udp_pg.h"
#include "udp_pg_efvi.h"
#include "fmt/format.h"



void handle(int32_t s) {
    fmt::print("recv signal: {}. \n", s);
    exit(0);
}

void usage() {
    fmt::print("./udp_pg_efvi -ping hostname dest_ip dest_port udp_size \n");
}

static
int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) {
        usage();
        return 0;
    }

    signal(SIGINT, handle);

    const char *type = argv[1];

    EfviUdpSend tx;
    EvfiUdpRecv rx;

    if (0 == strcmp(type, "-ping")) {
        const char *hostname = argv[2];
        const char      *dip = argv[3];
        uint16_t dport = atoi(argv[4]);
        int32_t   size = atoi(argv[5]);

        fmt::print("udp_pg_efvi: {} {} {}:{} {}. \n", type, hostname, dip, dport, size);

        std::thread th(std::bind(&EfviUdpSend::send, &tx, hostname,  dip, dport, size));

        sleep(1);
        rx.recv(hostname, dport+1);

        //

        th.join();
    }
    else {
        usage();
    }

    return 0;
}


int32_t main(int32_t argc, char **argv) {
    udp_ping_pong(argc, argv);
    return 0;
}

