#pragma once

#include <string>
#include <mutex>

namespace LOG {

class SyncLogBase {
public:
    SyncLogBase(const std::string &fname);
    virtual ~SyncLogBase() {}
    virtual int32_t write(const char *str, size_t len) = 0;

protected:
    std::string set_log_file_name();

protected:
    std::string log_fname_;
    std::mutex      mutex_;
    int32_t   log_file_fd_ = -1;
};

class SyncLogDefault : public SyncLogBase {
public:
    SyncLogDefault(const std::string &fname) : SyncLogBase(fname) {}
    virtual ~SyncLogDefault() {}
    virtual int32_t write(const char *str, size_t len);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SyncLogFile
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SyncLogFile : public SyncLogBase {
public:
    SyncLogFile(const std::string &fname) : SyncLogBase(fname) { }
    virtual ~SyncLogFile() {}
    virtual int32_t write(const char *str, size_t len);
};

} // namespace LOG
