#ifndef __LOG_LOG_H__
#define __LOG_LOG_H__

#include <string.h>
#include "logger.h"
#include "logdispatcher.h"
#include "format.h"

// CMakeLists.txt 这样做：set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
// makefile，这样做：CXX_FLAGS+=-D__FILENAME__='\"$(subst $(SOURCE_PREFIX)/,,$(abspath $<))\"'"
// 将__FILE__ 换成 __FILENAME__，可以去掉长前缀路径


// 将字符串转换为enum.
namespace LOG {
extern LOGLEVEL str2enum(const char* str);
}

#define SET_LOGGER(pLogger) \
    LOG::LogDispatcher::Instance()->SetLogger(pLogger)

#define AC_LOG(level, ...) \
    do { \
    	std::string s; \
	    LOG::LogDispatcher::Instance()->Log(level, LOG::FormatLog(s, LOG::LogDispatcher::Instance()->GetLogger()->GetFormat(), level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)) ; \
    } while(0)


#define  BASE(...) AC_LOG(LOG::LOGLEVEL::kBase , __VA_ARGS__)
#define TRACE(...) AC_LOG(LOG::LOGLEVEL::kTrace, __VA_ARGS__)
#define DEBUG(...) AC_LOG(LOG::LOGLEVEL::kDebug, __VA_ARGS__)
#define  INFO(...) AC_LOG(LOG::LOGLEVEL::kInfo , __VA_ARGS__)
#define  WARN(...) AC_LOG(LOG::LOGLEVEL::kWarning, __VA_ARGS__)
#define ERROR(...) AC_LOG(LOG::LOGLEVEL::kError,   __VA_ARGS__)
#define CRITICAL(...) AC_LOG(LOG::LOGLEVEL::kCritical, __VA_ARGS__)
#define ERROR_CMP_RET(cmp, ret, ...) if ((cmp)) {AC_LOG(LOG::LOGLEVEL::kError,   __VA_ARGS__); return ret;}



#endif /* __LOG_LOG_H__ */
