#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <new>
#include "parse_libpcap_file.h"

#include "parse_l1_layer.h"


bool ParseLibpcapFile::init(const char *src_ip, const char *dst_ip,
                            uint16_t src_port, uint16_t dst_port,
                            uint8_t protocol) {
    ip_parser_ = new (std::nothrow) ParseIPLayer;
    if (!ip_parser_ || !ip_parser_->init(src_ip, dst_ip, src_port, dst_port, protocol)) {
        return false;
    }

    mac_parser_ = new (std::nothrow) ParseEthLayer(ip_parser_);
    if (!mac_parser_) {
        return false;
    }

    buf_ = new (std::nothrow) char[buf_size];
    return (buf_ != nullptr);
}

void ParseLibpcapFile::read_file(const char *fname) {
    FILE *p = fopen(fname, "rb");
    if (!p) {
        fprintf(stdout, "fopen [%s] failed, %s. \n", fname, strerror(errno));
        return ;
    }

    fprintf(stdout, "fopen [%s] success. \n", fname);

    int32_t len = fread(buf_, sizeof(char), sizeof(pcap_hdr_t), p);
    if (get_pcap_file_header(buf_, len) < 0) {
        fprintf(stdout, "read get_pcap_file_header failed. \n");
        return;
    }

    int64_t tlen = len;
    while (!feof(p)) {
        len = fread(buf_, sizeof(char), buf_size, p);
        if (len > 0) {
            int32_t ret = get_pcap_package(buf_, len);
            if (ret <= 0) {
                break;
            }
            tlen += ret;
            fseek(p, tlen, SEEK_SET);
        }
    }

    fclose(p);
    fprintf(stdout, "file total length [%ld]. \n", tlen);
}

int32_t ParseLibpcapFile::get_pcap_package(const char *str, const int32_t len) {
    const char *beg = str;
    const char *end = str + len;

    while (beg < end) {
        if (beg + sizeof(PcapPkgHdr) > end) {
            break;
        }

        const PcapPkgHdr *hdr = (const PcapPkgHdr*)beg;
        const int32_t c = sizeof(PcapPkgHdr) + hdr->cap_len;
        if (c + beg > end) {
            break;
        }
        beg += sizeof(PcapPkgHdr);
        parse(hdr, beg);
        beg += hdr->cap_len;
    }

    return (beg - str);
}

int32_t ParseLibpcapFile::get_pcap_file_header(const char *str, int32_t len) {
    static_assert(sizeof(pcap_hdr_t) == 24, "");
    if (len == sizeof(pcap_hdr_t)) {
        memcpy(&pcap_file_head_, str, len);
        if (pcap_file_head_.magic != PCAP_HDR_MAGIC) {
            fprintf(stdout, "pcap_file_head.magic_number error. \n");
            return -1;
        }

        if (pcap_file_head_.link_layer_type != 0x01) {
            fprintf(stdout, "pcap_file_head.link_layer_type error. \n");
            return -1;
        }

        return len;
    }
    return -1;
}

void ParseLibpcapFile::parse(const PcapPkgHdr *hdr, const char *pkg) {
    fprintf(stdout, "%5u %u.%06u %4d, %4d  ",
            ++idx_, hdr->gmt_time_sec, hdr->micro_time_usec, hdr->cap_len, hdr->pkg_len);

    if (hdr->cap_len == hdr->pkg_len) {
        mac_parser_->parse(pkg, hdr->cap_len);
    }

    fprintf(stdout, "\n");
}

