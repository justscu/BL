#pragma once

#include <string>

class UtilFmt {
public:
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 格式化，比sprintf安全，且可以打印完整数据
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    static std::string format(const char *temp, ...);

private:
    static std::string vformat(const char *temp, va_list ap);
};
