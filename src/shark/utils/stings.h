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
