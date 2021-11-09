#include <iostream>
#include "parse_libpcap_file.h"

int32_t main(int32_t argc, char **argv) {
    fprintf(stdout, "./pkg_reorder \n");

    const char *pcap_file_name = argv[1];

    ParseLibpcapFile pcap;
    if (pcap.init("10.68.201.155", "10.68.201.154", 9129, 0, "tcp")) {
        pcap.read_file(pcap_file_name);
    }

    return 0;
}
