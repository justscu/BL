
#include <type_traits>
#include <stdio.h>
#include "crc_checksum.h"
#include "parse_l1_layer.h"

void ParseEthLayer::parse(const timeval *ct, const char *str, const int32_t len) {
    const eth_hdr *hd = (const eth_hdr*)str;
    print(hd);

    // only decode IP.
    if (hd->proto != PROTOCOL_IP) {
        return;
    }

    if (!checksum((const uint8_t*)str, len)) {
        log_dbg("  ETHLayer checksum failed.");
        return;
    }

    ip_layer_->parse(ct, str+sizeof(eth_hdr), len-sizeof(eth_hdr)-sizeof(MacFrameTail));
}

void ParseEthLayer::print(const eth_hdr *hdr) const {
    log_dbg("  [%02x:%02x:%02x:%02x:%02x:%02x->%02x:%02x:%02x:%02x:%02x:%02x/0x%04x] ",
            hdr->srcaddr[0], hdr->srcaddr[1], hdr->srcaddr[2],
            hdr->srcaddr[3], hdr->srcaddr[4], hdr->srcaddr[5],
            hdr->dstaddr[0], hdr->dstaddr[1], hdr->dstaddr[2],
            hdr->dstaddr[3], hdr->dstaddr[4], hdr->dstaddr[5],
            hdr->proto);
}

bool ParseEthLayer::checksum(const uint8_t *str, const int32_t len) const {
    return true;

    if (len < 46) {
        log_err(" len[%d] should>=46. ", len);
        return false;
    }

    const uint32_t rst = crc32(str, len-4);
    const uint32_t sum = (*(uint32_t*)(str+len-4)) & 0xFFFFFFFF;
    return (rst == sum);
}
