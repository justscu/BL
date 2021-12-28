#pragma once

#include "format.h"
#include "sync_log.h"

#define log_ac(level, ...) do {                                                     \
         if (level <= LOG::Dispatcher::instance().max_lvl) {                        \
             LOG::Format fmt;                                                       \
             fmt(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);             \
             LOG::Dispatcher::instance().sync_log->write(fmt.buf(), fmt.buf_len()); \
         }                                                                          \
     } while(0)                                                                     \

#define log_init(level, filename) LOG::Dispatcher::instance().init(level, filename)
#define log_err(...)   log_ac(LOG::kError, __VA_ARGS__)
#define log_warn(...)  log_ac(LOG::kWarning, __VA_ARGS__)
#define log_info(...)  log_ac(LOG::kInfo, __VA_ARGS__)
#define log_dbg(...)   log_ac(LOG::kDebug, __VA_ARGS__)
#define log_trace(...) log_ac(LOG::kTrace, __VA_ARGS__)
