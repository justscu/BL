#ifndef __PANIC_H__
#define __PANIC_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>


// usage:
//   Panic pa;
//   pa.panic(__FUNCTION__, "%s\n", "main_thread");
//

class Panic {
public:
    Panic();
    ~Panic();

    void panic(const char* funcname, const char* format, ...);
    void dump_stack(void);

private:
    int32_t vlog(const char* format, va_list ap);
    int32_t log(const char* format, ...);

    void dump_filename(char* buf);

private:
    const int32_t bksize = 256;
    FILE* f = nullptr; // backstack file pointer
};

#endif // __PANIC_H__
