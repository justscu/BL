
#include <sys/time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <inttypes.h>
#include <stdarg.h>
#include "logger.h"
#include "format.h"

namespace LOG {

 void Format::Date() {
	char buf[64];
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	struct tm stTime;
	localtime_r(&tv.tv_sec, &stTime);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &stTime);
	// usec
	sprintf(buf+20, "%.6" PRId64 "", tv.tv_usec);
	str_.append(buf);
}
void Format::Level(const LOGLEVEL level) {
	switch (level) {
		case kBase     : str_.append("BASE"); break;
		case kTrace    : str_.append("TRAC"); break;
		case kDebug    : str_.append("DBUG"); break;
		case kInfo     : str_.append("INFO"); break;
		case kWarning  : str_.append("WARN"); break;
		case kError    : str_.append("EROR"); break;
		case kCritical : str_.append("CRIT"); break;
		default:         str_.append("----"); break;
	}
}
void Format::File(const char *file) {
	str_.append(file);
}
void Format::Line(uint16_t line) {
	std::ostringstream os;
	os << line;
	str_.append(os.str());
}
void Format::ProcessID() {
	std::ostringstream os;
	os << getpid();
	str_.append(os.str());
}
void Format::ThreadID() {
	std::ostringstream os;
	os << pthread_self();
	str_.append(os.str());
}
void Format::Function(const char *func) {
	str_.append(func);
}


std::string& FormatLog(std::string &str,
		               const std::string &format,
                       LOGLEVEL    level,
					   const char *file, 
                       const int   line, 
                       const char *func, 
                       const char *fmt, 
                       ...) {
	Format f(str);
	for (std::string::size_type i = 0; i < format.length(); ++i) {
		if (format[i] == '%') {
			i++;
			switch(format[i]) {
				case 'D': f.Date();          break;
				case 'L': f.Level(level);    break;
				case 'F': f.File(file);      break;
				case 'l': f.Line(line);      break;
				case 'T': f.ThreadID();      break;
				case 'P': f.ProcessID();     break;
				case 'f': f.Function(func);  break;
				case 'm': {
							const int msg_len = 8192;
							char buf[msg_len];
							va_list ap;
							va_start(ap, fmt);
							vsnprintf(buf, msg_len, fmt, ap);
							va_end(ap);
							str.append(buf);
				          } 
					break;
				default: {
							--i;
							str.append(1, '%');
						}
					break;
						 
			}
		} else {
			str.append(1, format[i]);
		}
	}
	str.append("\n");
	return str;
}

}
