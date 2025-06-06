#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <errno.h>


using line_func_cb = bool (*)(std::vector<std::string> &fragments);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 读取txt文件
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsReadTxtFile {
public:
    UtilsReadTxtFile(const char *file_name) : file_name_(file_name) { }

    // 返回读取的总长度(一般为文件长度)
    // 每读取一行，调用一次cb; 各段均放在vector中返回，未去除空格/空.
    // delim为行中各段的分割符. 如'\t'
    int64_t read_file(char delim, line_func_cb cb);

    // 返回文件大小
    int64_t file_size();

    const char *last_err() const { return last_err_; }

private:
    const char *file_name_ = nullptr;
    char last_err_[512] = {0};
};
