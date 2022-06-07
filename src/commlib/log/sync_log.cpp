#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include "sync_log.h"

namespace LOG {

SyncLogBase::SyncLogBase(const std::string &fname) : log_fname_(fname) {
//    if ((fname.find("/") == std::string::npos && fname.find("\\") == std::string::npos)
//            || (fname.c_str()[0] != '/' && fname.c_str()[0] != '\\')) {
//        fprintf(stdout, "%s must full path. \n", fname.c_str());
//        exit(-1);
//    }
}

std::string SyncLogBase::set_log_file_name() {
    std::string fname(log_fname_);

    char buf[16];
    time_t t;
    if (-1 == time(&t)) {
        fprintf(stdout, "time failed. %s. \n", strerror(errno));
        return std::string("");
    }

    struct tm ltm;
    if (nullptr == localtime_r(&t, &ltm)) {
        fprintf(stdout, "localtime_r failed. %s. \n", strerror(errno));
        return std::string("");
    }

    strftime(buf, sizeof(buf), ".%Y%m%d", &ltm);
    fname.append(buf);
    return fname;
}

int32_t SyncLogDefault::write(const char *str, size_t len) {
    mutex_.lock();
    fprintf(stdout, "%s", std::string(str, len).c_str());
    mutex_.unlock();
    return len;
}

int32_t SyncLogFile::write(const char *str, size_t len) {
    if (log_file_fd_ < 0) {
        std::string fname = set_log_file_name();
        mutex_.lock();
        log_file_fd_ = open(fname.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_LARGEFILE, 0644);
        mutex_.unlock();
        if (log_file_fd_ < 0) {
            fprintf(stdout, "open log file [%s] failed, err[%s] \n", fname.c_str(), strerror(errno));
            return -1;
        }
    }

    if (pwrite(log_file_fd_, str, len, SEEK_END) != len) {
        close(log_file_fd_);
        log_file_fd_ = -1;
        fprintf(stdout, "pwrite log file failed, err[%s] \n", strerror(errno));
        return -1;
    }

    // fsync(log_file_fd_);

    return len;
}

} // namespace LOG
