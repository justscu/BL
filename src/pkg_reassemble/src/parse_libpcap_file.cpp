#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <new>
#include "define.h"
#include "parse_libpcap_file.h"
#include "parse_l1_layer.h"


bool SpliteLibpcapFile::init() {
    buf_ = new (std::nothrow) char[buf_size];
    return (buf_ != nullptr);
}

void SpliteLibpcapFile::read_file(const char *fname) {
    FILE *p = fopen(fname, "rb");
    if (!p) {
        log_err("fopen [%s] failed, %s. \n", fname, strerror(errno));
        return ;
    }

    log_info("fopen [%s] success. \n", fname);

    int32_t len = fread(buf_, sizeof(char), sizeof(pcap_hdr_t), p);
    if (get_pcap_file_header(buf_, len) < 0) {
        log_err("read get_pcap_file_header failed. \n");
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
    log_dbg(">>>>> file[%s]: read total length [%ld]. \n", fname, tlen);
}

int32_t SpliteLibpcapFile::get_pcap_package(const char *str, const int32_t len) {
    const char *beg = str;
    const char *end = str + len;

    while (beg < end) {
        if (beg + sizeof(PcapPkgHdr) > end) {
            break;
        }

        const PcapPkgHdr *hdr = (const PcapPkgHdr*)beg;
        const int32_t c = sizeof(PcapPkgHdr) + hdr->cap_len;

        assert(hdr->cap_len > 0);

        if (c + beg > end) {
            break;
        }
        while (!pcapbuf_.write(beg, c)) {
            // if buffer full, sleep 1 u-second.
            usleep(1);
        }
        beg += c;
    }

    return (beg - str);
}

int32_t SpliteLibpcapFile::get_pcap_file_header(const char *str, int32_t len) {
    static_assert(sizeof(pcap_hdr_t) == 24, "");
    if (len == sizeof(pcap_hdr_t)) {
        memcpy(&pcap_file_head_, str, len);
        if (pcap_file_head_.magic != PCAP_HEAD_MAGIC) {
            log_err("pcap_file_head.magic_number error. \n");
            return -1;
        }

        if (pcap_file_head_.link_layer_type != 0x01) {
            log_err("pcap_file_head.link_layer_type error. \n");
            return -1;
        }

        return len;
    }
    return -1;
}


bool ParseLibpcapData::set_filter(const char *src_ip, const char *dst_ip,
                                  uint16_t src_port, uint16_t dst_port,
                                  const char *protocol) {
    ip_parser_ = new (std::nothrow) ParseIPLayer;
    if (!ip_parser_) {
    	log_err("new ParseIPLayer failed.");
    	return false;
    }

    ip_parser_->set_ip_filter(src_ip, dst_ip);
    ip_parser_->set_protocol_filter(protocol);
    ip_parser_->set_port_filter(src_port, dst_port);
    if (!ip_parser_->create_l3_layer()) {
    	return false;
    }

    mac_parser_ = new (std::nothrow) ParseEthLayer(ip_parser_);
    if (!mac_parser_) {
    	log_err("new ParseEthLayer failed.");
        return false;
    }

    return true;
}

void ParseLibpcapData::parse(const PcapPkgHdr *hdr, const char *eth_pkg) {
    log_dbg("%5u %u.%06u %5d, %5d  ",
            ++idx_, hdr->ct.sec, hdr->ct.usec, hdr->cap_len, hdr->pkg_len);

    if (hdr->cap_len == hdr->pkg_len) {
        mac_parser_->parse(eth_pkg, hdr->cap_len, &(hdr->ct));
    }
    else {
    	log_dbg("cap_len != pkg_len. ");
    }

    log_dbg("\n");
}


/////////////////////////////
// for test
void save_pkgs(const char *src, const int32_t len) {
    std::ofstream ofs;
    ofs.open("/tmp/tcp_rb.txt", std::ios::app | std::ios::binary);
    ofs.write(src, len);
    ofs.flush();
}


void read_libpcap_file(const char *fname, SrSwBuffer &buf) {
    SpliteLibpcapFile s(buf);
    if (s.init()) {
        s.read_file(fname);
    }
}

void parse_pcap_data(SrSwBuffer &buf) {
    ParseLibpcapData s;
    if (!s.set_filter("10.25.26.219", "10.25.24.41", 9933, 0, "tcp")) {
        return;
    }

    const char *src = nullptr;
    while (true) {
        if (buf.read(src)) {
            PcapPkgHdr *hd = (PcapPkgHdr*)src;
            s.parse(hd, src + sizeof(PcapPkgHdr));
        }
    }

    // test
    if (0) {
        uint64_t size = 0;
        while (true) {
            if (buf.read(src)) {
                PcapPkgHdr *hd = (PcapPkgHdr*)src;
                save_pkgs(src, hd->cap_len + sizeof(PcapPkgHdr));

                assert(hd->cap_len > 0);
                size += (uint32_t(hd->cap_len) + sizeof(PcapPkgHdr));
                log_dbg("write_file_len [%lu] \n", size);
            }
        }
    }
}
