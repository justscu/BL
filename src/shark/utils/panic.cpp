#include <execinfo.h>
#include <time.h>
#include <stdlib.h>
#include "panic.h"

Panic::Panic() {
    char buf[128];
    dump_filename(buf);
    f = fopen(buf, "a+");
}

Panic::~Panic() {
    if (f) {
        fclose(f);
        f = nullptr;
    }
}

void Panic::panic(const char* funcname, const char* format, ...) {
    va_list ap;
    log("PANIC in %s():\n", funcname);
    va_start(ap, format);
    vlog(format, ap);
    va_end(ap);

    dump_stack();
    abort(); // generate a coredump if enabled.
}

void Panic::dump_stack(void) {
    void* func[bksize];
    int32_t size = backtrace(func, bksize);
    char**  symb = backtrace_symbols(func, size);

    if (symb == nullptr) return;

    // print stack info.
    while (size > 0) {
        log("%d: [%s]\n", size, symb[size-1]);
        --size;
    }

    free(symb);
}

// buf: yyyymmdd-hhmmss
void Panic::dump_filename(char* buf) {
    time_t t;
    time(&t);

    struct tm* p = localtime(&t);
    sprintf(buf, "./backstrace.log-%.4d%.2d%.2d-%.2d%.2d%.2d",
            p->tm_year+1900, p->tm_mon+1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
}

int32_t Panic::vlog(const char* format, va_list ap) {
    if (!f) {
        f = stderr;
    }

    int32_t ret = vfprintf(f, format, ap);
    fflush(f);
    return ret;
}

int32_t Panic::log(const char* format, ...) {
    va_list ap;

    va_start(ap, format);
    int32_t ret = vlog(format, ap);
    va_end(ap);
    return ret;
}
