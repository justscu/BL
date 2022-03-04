#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <new>
#include "parse_libpcap_file.h"
#include "parse_l1_layer.h"


bool SpliteLibpcapFile::init() {
    buf_ = new (std::nothrow) char[buf_size];
    return (buf_ != nullptr);
}

void SpliteLibpcapFile::read_file(const char *fname) {
    UtilsFileOpe fileope;
    if (!fileope.open(fname, "rb")) {
        log_err("fopen [%s] failed, %s.", fname, strerror(errno));
        return ;
    }

    const size_t fsize = fileope.size();
    log_info("fopen [%s] success. file_size[%ld]", fname, fsize);

    size_t tlen = fileope.read(buf_, sizeof(pcap_hdr_t));
    if (get_pcap_file_header(buf_, tlen) < 0) {
        log_err("read get_pcap_file_header failed.");
        return;
    }

    while (tlen < fsize) {
        const size_t l = fileope.read(buf_, buf_size);
        if (l > 0) {
            int32_t ret = get_pcap_package(buf_, l);
            if (ret <= 0) {
                break;
            }
            tlen += ret;
            fileope.seek(tlen, SEEK_SET);
        }
    }

    log_info(">>>>> file[%s]: parse total length [%ld].", fname, tlen);
}

int32_t SpliteLibpcapFile::get_pcap_package(const char *str, const int32_t len) {
    const char *beg = str;
    const char *end = str + len;

    cap_hdr caphdr = {0};

    while (beg < end) {
        if (beg + sizeof(PcapFilePkgHdr) > end) {
            break;
        }

        const PcapFilePkgHdr *hd = (const PcapFilePkgHdr*)beg;
        const int32_t c = sizeof(PcapFilePkgHdr) + hd->cap_len;

        assert(hd->cap_len > 0);

        if (c + beg > end) {
            break;
        }

        caphdr.ct.tv_sec  = hd->tm_sec;
        caphdr.ct.tv_usec = hd->tm_usec;
        caphdr.cap_len    = hd->cap_len;
        caphdr.pkg_len    = hd->pkg_len;

        pcapbuf_.write(&caphdr, beg+sizeof(PcapFilePkgHdr));
        beg += c;
    }

    return (beg - str);
}

int32_t SpliteLibpcapFile::get_pcap_file_header(const char *str, int32_t len) {
    static_assert(sizeof(pcap_hdr_t) == 24, "");
    if (len == sizeof(pcap_hdr_t)) {
        memcpy(&pcap_file_head_, str, len);
        if (pcap_file_head_.magic != PCAP_HEAD_MAGIC) {
            log_err("pcap_file_head.magic_number error.");
            return -1;
        }

        if (pcap_file_head_.link_layer_type != 0x01) {
            log_err("pcap_file_head.link_layer_type error.");
            return -1;
        }

        return len;
    }
    return -1;
}


bool ParseLibpcapData::set_filter(const char *src_ip, const char *dst_ip,
                                  uint16_t src_port, uint16_t dst_port,
                                  const char *protocol,
                                  L3DataReadyCBFunc cbfunc) {
    ip_parser_ = new (std::nothrow) ParseIPLayer;
    if (!ip_parser_) {
    	log_err("new ParseIPLayer failed.");
    	return false;
    }

    ip_parser_->set_ip_filter(src_ip, dst_ip);
    ip_parser_->set_protocol_filter(protocol);
    ip_parser_->set_port_filter(src_port, dst_port);
    if (!ip_parser_->create_l3_layer(cbfunc)) {
    	return false;
    }

    mac_parser_ = new (std::nothrow) ParseEthLayer(ip_parser_);
    if (!mac_parser_) {
    	log_err("new ParseEthLayer failed.");
        return false;
    }

    return true;
}

void ParseLibpcapData::parse(const cap_hdr *hdr, const char *eth_pkg) {
    log_dbg("package_%u %u.%06u %5d, %5d  ",
            ++idx_, hdr->ct.tv_sec, hdr->ct.tv_usec, hdr->cap_len, hdr->pkg_len);

    if (hdr->cap_len == hdr->pkg_len) {
        mac_parser_->parse(&(hdr->ct), eth_pkg, hdr->cap_len);
    }
    else {
    	log_dbg("cap_len != pkg_len.");
    }
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

    sleep(60);
}

void parse_pcap_data(const char *src_ip, const char *dst_ip,
                     const char *src_port, const char *dst_port,
                     SrSwBuffer &buf,
                     L3DataReadyCBFunc cbfunc) {
    ParseLibpcapData s;
    if (!s.set_filter(src_ip, dst_ip, atoi(src_port), atoi(dst_port), "tcp", cbfunc)) {
        return;
    }

    cap_hdr  hdr;
    const char *src = nullptr;
    while (true) {
        buf.read(&hdr, src);
        s.parse(&hdr, src);
    }
}
