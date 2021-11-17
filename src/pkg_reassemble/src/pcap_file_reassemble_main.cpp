#include <iostream>
#include <thread>
#include <mutex>
#include "parse_libpcap_file.h"

std::mutex mutex;

void usage() {
    fprintf(stdout, "./pkg_reassemble name.pcap src_ip dst_ip src_port dst_port protocol(tcp|udp) \n");
    fprintf(stdout, "example:  ./pkg_reassemble xxx.pcap 10.25.22.10 252.1.5.2 9933 9966 tcp \n");
    fprintf(stdout, "example:  ./pkg_reassemble xxx.pcap 10.25.22.10 252.1.5.2 9933 9966 udp \n");
    exit(-1);
}

int32_t main(int32_t argc, char **argv) {
    if (argc < 6) { usage(); }

    const char *fname    = argv[1]; // pcap file name
    const char *src_ip   = argv[2];
    const char *dst_ip   = argv[3];
    const char *src_port = argv[4];
    const char *dst_port = argv[5];

    SrSwBuffer buf;
    if (!buf.init()) {
        return 0;
    }

    std::thread *rth = new std::thread(std::bind(parse_pcap_data, src_ip, dst_ip, src_port, dst_port, std::ref(buf)));
    read_libpcap_file(fname, std::ref(buf));

    rth->join();
    delete rth;

    return 0;
}
