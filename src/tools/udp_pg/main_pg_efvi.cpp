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
#include "fmt/color.h"



void handle(int32_t s) {
    fmt::print("recv signal: {}. \n", s);
    exit(0);
}

void usage() {
    fmt::print("测量单向，需要 发送端 + 接收端. \n");
    fmt::print("./udp_pg_efvi -send hostname dest_ip udp_size \n");
    fmt::print("./udp_pg_efvi -recv hostname \n");

    exit(0);
}

static
int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);

    const char *type = argv[1];

    EfviUdpSend tx;
    EvfiUdpRecv rx;

    if (0 == strcmp(type, "-send")) {
        if (argc < 5) { usage(); }

        const char *hostname = argv[2];
        const char      *dip = argv[3];
        uint16_t dport = 1577;
        int32_t   size = atoi(argv[4]);

        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {} -> {}:{} {}. \n", type, hostname, dip, dport, size);

        tx.send(hostname, dip, dport, size);
    }
    else if (0 == strcmp(type, "-recv")) {
        if (argc < 3) { usage(); }

        const char *hostname = argv[2];
        uint16_t port = 1577;
        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
                "udp_pg_efvi: {} {}, port {}. \n", type, hostname, port);

        rx.recv(hostname, port);
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

