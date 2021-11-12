#include <iostream>
#include <thread>
#include <mutex>
#include "parse_libpcap_file.h"

std::mutex mutex;

int32_t main(int32_t argc, char **argv) {
    fprintf(stdout, "./pkg_reorder \n");

    const char *pcap_file_name = argv[1];

    SrSwBuffer buf;
    if (!buf.init()) {
        return 0;
    }

    std::thread *rth = new std::thread(std::bind(parse_pcap_data, std::ref(buf)));
    read_libpcap_file(pcap_file_name, std::ref(buf));

    rth->join();
    delete rth;

    return 0;
}
