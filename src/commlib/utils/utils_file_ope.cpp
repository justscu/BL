#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "utils_file_ope.h"

UtilsFileOpe::~UtilsFileOpe() {
    if (pfile_) {
        fflush(pfile_);
        fclose(pfile_);
        pfile_ = nullptr;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// mode:
//     rb , 打开一个二进制文件，只读
//     wb+, 打开一个二进制文件，允许读写
//     a+, 允许读取和追加
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool UtilsFileOpe::open(const char *fname, const char *mode) {
    set_file_name(fname);
    pfile_ = fopen(fname_, mode);
    if (!pfile_) {
        snprintf(last_err_, sizeof(last_err_)-1, "open [%s] failed. err[%s]", fname, strerror(errno));
        return false;
    }

    return true;
}

size_t UtilsFileOpe::size() {
    seek(0, SEEK_END);
    size_t ret = ftell(pfile_);
    seek(0, SEEK_SET);
    return ret;
}

size_t UtilsFileOpe::read(char *buf, size_t buf_size) {
    const size_t ret = fread(buf, 1, buf_size, pfile_);
    if (buf_size != ret) {
        snprintf(last_err_, sizeof(last_err_)-1, "read [%s] failed. err[%s]", fname_, strerror(errno));
    }
    return ret;
}

size_t UtilsFileOpe::write(const char *str, size_t len) {
    const size_t ret = fwrite(str, 1, len, pfile_);
    if (len != ret) {
        snprintf(last_err_, sizeof(last_err_)-1, "write [%s] failed. err[%s]", fname_, strerror(errno));
    }
    return ret;
}

bool UtilsFileOpe::create_path(const char *path_name) {
    if (0 != mkdir(path_name, 0777)) {
        if (errno != EEXIST) {
            fprintf(stdout, "mkdir [%s] failed. err[%s]. \n", path_name, strerror(errno));
            return false;
        }
    }

    return true;
}

bool UtilsFileOpe::file_exist(const char *filename) {
    return 0 == access(filename, R_OK);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 遍历目录，将目录下所有的文件名，存放到files.
// 递归遍历所有子目录
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool UtilsFileOpe::traverse_dir(const char *dir_name, std::vector<std::string> &files) {
    if (!dir_name) {
        return true;
    }

    struct dirent *filename = nullptr;
    DIR *dir = opendir(dir_name);
    if (!dir) {
        snprintf(last_err_, sizeof(last_err_)-1, "open [%s] failed: %s.", dir_name, strerror(errno));
        return false;
    }

    char buf[256] = {0};
    while ((filename = readdir(dir)) != nullptr) {
        if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) {
            continue;
        }
        snprintf(buf, sizeof(buf)-1, "%s/%s", dir_name, filename->d_name);

        struct stat s;
        stat(buf, &s);
        if (S_ISDIR(s.st_mode)) {
            traverse_dir(buf, files);
        }
        else {
            files.emplace_back(buf);
        }
    }

    return true;
}



// 返回读取的长度
int64_t UtilsReadTxtFile::read_file(char delim, line_func_cb cb) {
    std::ifstream fin(file_name_);
    if (!fin.is_open()) {
        snprintf(last_err_, sizeof(last_err_)-1, "open [%s] failed. err[%s]", file_name_, strerror(errno));
        return 0;
    }

    int64_t file_size = 0;

    std::vector<std::string> fragments;

    char buf[4096];
    while (fin.getline(buf, sizeof(buf))) {
        file_size += fin.gcount();

        fragments.clear();

        std::istringstream ifs(buf);

        // 将行切割成段
        std::string fragment;
        while (std::getline(ifs, fragment, delim)) {
            fragments.push_back(fragment);
        }
        //
        cb(fragments);
    }

    return file_size;
}

int64_t UtilsReadTxtFile::file_size() {
    FILE *pfile = fopen(file_name_, "rb+");
    if (!pfile) {
        snprintf(last_err_, sizeof(last_err_)-1, "open [%s] failed. err[%s]", file_name_, strerror(errno));
        return -1;
    }

    fseek(pfile, 0, SEEK_END);
    size_t ret = ftell(pfile);
    fseek(pfile, 0, SEEK_SET);

    fclose(pfile);

    return ret;
}
