#pragma once

#include <stdio.h>
#include <stdint.h>
#include <mutex>

extern std::mutex mutex;

#define log_err(format, ...) do { \
        mutex.lock(); \
        fprintf(stdout, format , ## __VA_ARGS__); \
        mutex.unlock(); \
    } while(0)


#define log_dbg(format, ...) do { \
        mutex.lock(); \
        fprintf(stdout, format, ## __VA_ARGS__); \
        mutex.unlock(); \
    } while(0)

#define log_info(format, ...) do { \
        mutex.lock(); \
        fprintf(stdout, format, ## __VA_ARGS__); \
        mutex.unlock(); \
    } while(0)
