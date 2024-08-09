#pragma once

#include <vector>
#include <string>
#include <assert.h>


//#define is_same_string(A, B, LEN) (0==(((*(const uint64_t*)(A)) ^ (*(const uint64_t*)(B))) & (~(-1ULL << ((LEN)*8)))))


inline
bool is_same_string(const char *str1, const char *str2, uint32_t len) {
    assert(len <= 8);

    union {
        const char      *str;
        const uint64_t *intv;
    }
    c1 = {.str = str1},
    c2 = {.str = str2};

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

    // vec: ["abc", "123", "12"]
    // out: "abc,123,12"
    static std::string tostr(const std::vector<uint16_t> &vec);

public:
    static const char *white_space_delimiters;
};
