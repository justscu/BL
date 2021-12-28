#ifndef __LOG_FORMAT_H__
#define __LOG_FORMAT_H__

#include <string>
#include "logger.h"

namespace LOG {
class Format {
public:
    Format(std::string &str) : str_(str) {
    }
    ~Format() {
    }
    Format(const Format&) = delete;
    Format& operator=(const Format &) = delete;

public:
    void Date();
    void Level(const LOGLEVEL level);
    void File(const char *file);
    void Line(uint16_t line);
    void ProcessID();
    void ThreadID();
    void Function(const char *func);

private:
    std::string &str_;
};

std::string& FormatLog(std::string &str, const std::string &format,
        LOGLEVEL level, const char *file, const int line, const char *func,
        const char *fmt, ...);

}
#endif /*__LOG_FORMAT_H__*/
