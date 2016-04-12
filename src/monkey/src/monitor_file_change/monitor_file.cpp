#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "../../env.h"
#include "monitor_file.h"


#define XX(name, value) \
    if (0 == strncmp(src+pos, #name, strlen(#name))) { \
        dtline.parse(src+pos, len, data.value[i##value]); \
        pos += len; \
        ++i##value; \
    }

// 初始化mktdt_file_data结构
bool mktdt_file::Init(char* src, const int32_t src_len, mktdt_file_data& data) {
	int32_t pos = 0;
	int32_t len = 0;
	int32_t i_header = 0, i_md001 = 0, i_md002 = 0, i_md003 = 0, i_md004 = 0, i_trailer = 0;
	mktdt_line dtline;
	// header
	XX(HEADER, _header);
	// 6 为标识符长度
	while (pos + 6 < src_len) {
		XX(MD001, _md001)
else XX(MD002, _md002)
		else XX(MD003, _md003)
		else XX(MD004, _md004)
		else XX(TRAILER, _trailer)
		else {
		char pbuf[32];
		snprintf(pbuf, sizeof(pbuf) - 1, "%s", src + pos);
		fprintf(stdout, "err: %s \n", pbuf);
		dtline.find_end(src, len);
		}
	}

	return true;
}
#undef XX

#define XX(name, value) \
    if (0 == strncmp(src+pos, #name, strlen(#name))) { \
        if (dtline.checksum(src+pos, src_len-pos, len, CheckSumLine)) { \
            if (CheckSumLine != data.value[i##value].CheckSumLine) { \
				dtline.print(data.value[i##value]); \
				data.value[i##value].CheckSumLine = CheckSumLine; \
                dtline.parse(src+pos, len, data.value[i##value]);\
                dtline.print(data.value[i##value]); \
            } \
        } \
        pos += len; ++i##value; \
    }

// 解析 & 更新
// 并将发生变化的行，打印出来
bool mktdt_file::Parse(char* src, const int32_t src_len, mktdt_file_data& data) {
	int32_t pos = 0;
	int32_t len = 0;
	int32_t i_header = 0, i_md001 = 0, i_md002 = 0, i_md003 = 0, i_md004 = 0, i_trailer = 0;
	int32_t CheckSumLine; // 用来存储每行的校验码
	mktdt_line dtline;
	// header
	XX(HEADER, _header);
	// 6 为标识符长度
	while (pos + 6 < src_len) {
		XX(MD001, _md001)
else XX(MD002, _md002)
else XX(MD003, _md003)
		else XX(MD004, _md004)
		else XX(TRAILER, _trailer)
		else {
		char pbuf[32];
		snprintf(pbuf, sizeof(pbuf) - 1, "%s", src + pos);
		fprintf(stdout, "err: %s \n", pbuf);
		dtline.find_end(src, len);
		}
	}

	return true;
}
#undef XX

#pragma warning(disable :4996)
void mktdt_file::print_time() {
	char buf[64];
	time_t t = time(NULL);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
	fprintf(stdout, "%s", buf);
}


bool test_monitor_file(const char* filename) {
	FILE* pFile = fopen(filename, "r");
	if (NULL == pFile) {
		fprintf(stderr, "fopen[%s] error[%s]\n", filename, strerror(errno));
		return false;
	}

	const int32_t size = 1024 * 1024 * 4; // 4M
	char* buf = new char[size];
	mktdt_file mf;
	mktdt_file_data *mf_data = new mktdt_file_data;
	int32_t rsize = (int32_t)fread(buf, sizeof(char), size, pFile);
	fprintf(stdout, "read size[%d] \n", rsize);
	mf.Init(buf, rsize, *mf_data); // init

#ifdef __linux__
	timespec newtime;
#else
	time_t newtime = 0;
#endif

	struct stat st; 
	
	while (true) {
		if (0 != fstat(fileno(pFile), &st)) {
			delete[] buf;
			fprintf(stderr, "fstat error[%s]\n", strerror(errno));
			fclose(pFile);
			return false;;
		}
		// 文件更新了
#ifdef __linux__
		if (newtime.tv_nsec != st.st_mtim.tv_nsec || newtime.tv_sec != st.st_mtim.tv_sec) {
			newtime.tv_nsec = st.st_mtim.tv_nsec;
			newtime.tv_sec = st.st_mtim.tv_sec;
			mf.print_time();
			fseek(pFile, 0, SEEK_SET);
			rsize = fread(buf, sizeof(char), size, pFile);
			fprintf(stdout, "file change read size[%d] \n", rsize);
			mf.Parse(buf, rsize, mf_data);
			continue;
		}
		sleep(1);
#endif

#ifdef _WIN32
		if (newtime != st.st_mtime) {
			newtime = st.st_mtime;
			mf.print_time();
			fseek(pFile, 0, SEEK_SET);
			rsize = (int32_t)fread(buf, sizeof(char), size, pFile);
			fprintf(stdout, "file change read size[%d] \n", rsize);
			mf.Parse(buf, rsize, *mf_data);
			fprintf(stdout, "\n");
			continue;
		}
		Sleep(10);
#endif
	}

	delete []buf;
	delete []mf_data;
	fclose(pFile);
	return true;
}
