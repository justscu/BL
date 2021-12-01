#include <pcap.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include "reassemble/parse_libpcap_file.h"

std::mutex mutex;

void show_all_devs_ipv4() {
    char ebuf[PCAP_ERRBUF_SIZE] = {0};
    pcap_if_t *alldevs = nullptr;
    if (PCAP_ERROR == pcap_findalldevs(&alldevs, ebuf)) {
        log_err("%s. \n", ebuf);
        return;
    }

    for (pcap_if_t *dev = alldevs; dev != nullptr; dev = dev->next) {
        log_info("%s: %s \n", dev->name, dev->description);
        pcap_addr_t *add = dev->addresses;
        for (; add != nullptr; add = add->next) {
            if (add->addr->sa_family == AF_INET) {
                ((sockaddr_in*)(add->addr))->sin_addr.s_addr; // ip
                ((sockaddr_in*)(add->netmask))->sin_addr.s_addr; // mask

                char src[32] = {0};
                inet_ntop(AF_INET, &((sockaddr_in*)(add->addr))->sin_addr.s_addr, src, sizeof(src));
                log_info("        ip[%s] ", src);

                inet_ntop(AF_INET, &((sockaddr_in*)(add->netmask))->sin_addr.s_addr, src, sizeof(src));
                log_info("mask[%s] ", src);
                log_info("\n");
            }
        }
        log_info("\n");
    }

    pcap_freealldevs(alldevs);
}

void callback(u_char *userarg, const struct pcap_pkthdr *hdr, const u_char *pkg) {
    static_assert(sizeof(pcap_pkthdr) == sizeof(cap_hdr), "");

    // log_dbg("hdr.len[%u] hdr.caplen[%u] \n", hdr->len, hdr->caplen);
    SrSwBuffer *buf = (SrSwBuffer*)userarg;
    while (!buf->write((const cap_hdr*)hdr, (const char *)pkg)) {
        usleep(1);
    }
}

// capture packages from net-card.
void capture_thread(const char *device_name, const char *filter_str, SrSwBuffer &buf) {
    char ebuf[PCAP_ERRBUF_SIZE];
    pcap_t *handler = pcap_open_live(device_name, 65535, PCAP_OPENFLAG_PROMISCUOUS, 0, ebuf);
    if (!handler) {
        log_err("%s \n", ebuf);
        return;
    }

    struct bpf_program filter = {0};
    if (PCAP_ERROR == pcap_compile(handler, &filter, filter_str, 1, 0)) {
        log_err("%s. \n", pcap_geterr(handler));
        return ;
    }
    if (PCAP_ERROR == pcap_setfilter(handler, &filter)) {
        log_err("%s. \n", pcap_geterr(handler));
        return ;
    }

    pcap_loop(handler, -1, callback, (u_char*)&buf);
    pcap_close(handler);
}


void usage() {
    fprintf(stdout, "./live_pkgs device_name src_ip dst_ip src_port dst_port protocol(tcp|udp) \n");
    fprintf(stdout, "example:  ./live_pkgs eth1 10.25.22.10 252.1.5.2 9933 9966 tcp \n");
    fprintf(stdout, "example:  ./live_pkgs eth1 10.25.22.10 252.1.5.2 9933 9966 udp \n");
    exit(-1);
}


void tcp_data_ready_cbfunc(const char *src, const int32_t len) {
    std::ofstream ofs;
    ofs.open("/tmp/tcp_rb-pcap.txt", std::ios::app | std::ios::binary);
    ofs.write(src, len);
    ofs.flush();
}

int32_t main(int32_t argc, char **argv) {
    if (argc < 6) { usage(); }

    const char *device   = argv[1];
    const char *src_ip   = argv[2];
    const char *dst_ip   = argv[3];
    const char *src_port = argv[4];
    const char *dst_port = argv[5];

    char filter_str[256] = {0};
    snprintf(filter_str, sizeof(filter_str)-1,
            "src host %s and dst host %s and src port %s",
            src_ip, dst_ip, src_port);

    log_info("filter_str[%s] \n", filter_str);

    SrSwBuffer buf;
    if (!buf.init()) {
        return 0;
    }

    std::thread *rth = new std::thread(std::bind(capture_thread, device, filter_str, std::ref(buf)));

    /// parse.
    ParseLibpcapData s;
    if (!s.set_filter(src_ip, dst_ip, atoi(src_port), atoi(dst_port), "tcp", tcp_data_ready_cbfunc)) {
        exit(-1);
    }

    cap_hdr hd;
    const char *src = nullptr;
    while (true) {
        if (buf.read(&hd, src)) {
            s.parse(&hd, src);
        }
    }

    rth->join();
    delete rth;

    return 0;
}
