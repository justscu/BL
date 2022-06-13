#include <stdio.h>
#include <string.h>
#include <errno.h>
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
//     rb+, 打开一个二进制文件，只读
//     wb+, 打开一个二进制文件，允许读写
//     a+, 允许读取和追加
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool UtilsFileOpe::open(const char *fname, const char *mode) {
    set_file_name(fname);
    pfile_ = fopen(fname_, mode);
    if (!pfile_) {
        snprintf(last_err_, sizeof(last_err_)-1,
                "open [%s] failed. err[%s]", fname, strerror(errno));

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
