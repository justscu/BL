#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "utils_algorithm_crypto.h"

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "0123456789+/";

static_assert(sizeof(base64_chars) == 64 + 1, "");

static const char base64_index[256] = {
    /* 0x00 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0x10 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0x20 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    /* 0x30 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    /* 0x40 */ -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    /* 0x50 */ 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    /* 0x60 */ -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    /* 0x70 */ 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    /* 0x80 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0x90 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xA0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xB0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xC0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xD0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xE0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 0xF0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

uint32_t base64_encode(const char *input, const uint32_t input_len,
                       char *output, uint32_t output_len) {
    assert(output_len > 0);

    if (!input || !output || input_len == 0) {
        return 0;
    }

    const uint32_t ret = input_len/3 * 4 + (input_len % 3 > 0 ? 4 : 0);
    if (ret > output_len) {
        return 0; // err.
    }

    //
    const uint32_t len1 = input_len / 3;
    for (uint32_t i = 0; i < len1; ++i) {
        const uint32_t triple = ((unsigned char)input[i*3 + 0] << 16)
                              + ((unsigned char)input[i*3 + 1] << 8)
                              +  (unsigned char)input[i*3 + 2];
        output[i*4 + 0] = base64_chars[(triple >> 18) & 0x3F];
        output[i*4 + 1] = base64_chars[(triple >> 12) & 0x3F];
        output[i*4 + 2] = base64_chars[(triple >>  6) & 0x3F];
        output[i*4 + 3] = base64_chars[(triple >>  0) & 0x3F];
    }

    // 处理剩余的 1 字节
    if (input_len % 3 == 1) {
        const uint32_t triple = (unsigned char)input[len1*3] << 16;
        output[len1*4 + 0] = base64_chars[(triple >> 18) & 0x3F];
        output[len1*4 + 1] = base64_chars[(triple >> 12) & 0x3F];
        output[len1*4 + 2] = '=';
        output[len1*4 + 3] = '=';
    }
    // 处理剩余的 2 字节
    else if (input_len % 3 == 2) {
        const uint32_t triple = ((unsigned char)input[len1*3 + 0] << 16) + ((unsigned char)input[len1*3 + 1] << 8);
        output[len1*4 + 0] = base64_chars[(triple >> 18) & 0x3F];
        output[len1*4 + 1] = base64_chars[(triple >> 12) & 0x3F];
        output[len1*4 + 2] = base64_chars[(triple >>  6) & 0x3F];
        output[len1*4 + 3] = '=';
    }

    assert(ret % 4 == 0);

    return ret;
}

uint32_t base64_decode(const char *input, const uint32_t input_len,
                       char *output, uint32_t output_len) {
    if (!input || !output || input_len % 4 != 0) {
        return 0;
    }

    const uint32_t ret = input_len / 4 * 3;
    if (ret > output_len) {
        return 0;
    }

    const uint32_t len1 = input_len / 4;
    for (uint32_t i = 0; i < len1; ++i) {
        uint8_t c0 = (uint8_t)input[i*4 + 0];
        uint8_t c1 = (uint8_t)input[i*4 + 1];
        uint8_t c2 = (uint8_t)input[i*4 + 2];
        uint8_t c3 = (uint8_t)input[i*4 + 3];

        //
        uint8_t v0 = base64_index[c0];
        uint8_t v1 = base64_index[c1];
        uint8_t v2 = base64_index[c2];
        uint8_t v3 = base64_index[c3];

        // 解码为3个字节
        unsigned char b0 = (v0 << 2) | (v1 >> 4);
        unsigned char b1 = ((v1 & 0x0F) << 4) | (v2 >> 2);
        unsigned char b2 = ((v2 & 0x03) << 6) | v3;

        // check
        if (v0 == (uint8_t)-1
                || v1 == (uint8_t)-1
                || ((c2 != '=') && (v2 == (uint8_t)-1))
                || ((c3 != '=') && (v3 == (uint8_t)-1))) {
            return 0;
        }

        if (c2 == '=') {
            assert(c3 == '=');
            output[i*3 + 0] = b0;
            return ret - 2;
        }
        else if (c3 == '=') {
            output[i*3 + 0] = b0;
            output[i*3 + 1] = b1;
            output[i*3 + 2] = 0;
            return ret - 1;
        }
        else {
            output[i*3 + 0] = b0;
            output[i*3 + 1] = b1;
            output[i*3 + 2] = b2;
        }
    }

    return ret;
}
