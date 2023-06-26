#pragma once

#include <vector>
#include <string>
#include <assert.h>


//#define StrNCmp(A, B, LEN) (0==(((*(const uint64_t*)(A)) ^ (*(const uint64_t*)(B))) & (~(-1ULL << ((LEN)*8)))))
union UnionStrInt64 {
    const char      *str;
    const uint64_t *intv;
};

inline
bool StrNCmp(const char *str1, const char *str2, uint32_t len) {
    UnionStrInt64 c1 = {.str = str1};
    UnionStrInt64 c2 = {.str = str2};

    return 0 == ((*c1.intv ^ *c2.intv) & (~(-1ULL << (len*8))));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// string utils.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsStr {
public:
    static void split(const std::string &str, const char delim, std::vector<std::string> &out);
    static void trim(std::string &str, const std::string &delim);
    // "ab, a c ,, , adf"   =>   "ab,a c,,,adf"
    static void trim(std::vector<std::string> &vec, const std::string &delim);
    // "ab, a c ,, , adf"   =>   "ab,a c,adf"
    static void trim_and_rm(std::vector<std::string> &vec, const std::string &delim);
    static void to_lower(std::string &str);
    static void to_upper(std::string &str);
    static void replace(std::string &str, const std::string &from, const std::string &to);

public:
    static const char *white_space_delimiters;
};
