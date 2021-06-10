#pragma once

#include <string>

///////////////////////////////
//
// 处理字符串
//
///////////////////////////////
class UtilsString {
public:
    // 去掉str开头的空格
    const char* trim(const char *str);
    // 去掉str前后的空格
    std::string trim(const std::string &str);
    // 将src按照delim进行切分,将结果放入vec
    bool split(const char *src, const char *delim, std::vector<std::string> &vec);

    // 按照二进制打印数据
    // 如 binary_print<uint64_t>(4841256);
    // binary_print<char>('c');
    template <class TYPE>
    void binary_print(const TYPE u) {
        for (int i = sizeof(TYPE)*8-1; i >= 0; --i) {
            TYPE pos = (1ULL << i);
            if (u & pos) printf("1");
            else         printf("0");
        }
    }
};
