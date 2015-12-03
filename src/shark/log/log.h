#ifndef __LOG_LOG_H__
#define __LOG_LOG_H__

#include <string.h>
#include "logger.h"
#include "logdispatcher.h"
#include "format.h"

#define SET_LOGGER(level, pLogger) \
    LOG::LogDispatcher::Instance()->SetLogger(level, pLogger)

#define AC_LOG(level, ...) \
    do { \
    	std::string s; \
	    LOG::LogDispatcher::Instance()->Log(level, LOG::FormatLog(s, LOG::LogDispatcher::Instance()->GetLogger(level)->GetFormat(), level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)) ; \
    } while(0)

#define AC_INFO
#define AC_DEBUG
#define AC_ERROR
#define AC_ERROR_CMP_RET
#define AC_WARN

#ifdef AC_INFO
#define INFO(...) AC_LOG(LOG::LOGLEVEL::kInfo ,   __VA_ARGS__)
#else
#define INFO(...) (void(0))
#endif

#ifdef AC_DEBUG
#define DEBUG(...) AC_LOG(LOG::LOGLEVEL::kDebug,   __VA_ARGS__)
#else
#define DEBUG(...) (void(0))
#endif

#ifdef AC_ERROR
#define ERROR(...) AC_LOG(LOG::LOGLEVEL::kError,   __VA_ARGS__)
#else
#define ERROR(...) (void(0))
#endif

#ifdef AC_ERROR_CMP_RET
#define ERROR_CMP_RET(cmp, ret, ...) if ((cmp)) {AC_LOG(LOG::LOGLEVEL::kError,   __VA_ARGS__); return ret;}
#else
#define ERROR_CMP_RET (void(0))
#endif

#ifdef AC_WARN
#define  WARN(...) AC_LOG(LOG::LOGLEVEL::kWarning, __VA_ARGS__)
#else
#define WARN(...) (void(0))
#endif

#endif /* __LOG_LOG_H__ */
