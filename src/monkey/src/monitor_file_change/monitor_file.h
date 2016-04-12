#ifndef __MONITOR_FILE_H__
#define __MONITOR_FILE_H__

#include <time.h>
#include "../../env.h"
#include "parse_mktdt90.h"

// test
bool test_monitor_file(const char* filename);


struct mktdt_file_data {
	mktdt_line_header  _header[8];
	mktdt_line_trailer _trailer[8];
	mktdt_line_MD001   _md001[512];
	mktdt_line_MD002   _md002[2048];
	mktdt_line_MD003   _md003[2048];
	mktdt_line_MD004   _md004[2048];
};

//
class mktdt_file {
public:
	// 初始化mktdt_file_data结构
	bool Init(char* src, const int32_t src_len, mktdt_file_data& data);
	// 解析 & 更新
	// 并将发生变化的行，打印出来
	bool Parse(char* src, const int32_t src_len, mktdt_file_data& data);
	void print_time();
};


#endif /*__MONITOR_FILE_H__*/
