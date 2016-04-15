#ifndef __FAST_CMP_STRING_H__
#define __FAST_CMP_STRING_H__

// 快速字符串比较，需要a, b均为字符串指针
// 字符串相同时，返回true; 不相同，返回false

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define FAST_cmpstr_1(a, b)   (0 == ((a)[0] ^ (b)[0]))
#define FAST_cmpstr_2(a, b)   (0 == ((*((int16_t*)(a))) ^ (*((int16_t*)(b)))))
#define FAST_cmpstr_3(a, b)   (0 == (((*((int32_t*)(a))) ^ (*((int32_t*)(b)))) & 0xFFFFFF))
#define FAST_cmpstr_4(a, b)   (0 == ((*((int32_t*)(a))) ^ (*((int32_t*)(b)))))

#define FAST_cmpstr_5(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) &     0xFFFFFFFFFF))
#define FAST_cmpstr_6(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) &   0xFFFFFFFFFFFF))
#define FAST_cmpstr_7(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) & 0xFFFFFFFFFFFFFF))
#define FAST_cmpstr_8(a, b)   (0 ==  ((*((int64_t*)(a))) ^ (*((int64_t*)(b)))))

#define FAST_cmpstr_9(a, b)   (FAST_cmpstr_8(a, b) && FAST_cmpstr_1(a+8, b+8))
#define FAST_cmpstr_10(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_2(a+8, b+8))
#define FAST_cmpstr_11(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_3(a+8, b+8))
#define FAST_cmpstr_12(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_4(a+8, b+8))

#define FAST_cmpstr_13(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_5(a+8, b+8))
#define FAST_cmpstr_14(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_6(a+8, b+8))
#define FAST_cmpstr_15(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_7(a+8, b+8))
#define FAST_cmpstr_16(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_8(a+8, b+8))

#elif __BYTE_ORDER == __BIG_ENDIAN

#define FAST_cmpstr_1(a, b)   (0 == ((a)[0] ^ (b)[0]))
#define FAST_cmpstr_2(a, b)   (0 == ((*((int16_t*)(a))) ^ (*((int16_t*)(b)))))
#define FAST_cmpstr_3(a, b)   (0 == (((*((int32_t*)(a))) ^ (*((int32_t*)(b)))) & 0xFFFFFF00))
#define FAST_cmpstr_4(a, b)   (0 == ((*((int32_t*)(a))) ^ (*((int32_t*)(b)))))

#define FAST_cmpstr_5(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) & 0xFFFFFFFFFF0000))
#define FAST_cmpstr_6(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) & 0xFFFFFFFFFFFF00))
#define FAST_cmpstr_7(a, b)   (0 == (((*((int64_t*)(a))) ^ (*((int64_t*)(b)))) & 0xFFFFFFFFFFFFFF))
#define FAST_cmpstr_8(a, b)   (0 ==  ((*((int64_t*)(a))) ^ (*((int64_t*)(b)))))

#define FAST_cmpstr_9(a, b)   (FAST_cmpstr_8(a, b) && FAST_cmpstr_1(a+8, b+8))
#define FAST_cmpstr_10(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_2(a+8, b+8))
#define FAST_cmpstr_11(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_3(a+8, b+8))
#define FAST_cmpstr_12(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_4(a+8, b+8))

#define FAST_cmpstr_13(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_5(a+8, b+8))
#define FAST_cmpstr_14(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_6(a+8, b+8))
#define FAST_cmpstr_15(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_7(a+8, b+8))
#define FAST_cmpstr_16(a, b)  (FAST_cmpstr_8(a, b) && FAST_cmpstr_8(a+8, b+8))

#else
#error "Unknown byte order"
#endif

// 测试程序
void test_fast_cmpstr() {
    const char* a = "123456789ABCDEF0123";
    char b[] = "123456789ABCDEF0123";
    for (int i = 0; i < 17; ++i) {
        char t = b[i];
        b[i] = 'X';
        printf("1[%d] 2[%d] 3[%d] 4[%d] 5[%d] 6[%d] 7[%d] 8[%d] 9[%d] 10[%d] 11[%d] 12[%d] 13[%d] 14[%d] 15[%d] 16[%d]\n",
                FAST_cmpstr_1(a, b),  FAST_cmpstr_2(a, b),  FAST_cmpstr_3(a, b),  FAST_cmpstr_4(a, b),
                FAST_cmpstr_5(a, b),  FAST_cmpstr_6(a, b),  FAST_cmpstr_7(a, b),  FAST_cmpstr_8(a, b),
                FAST_cmpstr_9(a, b),  FAST_cmpstr_10(a, b), FAST_cmpstr_11(a, b), FAST_cmpstr_12(a, b),
                FAST_cmpstr_13(a, b), FAST_cmpstr_14(a, b), FAST_cmpstr_15(a, b), FAST_cmpstr_16(a, b));
        b[i] = t;
    }
}

#endif /*__FAST_CMP_STRING_H__*/
