#ifndef __LOG_LOGDISPATCHER_H__
#define __LOG_LOGDISPATCHER_H__


#include <vector>
#include <iostream>
#include "lock.h"
#include "logger.h"

namespace LOG {
// 
// TODO: 
// Caution: LogDispatcher程序中注释掉了Mutex，是出于日志库的性能考虑，
//          在程序启动时，先设置好各个级别的Logger，之后多线程写日志时，
//          不要再次设置Logger 
//
class LogDispatcher {
private:
	LogDispatcher() {
		logVec_.resize(kLogPriMax);
		Logger * p = new (std::nothrow) OStreamLogger(std::cout);
		if (p != nullptr) {
			for (auto i = 0; i < kLogPriMax; ++i) {
				logVec_[i] = p;
			}
		}
	}
	LogDispatcher(const LogDispatcher &) = delete;
	LogDispatcher& operator= (const LogDispatcher &) = delete;
public:
	static LogDispatcher* Instance() {
		//static LogDispatcher *p = new LogDispatcher; // for valgrind check
		static LogDispatcher p;
		return &p;
	}
	~LogDispatcher() {
		if (logVec_[kBase] != nullptr) {
			delete logVec_[kBase];
			logVec_[kBase] = nullptr;
		}
		logVec_.clear();
	}
	// Caution: 请在初始化时设置，不要在多线程运行过程中设置
	int SetLogger(const LOGLEVEL level, Logger *newLogger) {
		if (newLogger == nullptr) {
			newLogger = logVec_[0];
		}
		// level=0，不能修改
		if (level > 0 && level < kLogPriMax) {
			// mutex_.Acquire();
			logVec_[level] = newLogger;
			// mutex_.Release();
			return 0;
		}
		return -1;
	}
	inline
	Logger* GetLogger(const LOGLEVEL level) {
		Logger *p = logVec_[0];;
		if (level >0 && level < kLogPriMax) {
			// mutex_.Acquire();
			p = logVec_[level];
			// mutex_.Release();
		}
		return p;
	}
	// 写日志
	inline
	int Log(LOGLEVEL level, const std::string &msg) {
		return GetLogger(level)->Log(level, msg);
	}
private:
	Mutex                mutex_;
	std::vector<Logger*> logVec_; //   按照logPriority进行设置
};

}

#endif /*__LOG_LOGDISPATCHER_H__*/
