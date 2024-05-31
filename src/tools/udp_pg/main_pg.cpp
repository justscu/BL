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
    fmt::print("测量来回延时，需要 发送端 + 接收端. \n");
    fmt::print("test udp delay (ping-pong test: server --> client  --> server). \n");
    fmt::print("    ./udp_pg -ping local_ip dest_ip udp_size. \n");
    fmt::print("    ./udp_pg -pong hostname \n");

    exit(0);
}

static
int32_t udp_ping_pong(int32_t argc, char **argv) {
    if (argc < 3) { usage(); }

    signal(SIGINT, handle);

    const char *type = argv[1];

    UdpPG ping;

    if (0 == strcmp(type, "-ping")) {
        if (argc < 5) { usage(); }

        char      *lip = argv[2]; // local ip
        uint16_t lport = 10112;
        char      *dip = argv[3]; // dst ip
        uint16_t dport = 1577;
        int32_t   size = atoi(argv[4]);

        fmt::print(fg(fmt::rgb(250, 20, 36)) | fmt::emphasis::italic,
                "udp_pg: {} {}:{} -> {}:{} {}. \n", type, lip, lport, dip, dport, size);

        std::thread th(std::bind(&UdpPG::recv_udp, &ping, dport+1));
        th.detach();

        ping.send_udp(lip, lport, dip, dport, size);
    }
    else if (0 == strcmp(type, "-pong")) {
        if (argc < 3) { usage(); }

        char *hostname = argv[2];
        uint16_t  port = 1577;

        fmt::print(fg(fmt::rgb(250, 20, 36)) | fmt::emphasis::italic,
                "udp_pg: {} {}. \n", type, hostname, port);

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

