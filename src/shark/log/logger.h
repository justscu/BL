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
enum LOGLEVEL {
    kBase = 0, // base, 使用fd=0输出
    kTrace = 1,
    kDebug = 2,
    kInfo = 3,
    kWarning = 4,
    kError = 5,
    kCritical = 6,
    kLogPriMax = 7
};

// [date][thread id][level] msg (file:line function)
#define DEFAULT_FORMAT "[%D][%T][%L] %m (%F:%l %f)"
// base
class Logger {
public:
    Logger(const std::string & format = DEFAULT_FORMAT) :
            format_(format) {
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
private:
    const std::string format_;
};

// OStream
class OStreamLogger: public Logger {
public:
    OStreamLogger(std::ostream &stream, const std::string &format =
    DEFAULT_FORMAT) :
            Logger(format), stream_(stream) {
    }

    virtual int Log(LOGLEVEL, const std::string &msg) {
        mutex_.Acquire();
        stream_ << msg; // << std::endl;
        mutex_.Release();
        return 0;
    }
private:
    Mutex mutex_;
    std::ostream &stream_;
};

// file
class FileLogger: public Logger {
public:
    FileLogger(const std::string &fileName, const std::string &format =
            DEFAULT_FORMAT) :
            Logger(format), fileName_(fileName) {
        fd_ = -1;
    }
    virtual ~FileLogger() {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
    }
    virtual int Log(LOGLEVEL, const std::string &msg);
    std::string GetLogFileName();
private:
    int fd_;
    std::string fileName_;
    Mutex mutex_;
};

}

#endif /* __LOG_LOGGER_H__ */
