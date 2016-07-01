#ifndef __LOG_LOGDISPATCHER_H__
#define __LOG_LOGDISPATCHER_H__

#include <vector>
#include <iostream>
#include <assert.h>
#include "lock.h"
#include "logger.h"

namespace LOG {
class LogDispatcher {
private:
    LogDispatcher() {
        default_logger_ = new (std::nothrow) LOG::OStreamLogger(std::cout, LOG::LOGLEVEL::kTrace);
        logger_         = default_logger_;
        assert(default_logger_ != nullptr);
    }
    LogDispatcher(const LogDispatcher &) = delete;
    LogDispatcher& operator=(const LogDispatcher &) = delete;
public:
    static LogDispatcher* Instance() {
        static LogDispatcher p;
        return &p;
    }
    ~LogDispatcher() {
        if (default_logger_ != nullptr) {
            delete default_logger_;
            default_logger_ = nullptr;
        }
    }
    inline
    void SetLogger(Logger *newLogger) {
        if (newLogger != nullptr) logger_ = newLogger;
    }
    inline
    Logger* GetLogger() {
        return logger_;
    }
    // 写日志
    inline
    int Log(LOGLEVEL level, const std::string &msg) {
        return logger_->Log(level, msg);
    }
private:
    Logger*         logger_; // logger
    Logger* default_logger_;
};

}

#endif /*__LOG_LOGDISPATCHER_H__*/
