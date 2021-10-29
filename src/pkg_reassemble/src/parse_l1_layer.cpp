#include "parse_l1_layer.h"

#include <type_traits>
#include <stdio.h>
#include "crc_checksum.h"


#define IP_PKG_TYPE 0x08

void ParseEthLayer::parse(const char *str, const int32_t len) {
    print((const EthHdr*)str);

    // 只解析IP协议
    if (((const EthHdr*)str)->proto != IP_PKG_TYPE) {
        return;
    }

    if (!checksum((const uint8_t*)str, len)) {
        fprintf(stdout, "MacLayer checksum failed.");
        return;
    }

    ip_layer_->parse(str+sizeof(EthHdr), len-sizeof(EthHdr)-sizeof(MacFrameTail));
}

void ParseEthLayer::print(const EthHdr *hdr) const {
    fprintf(stdout, "[%02x:%02x:%02x:%02x:%02x:%02x->%02x:%02x:%02x:%02x:%02x:%02x/0x%x] ",
            hdr->srcaddr[0], hdr->srcaddr[1], hdr->srcaddr[2],
            hdr->srcaddr[3], hdr->srcaddr[4], hdr->srcaddr[5],
            hdr->dstaddr[0], hdr->dstaddr[1], hdr->dstaddr[2],
            hdr->dstaddr[3], hdr->dstaddr[4], hdr->dstaddr[5],
            hdr->proto);
}

bool ParseEthLayer::checksum(const uint8_t *str, const int32_t len) const {
    return true;

    if (len < 46) {
        fprintf(stdout, " len[%d] should>=46. ", len);
        return false;
    }

    const uint32_t rst = crc32(str, len-4);
    const uint32_t sum = (*(uint32_t*)(str+len-4)) & 0xFFFFFFFF;
    return (rst == sum);
}
