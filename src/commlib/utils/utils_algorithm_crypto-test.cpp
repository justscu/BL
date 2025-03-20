#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "utils_algorithm_crypto.h"


void algorithm_crypto_test1() {
    // encode
    const char *src1 = "Hello!"; // 6
    const char *src2 = "Wor123XBC!";  // 10
    const char *src3 = "Wor123XBC!@"; // 11

    const char *dst1 = "SGVsbG8h";
    const char *dst2 = "V29yMTIzWEJDIQ==";
    const char *dst3 = "V29yMTIzWEJDIUA=";

    char output[64] = {0};

    // encode
    memset(output, 0, sizeof(output));
    uint32_t len1 = base64_encode(src1, strlen(src1), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len1, output);
    assert(len1 == 8);
    assert(0 == memcmp(output, dst1, len1));

    memset(output, 0, sizeof(output));
    uint32_t len2 = base64_encode(src2, strlen(src2), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len2, output);
    assert(0 == memcmp(output, dst2, len2));

    memset(output, 0, sizeof(output));
    uint32_t len3 = base64_encode(src3, strlen(src3), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len3, output);
    assert(0 == memcmp(output, dst3, len3));

    // decode
    memset(output, 0, sizeof(output));
    len1 = base64_decode(dst1, strlen(dst1), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len1, output);
    assert(len1 == 6);
    assert(0 == memcmp(output, src1, len1));

    memset(output, 0, sizeof(output));
    len2 = base64_decode(dst2, strlen(dst2), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len2, output);
    assert(len2 == 10);
    assert(0 == memcmp(output, src2, len2));

    memset(output, 0, sizeof(output));
    len3 = base64_decode(dst3, strlen(dst3), output, sizeof(output));
    fprintf(stdout, "%d, %s. \n", len3, output);
    assert(len3 == 11);
    assert(0 == memcmp(output, src3, len3));
}

// 特殊测试
void algorithm_crypto_test2() {
    char input[100];
    char output[100];

    // input: nullptr
    uint32_t ret1 = base64_encode(nullptr, 10, output, sizeof(output));
    uint32_t ret2 = base64_encode(input  ,  0, output, sizeof(output));
    assert(ret1 == 0 && ret2 == 0);

    ret1 = base64_decode(nullptr, 10, output, sizeof(output));
    ret2 = base64_decode(input  ,  0, output, sizeof(output));
    assert(ret1 == 0 && ret2 == 0);

    // output: nullptr
    ret1 = base64_encode(input, 10, nullptr, sizeof(output));
    ret2 = base64_decode(input, 10, nullptr, sizeof(output));
    assert(ret1 == 0 && ret2 == 0);

    // 输出长度小
    ret1 = base64_encode(input, sizeof(input), output, sizeof(output));
    ret2 = base64_decode(input, sizeof(input), output, 16);
    assert(ret1 == 0 && ret2 == 0);

    // 输入非4的倍数
    ret1 = base64_decode(input, 7, output, 8);
    assert(ret1 == 0);

    // 非有效字符测试
    ret1 = base64_decode("SGvsbG@=", 8, output, 8);
    assert(ret1 == 0);
}
