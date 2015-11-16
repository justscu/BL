#ifndef __UTILS_MACRO_HELPER_H__
#define __UTILS_MACRO_HELPER_H__

// 用命令：gcc -E -P a.cc > a.macro，可以将看到展开后宏的样子
//
//

////////////// 计算宏参数个数 //////////////
#define COUNT_ARG_N(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, N, ...) N
#define COUNT_ARG_N_HELPER(...) COUNT_ARG_N(__VA_ARGS__)
#define COUNT_ARG(...) COUNT_ARG_N_HELPER(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define GET_ARG_1(a1, ...) a1
#define GET_ARG_2(a1, a2, ...) a2
// 如　int a =  COUNT_ARG(x, y, z);
// int a = COUNT_ARG((a, b), a, b); // 3

////////////// 禁止拷贝 ///////////////////
#define DISALLOW_COPYING(classname) \
	classname(const classname &) = delete; \
	classname& operator=(const classname &) = delete; \

////////////// 宏函数　/////////////////////
/* a simple macro for making hash "process" functions */
#define HASH_PROCESS(func_name, compress_name, state_var, block_size)               \
    int func_name(hash_state* md, const unsigned char* in, unsigned long inlen) {   \
        unsigned long n;                                                            \
        int err;                                                                    \
        LTC_ARGCHK(md != NULL);                                                     \
        LTC_ARGCHK(in != NULL);                                                     \
        if (md->state_var.curlen > sizeof(md->state_var.buf)) {                     \
            return CRYPT_INVALID_ARG;                                               \
        }                                                                           \
        while (inlen > 0) {                                                         \
            if (md->state_var.curlen == 0 && inlen >= block_size) {                 \
                if ((err = compress_name(md, (unsigned char*)in)) != CRYPT_OK) {    \
                    return err;                                                     \
                }                                                                   \
                md->state_var.length += block_size * 8;                             \
                in += block_size;                                                   \
                inlen -= block_size;                                                \
            } else {                                                                \
                n = MIN(inlen, (block_size - md->state_var.curlen));                \
                memcpy(md->state_var.buf + md->state_var.curlen, in, (size_t)n);    \
                md->state_var.curlen += n;                                          \
                in += n;                                                            \
                inlen -= n;                                                         \
                if (md->state_var.curlen == block_size) {                           \
                    if ((err = compress_name(md, md->state_var.buf)) != CRYPT_OK) { \
                        return err;                                                 \
                    }                                                               \
                    md->state_var.length += 8 * block_size;                         \
                    md->state_var.curlen = 0;                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        return CRYPT_OK;                                                            \
    }


#endif /* __UTILS_MACRO_HELPER_H__ */
