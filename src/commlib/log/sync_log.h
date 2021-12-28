#pragma once

#include <string>
#include <mutex>

namespace LOG {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SyncLogFile
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SyncLogFile {
public:
    explicit SyncLogFile(const std::string &fname);
    int32_t write(const char *str, size_t len);

protected:
    std::string set_log_file_name();

protected:
    std::string log_fname_;
    std::mutex      mutex_;
    int32_t   log_file_fd_ = -1;
};

} // namespace LOG
