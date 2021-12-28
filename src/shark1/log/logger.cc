#include <string.h>
#include <errno.h>
#include "logger.h"

namespace LOG {

LOGLEVEL str2enum(const char* str) {
    if (0 == strcasecmp(str, "kEmerg"))    { return kEmerg; }
    if (0 == strcasecmp(str, "kAlert"))    { return kAlert; }
    if (0 == strcasecmp(str, "kCritical")) { return kCritical; }
    if (0 == strcasecmp(str, "kError"))    { return kError; }
    if (0 == strcasecmp(str, "kWarning"))  { return kWarning; }
    if (0 == strcasecmp(str, "kInfo"))     { return kInfo; }
    if (0 == strcasecmp(str, "kDebug"))    { return kDebug; }
    if (0 == strcasecmp(str, "kTrace"))    { return kTrace; }
    return kUnkown;
}

int FileLogger::Log(LOGLEVEL level, const std::string &msg) {
    if (level > level_) return 0;

    // time
    time_t t;      time(&t);
    struct tm now; localtime_r(&t, &now);
    if (now.tm_yday != tm_yday_) {
        mutex_.Acquire();
        tm_yday_ = now.tm_yday;
        close(fd_);
        fd_ = -1;
        mutex_.Release();
    }

    // create log file
    if (fd_ < 0) {
        mutex_.Acquire();
        if (fd_ < 0) {
            fd_ = open(GetLogFileName().c_str(),  O_CREAT | O_WRONLY | O_APPEND /*|O_LARGEFILE*/, 0644);
            if (fd_ < 0) {
                mutex_.Release();
                printf("open log file[%s] failed: err[%s] \n", GetLogFileName().c_str(), strerror(errno));
                return -1;
            }
        }
        mutex_.Release();
    }

    // pwrite
    if (pwrite(fd_, msg.c_str(), msg.size(), SEEK_END) != (int) msg.size()) {
        close(fd_);
        fd_ = -1;
        printf("pwrite failed: err[%s], log[%s] \n", strerror(errno), msg.c_str());
        return -2;
    }
    // success
    return 0;
}

std::string FileLogger::GetLogFileName() {
    char buf[64];
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm stTime;
    localtime_r(&tv.tv_sec, &stTime);
//    strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &stTime);
    strftime(buf, sizeof(buf), "%Y%m%d", &stTime);

    return fileName_ + ".log." + buf;
}


}
