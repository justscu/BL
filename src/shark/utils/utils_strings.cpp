#include "utils_strings.h"

// 去掉str开头的空格
const char* UtilsString::trim(const char *str) {
    while(*str != 0 && (*str == ' ' || *str == '\r' || *str == '\t' || *str == '\n')) {
        str++;
    }
    return str;
}

// 去掉str前后的空格
std::string UtilsString::trim(const std::string &str) {
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

// 将src按照delim进行切分,将结果放入vec
bool UtilsString::split(const char *src, const char *delim, std::vector<std::string> &vec) {
    if (src == nullptr || strlen(src) == 0) {
        return true;
    }
    if (delim == nullptr || strlen(delim) == 0) {
        return true;
    }

    const uint32_t len = strlen(src) + 1;
    char *str = new (std::nothrow) char[len];
    if (!str) {
        return false;
    }
    memcpy(str, src, len);
    char* p = strtok(str, delim);
    while(p) {
        vec.push_back(p);
        p = strtok(nullptr, delim);
    }
    delete [] str;
    str = nullptr;

    return true;
}
