#include "utils_fmt.h"
#include <stdarg.h>
#include <assert.h>

std::string UtilFmt::format(const char *temp, ...) {
    va_list ap;
    va_start(ap, temp);
    std::string s = vformat(temp, ap);
    va_end(ap);
    return s;
}

std::string UtilFmt::vformat(const char *temp, va_list ap) {
    std::string ret;
    // 与分配1K，不够时自动扩展
    int32_t size = 1024; // 1K
    while (true) {
        char buf[size];
        va_list aq;
        __va_copy(aq, ap);
        int32_t r = vsnprintf(buf, size, temp, aq);
        assert(r >= 0);
        if (r < size) {
            ret = buf;
            break;
        }
        size *= 2;
    } // while

    return ret;
}
