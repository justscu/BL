#pragma once

#include <stdint.h>

class UtilsFileOpe {
public:
    UtilsFileOpe() {}
    ~UtilsFileOpe();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // mode:
    //     rb+, 打开一个二进制文件，只读
    //     wb+, 打开一个二进制文件，允许读写
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool open(const char *fname, const char *mode);

    size_t read(char *buf, int32_t buf_size);
    size_t write(const char *str, int32_t len);

    const char *last_error() const { return last_err_; }

private:
    void set_file_name(const char *fname) { strncpy(fname_, fname, sizeof(fname_)); }

private:
    char last_err_[512] = {0};
    char    fname_[256] = {0};
    FILE   *pfile_      = nullptr;
};
