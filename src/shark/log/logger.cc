#include "logger.h"

namespace LOG {
int FileLogger::Log(LOGLEVEL, const std::string &msg) {
	// create log file
	if (fd_ < 0) {
		mutex_.Acquire();
		if (fd_ < 0) {
			fd_ = open(GetLogFileName().c_str(), O_CREAT|O_WRONLY|O_APPEND /*|O_LARGEFILE*/, 0644);
			if (fd_ < 0) {
				mutex_.Release();
				printf("open log file[%s] failed \n", GetLogFileName().c_str());
				return -1;
			}
		}
		mutex_.Release();
	}

	// pwrite
	if (pwrite(fd_, msg.c_str(), msg.size(), SEEK_END) != (int)msg.size()) {
		close(fd_);
		fd_ = -1;
		printf("pwrite failed, log[%s] \n", msg.c_str());
		return -2;
	}
	// success
	return 0;
}

std::string FileLogger::GetLogFileName() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	std::ostringstream os;
	os << tv.tv_sec;

	return fileName_ + ".log." + os.str();
}

}
