#include <fstream>
#include <sstream>
#include <string.h>
#include "utils_file_txt.h"

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
