#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>

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

    int64_t seek(int64_t offset, int32_t whence) { return fseek(pfile_, offset, whence); }
    size_t size();
    size_t read(char *buf, size_t buf_size);
    size_t write(const char *str, size_t len);

    const char *last_error() const { return last_err_; }

public:
    static bool file_exist(const char *file_name);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 遍历目录，将目录下所有的文件名，存放到files.
    // 递归遍历所有子目录
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    static bool traverse_dir(const char *dir_name, std::vector<std::string> &files);

private:
    void set_file_name(const char *fname) { strncpy(fname_, fname, sizeof(fname_)); }

private:
    char last_err_[512] = {0};
    char    fname_[256] = {0};
    FILE   *pfile_      = nullptr;
};
