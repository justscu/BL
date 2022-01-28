#pragma once

#include <string>
#include "sync_log.h"

namespace LOG {

enum LEVEL {
    kOff     = 0,  // not write log.
    kError   = 1,
    kWarning = 2,
    kInfo    = 3,
    kDebug   = 4,
    kTrace   = 5
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Format
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Format {
public:
    void operator()(LEVEL level,
                    const char *file,
                    const int32_t line,
                    const char *func,
                    const char *fmt, ...);

    const char *buf() const { return buf_; }
    int32_t buf_len() const { return len_; }

    static LEVEL to_lvl(const char *str);

private:
    void pnt_time();
    void pnt_lvl(const LEVEL lvl);

private:
    char    buf_[1024] = {0};
    int32_t len_ = 0;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Dispatcher
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Dispatcher {
public:
    static Dispatcher& instance() {
        static Dispatcher v;
        return v;
    }

    void init(LEVEL lvl, const std::string &log_file_name);

private:
    Dispatcher() { sync_log = new SyncLogDefault(""); }

public:
    LEVEL      max_lvl = kInfo;
    SyncLogBase *sync_log;
};

} // namespace LOG
