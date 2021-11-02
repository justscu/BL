#pragma once

#include "parse_l3_layer.h"
#include "parse_l2_layer.h"
#include "headers.h"

class ParseEthLayer {
public:
    ParseEthLayer(ParseIPLayer *ps1) : ip_layer_(ps1) {
        static_assert(sizeof(EthHdr) == 14, "sizeof(EthHdr)=14.");
    }
    // str: 链路层数据(含头部)
    // len: 链路层数据长度
    void parse(const char *str, const int32_t len);

private:
    void print(const EthHdr *hdr) const;
    bool checksum(const uint8_t *str, const int32_t len) const;

private:
    ParseIPLayer *ip_layer_ = nullptr;
};
