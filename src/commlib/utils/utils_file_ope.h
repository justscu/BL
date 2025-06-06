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
    //     rb , 打开一个二进制文件，只读
    //     wb+, 打开一个二进制文件，允许读写
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool open(const char *fname, const char *mode);
    FILE* pfile() { return pfile_; }

    int64_t seek(int64_t offset, int32_t whence) { return fseek(pfile_, offset, whence); }
    size_t size();
    size_t read(char *buf, size_t buf_size);
    size_t write(const char *str, size_t len);

    const char *last_error() const { return last_err_; }

public:
    bool create_path(const char *path_name);
    bool file_exist(const char *file_name);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 遍历目录，将目录下所有的文件名，存放到files.
    // 递归遍历所有子目录
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool traverse_dir(const char *dir_name, std::vector<std::string> &files);

private:
    void set_file_name(const char *fname) { strncpy(fname_, fname, sizeof(fname_)); }

private:
    char last_err_[512] = {0};
    char    fname_[256] = {0};
    FILE   *pfile_      = nullptr;
};



using line_func_cb = bool (*)(std::vector<std::string> &fragments);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 读取txt文件
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsReadTxt {
public:
    UtilsReadTxt(const char *file_name) : file_name_(file_name) { }

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
