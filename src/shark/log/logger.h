#ifndef __LOG_LOGGER_H__
#define __LOG_LOGGER_H__

#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include "lock.h"

namespace LOG {
// log level
// 值越小，优先级越高
enum LOGLEVEL {
    kEmerg     = 0,  // The system is unusable.
    kAlert     = 1,  // Actions that must be taken care of immediately.
    kCritical  = 2,  // Critical conditions.
    kError     = 3,
    kWarning   = 4,
    kInfo      = 5,
    kDebug     = 6,
    kTrace     = 7,
    kUnkown    = 8
};

// [date][thread id][level] msg (file:line function)
#define DEFAULT_FORMAT "[%D][%T][%L] %m (%F:%l %f)"
// base
class Logger {
public:
    Logger(LOGLEVEL level = kInfo, const std::string & format = DEFAULT_FORMAT)
: level_(level), format_(format) {
    }
    virtual ~Logger() {
    }
    virtual int Log(LOGLEVEL level, const std::string &) = 0;
    inline const std::string& GetFormat() const {
        return format_;
    }
private:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
protected:
    LOGLEVEL           level_; // 默认级别(>= level_时打印)
private:
    const std::string format_;
};

// OStream
class OStreamLogger: public Logger {
public:
    // stream,   输出流
    // level,    日志级别，当<= level时，会被写入到输出流
    // format,   缺省的日志格式
    OStreamLogger(std::ostream &stream = std::cout,
            LOGLEVEL level = kInfo,
            const std::string &format = DEFAULT_FORMAT)
: Logger(level, format), stream_(stream) {
    }

    virtual int Log(LOGLEVEL level, const std::string &msg) {
        if (level <= level_) {
            mutex_.Acquire();
            stream_ << msg; // << std::endl;
            mutex_.Release();
        }
        return 0;
    }
private:
    Mutex          mutex_;
    std::ostream &stream_;
};

// file
class FileLogger: public Logger {
public:
    // fileName, 日志文件名
    // level,    日志级别，当<= level时，会被写入到日志文件
    // format,   缺省的日志格式
    FileLogger(const std::string &fileName,
            LOGLEVEL level = kInfo,
            const std::string &format = DEFAULT_FORMAT)
: Logger(level, format), fileName_(fileName) {
        fd_ = -1;
        time_t t;      time(&t);
        struct tm now; localtime_r(&t, &now);
        tm_yday_ = now.tm_yday;
    }
    virtual ~FileLogger() {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
    }
    virtual int Log(LOGLEVEL level, const std::string &msg);
    std::string GetLogFileName();
private:
    int               fd_;
    std::string fileName_;
    Mutex          mutex_;
    int32_t      tm_yday_;
};

}

#endif /* __LOG_LOGGER_H__ */
