#include <iostream>
#include "parse_libpcap_file.h"

int32_t main(int32_t argc, char **argv) {
    fprintf(stdout, "./pkg_reorder \n");

    const char *pcap_file_name = argv[1];

    ParseLibpcapFile pcap;
    if (pcap.init("10.25.24.41", "10.25.26.219", 9933, 0, "tcp")) {
        pcap.read_file(pcap_file_name);
    }

    return 0;
}
