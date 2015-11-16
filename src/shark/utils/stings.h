#include <string>
#include <vector>
#include <string.h>

// 去掉@str开头的空格
const char* trim(const char* str) {
    while(*str != 0 && (*str == ' ' || *str == '\r' || *str == '\t' || *str == '\n')) {
        str++;
    }

    return str;
}

// 去掉@str前后的空格
std::string trim(const std::string &str) {
    std::string ret;
    // trim left
    auto it1 = str.begin();
    for (; it1 != str.end(); ++it1) {
        if (*it1 == ' ' || *it1 == '\r' || *it1 == '\t' || *it1 == '\n') {
            continue;
        }
        break;
    }
    ret.assign(it1, str.end());
    // trim right
    auto it2 = ret.end()-1;
    for (; it2 != ret.begin(); --it2) {
        if (*it2 == ' ' || *it2 == '\r' || *it2 == '\t' || *it2 == '\n') {
            continue;
        }
        break;
    }

    ret.erase(it2+1, ret.end());
    return ret;
}

// 将@src按照@delim进行切分,将结果放入@vec
void split(const char* src, const char* delim, std::vector<std::string>& vec) {
    if (src == NULL || strlen(src) == 0) {
        return;
    }
    if (delim == NULL || strlen(delim) == 0) {
        return;
    }

    char* str = new char[strlen(src)+1];
    strcpy(str, src);
    char* p = strtok(str, delim);
    while(p) {
        vec.push_back(p);
        p = strtok(NULL, delim);
    }
    delete [] str;
    str = NULL;
}

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
