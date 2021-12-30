#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <algorithm>
#include <sys/syscall.h>
#include "format.h"

namespace LOG {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Format
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Format::operator()(LEVEL level,
                        const char *file,
                        const int32_t line,
                        const char *func,
                        const char *fmt, ...) {
    pnt_time();
    pnt_lvl(level);
    {
        va_list ap;
        va_start(ap, fmt);
        len_ += vsnprintf(buf_+len_, sizeof(buf_)-len_, fmt, ap);
        va_end(ap);
    }

    if (level <= kWarning ) {
        len_ += snprintf(buf_+len_, sizeof(buf_)-len_, " (%s:%d)", file, line);
    }

    len_ += snprintf(buf_+len_, sizeof(buf_)-len_, "\n");
}

LEVEL Format::to_lvl(const char *str) {
    std::string s(str);
    std::for_each(s.begin(), s.end(), [](char &c) { c = std::tolower(c); });

    if (s.find("off") != std::string::npos) { return kOff; }
    if (s.find("err") != std::string::npos) { return kError; }
    if (s.find("warn") != std::string::npos) { return kWarning; }
    if (s.find("info") != std::string::npos) { return kInfo; }
    if (s.find("debug") != std::string::npos) { return kDebug; }
    if (s.find("trace") != std::string::npos) { return kTrace; }

    return kInfo;
}

void Format::pnt_time() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm t;
    localtime_r(&tv.tv_sec, &t);
    len_ += snprintf(buf_, sizeof(buf_)-1, "%02d:%02d:%02d.%06ld", t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec);
}

void Format::pnt_lvl(const LEVEL lvl) {
    const char *str = nullptr;
    switch (lvl) {
        case kError:   str = "EROR"; break;
        case kWarning: str = "WARN"; break;
        case kInfo:    str = "INFO"; break;
        case kDebug:   str = "DEBG"; break;
        case kTrace:   str = "TRAC"; break;
        default:       str = "----"; break;
    }
    len_ += snprintf(buf_+len_, sizeof(buf_)-len_, "[%s][%lu] ", str, syscall(SYS_gettid));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LogDispatcher
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Dispatcher::init(LEVEL lvl, const std::string &log_file_name) {
    max_lvl  = lvl;
    sync_log = new (std::nothrow) SyncLogFile(log_file_name);
    if (!sync_log) {
        fprintf(stdout, "new SyncLogFile failed. \n");
        exit(-1);
    }
}

} // namespace LOG
