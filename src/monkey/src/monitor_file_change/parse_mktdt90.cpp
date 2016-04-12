#include <iomanip>
#include <stdio.h>
#include <math.h>
#include "parse_mktdt90.h"

// 解析int型
// 返回值为本次解析的长度（包括结束字符）
int32_t a2i(const char* src, int32_t& out) {
	out = 0;
	int32_t i = 0;
	// 空格
	while (src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		if (src[i] != MKTDT_blank) {
			break;
		}
		++i;
	}
	while (src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		out = out * 10 + (src[i++] - '0');
	}
	return i + 1;
}

// 解析float型，分成2部分：整数和小数
// 返回值为本次解析的长度（包括结束字符）
int32_t a2f(const char* src, int32_t& inte, int32_t& deci) {
	inte = deci = 0;
	int32_t i = 0;
	//
	while (src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		if (src[i] != MKTDT_blank) {
			break;
		}
		++i;
	}
	// integer
	while (src[i] != MKTDT_point && src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		inte = inte * 10 + (src[i++] - '0');
	}
	++i; // "."
		 // decimal
	while (src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		deci = deci * 10 + (src[i++] - '0');
	}
	return i + 1;
}

// 解析string型
// out     为解析出来的字符串
// out_size为用户提供的out的大小
// out_len 为输出的字符串长度
// 返回值为本次解析的长度（包括结束字符）
int32_t a2a(const char* src, char* out, const int32_t out_size, int32_t& out_len) {
	int32_t   i = 0;
	while (src[i] != MKTDT_END_section && src[i] != MKTDT_END_line) {
		out[i%out_size] = src[i];
		++i;
	}
	out_len = i;
	// 去掉末尾的空格
	while (out_len > 0 && out[(out_len - 1) % out_size] == MKTDT_blank) {
		--out_len;
	}
	return i + 1;
}


/////////////////////////////////////////////////////////////
// 设置字符串
#define MKTDT_SET_int32(name) ret = a2i(src+len, m.name); \
    if (ret > 0) { len += ret; } \
    else         { fprintf(stderr, "parse" #name "error! \n"); break;}

#define MKTDT_SET_float(name) ret = a2f(src+len, m.name##_inte, m.name##_deci); \
    if (ret > 0) { len += ret; } \
    else         { fprintf(stderr, "parse" #name "error! \n"); break;}

#define MKTDT_SET_char(name, size) ret = a2a(src+len, m.name, size, out_len); \
    if (ret > 0) { len += ret; \
        if (out_len < size - 1) m.name[out_len] = 0; \
    } else { fprintf(stderr, "parse" #name "error! \n"); break;}


// 获取float值
#define MKTDT_GET_float(name) (m.name##_inte + m.name##_deci/pow(10.0, m.name##_deci_len))
// print raw
#define MKTDT_PRT_char_raw(name,  width) std::cout << std::left  << std::setw(width) << std::setfill(' ') << std::string(m.name, width).c_str()
#define MKTDT_PRT_int32_raw(name, width) std::cout << std::right << std::setw(width) << std::setfill(' ') << m.name
#define MKTDT_PRT_float_raw(name, width) std::cout << std::right << std::setw(width) << std::setprecision(m.name##_deci_len) << std::setiosflags(std::ios::fixed) << std::setfill(' ') << MKTDT_GET_float(name)
// print
#define MKTDT_PRT_char(name,  width) MKTDT_PRT_char_raw(name,  width) << "|"
#define MKTDT_PRT_int32(name, width) MKTDT_PRT_int32_raw(name, width) << "|"
#define MKTDT_PRT_float(name, width) MKTDT_PRT_float_raw(name, width) << "|"
// print last
#define MKTDT_PRT_char_last(name,  width) MKTDT_PRT_char_raw(name,  width) << std::endl
#define MKTDT_PRT_int32_last(name, width) MKTDT_PRT_int32_raw(name, width) << std::endl
#define MKTDT_PRT_float_last(name, width) MKTDT_PRT_float_raw(name, width) << std::endl



//////////////////// header
// 解析成功时，返回true
// len为本次解析的长度
bool mktdt_line::parse(const char* src, int32_t& len, mktdt_line_header& m) {
	len = 0;
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		MKTDT_SET_char(BeginString, 6);
		MKTDT_SET_char(Version, 8);
		MKTDT_SET_int32(BodyLength);
		MKTDT_SET_int32(TotNumTradeReports);
		MKTDT_SET_int32(MDReportID);
		MKTDT_SET_char(SenderCompID, 6);
		MKTDT_SET_char(MDTime, 21);
		MKTDT_SET_int32(MDUpdateType);
		MKTDT_SET_char(MDSesStatus, 8);
		bSucc = true;
	} while (0);
	// 找到行结束符号
	find_end(src, len);
	return bSucc;
}

bool mktdt_line::parse(const char* src, int32_t& len, mktdt_line_trailer& m) {
	len = 0;
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		MKTDT_SET_char(EndString, 7);
		MKTDT_SET_char(CheckSum, 3);
		bSucc = true;
	} while (0);

	// 找到行结束符号
	find_end(src, len);
	return bSucc;
}

bool mktdt_line::parse(const char* src, int32_t& len, mktdt_line_MD001& m) {
	len = 0;
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		MKTDT_SET_char(MDStreamID, 5);
		MKTDT_SET_char(SecurityID, 6);
		MKTDT_SET_char(Symbol, 8);
		MKTDT_SET_int32(TradeVolume);
		MKTDT_SET_float(TotalValueTraded);
		MKTDT_SET_float(PreClosePx);
		MKTDT_SET_float(OpenPrice);
		MKTDT_SET_float(HighPrice);
		MKTDT_SET_float(LowPrice);
		MKTDT_SET_float(TradePrice);
		MKTDT_SET_float(ClosePx);
		MKTDT_SET_char(TradingPhaseCode, 8);
		MKTDT_SET_char(Timestamp, 12);
		bSucc = true;
	} while (0);
	// 找到行结束符号
	find_end(src, len);
	return bSucc;
}

template<class T>
bool mktdt_line::parse(const char* src, int32_t& len, T& m) {
	len = 0;
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		MKTDT_SET_char(MDStreamID, 5);
		MKTDT_SET_char(SecurityID, 6);
		MKTDT_SET_char(Symbol, 8);
		MKTDT_SET_int32(TradeVolume);
		MKTDT_SET_float(TotalValueTraded);
		MKTDT_SET_float(PreClosePx);
		MKTDT_SET_float(OpenPrice);
		MKTDT_SET_float(HighPrice);
		MKTDT_SET_float(LowPrice);
		MKTDT_SET_float(TradePrice);
		MKTDT_SET_float(ClosePx);
		MKTDT_SET_float(BuyPrice1);
		MKTDT_SET_int32(BuyVolume1);
		MKTDT_SET_float(SellPrice1);
		MKTDT_SET_int32(SellVolume1);
		MKTDT_SET_float(BuyPrice2);
		MKTDT_SET_int32(BuyVolume2);
		MKTDT_SET_float(SellPrice2);
		MKTDT_SET_int32(SellVolume2);
		MKTDT_SET_float(BuyPrice3);
		MKTDT_SET_int32(BuyVolume3);
		MKTDT_SET_float(SellPrice3);
		MKTDT_SET_int32(SellVolume3);
		MKTDT_SET_float(BuyPrice4);
		MKTDT_SET_int32(BuyVolume4);
		MKTDT_SET_float(SellPrice4);
		MKTDT_SET_int32(SellVolume4);
		MKTDT_SET_float(BuyPrice5);
		MKTDT_SET_int32(BuyVolume5);
		MKTDT_SET_float(SellPrice5);
		MKTDT_SET_int32(SellVolume5);
		bSucc = true;
	} while (0);

	return bSucc;
}

bool mktdt_line::parse(const char* src, int32_t& len, mktdt_line_MD002& m) {
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		if (!parse<mktdt_line_MD00X>(src, len, m)) break;
		MKTDT_SET_char(TradingPhaseCode, 8);
		MKTDT_SET_char(Timestamp, 12);
		bSucc = true;
	} while (0);
	// 找到行结束符号
	find_end(src, len);
	return bSucc;
}

bool mktdt_line::parse(const char* src, int32_t& len, mktdt_line_MD004& m) {
	int32_t ret = 0;
	int32_t out_len = 0;
	bool    bSucc = false;
	do {
		if (!parse<mktdt_line_MD00X>(src, len, m)) break;
		MKTDT_SET_float(PreCloseOPV);
		MKTDT_SET_float(IOPV);
		MKTDT_SET_char(TradingPhaseCode, 8);
		MKTDT_SET_char(Timestamp, 12);
		bSucc = true;
	} while (0);
	// 找到行结束符号
	find_end(src, len);
	return bSucc;
}

bool mktdt_line::checksum(const char* src, int32_t src_len, char* result) {
	char sum = 0;
	for (int32_t i = 0; i < src_len; ++i) {
		sum += src[i];
	}
	result[2] = (sum & 0xFF) % 10 + '0';
	result[1] = ((sum & 0xFF) / 10) % 10 + '0';
	result[0] = (sum & 0xFF) / 100 + '0';
	return true;
}

bool mktdt_line::checksum(const char* src, int32_t src_len, int32_t& len, int32_t& result) {
	len = 0;
	result = 0;
	for (; len < src_len && src[len] != MKTDT_END_line; ++len) {
		//result += (src[len] & 0xFF);
		result ^= (src[len] ^ 0xFF);
	}
	++len;
	return true;
}

void mktdt_line::print(const mktdt_line_header& m) {
	MKTDT_PRT_int32(CheckSumLine, 6);
	MKTDT_PRT_char(BeginString, 6);
	MKTDT_PRT_char(Version, 8);
	MKTDT_PRT_int32(BodyLength, 10);
	MKTDT_PRT_int32(TotNumTradeReports, 5);
	MKTDT_PRT_int32(MDReportID, 8);
	MKTDT_PRT_char(SenderCompID, 6);
	MKTDT_PRT_char(MDTime, 21);
	MKTDT_PRT_int32(MDUpdateType, 1);
	MKTDT_PRT_char_last(MDSesStatus, 8);
}

void mktdt_line::print(const mktdt_line_trailer& m) {
	MKTDT_PRT_int32(CheckSumLine, 6);
	MKTDT_PRT_char(EndString, 7);
	MKTDT_PRT_char_last(CheckSum, 3);
}

void mktdt_line::print(const mktdt_line_MD001& m) {
	MKTDT_PRT_int32(CheckSumLine, 6);
	MKTDT_PRT_char(MDStreamID, 5);
	MKTDT_PRT_char(SecurityID, 6);
	MKTDT_PRT_char(Symbol, 8);
	MKTDT_PRT_int32(TradeVolume, 16);
	MKTDT_PRT_float(TotalValueTraded, 16);
	MKTDT_PRT_float(PreClosePx, 11);
	MKTDT_PRT_float(OpenPrice, 11);
	MKTDT_PRT_float(HighPrice, 11);
	MKTDT_PRT_float(LowPrice, 11);
	MKTDT_PRT_float(TradePrice, 11);
	MKTDT_PRT_float(ClosePx, 11);
	MKTDT_PRT_char(TradingPhaseCode, 8);
	MKTDT_PRT_char_last(Timestamp, 12);
}

void mktdt_line::print(const mktdt_line_MD00X& m) {
	MKTDT_PRT_int32(CheckSumLine, 6);
	MKTDT_PRT_char(MDStreamID, 5);
	MKTDT_PRT_char(SecurityID, 6);
	MKTDT_PRT_char(Symbol, 8);
	MKTDT_PRT_int32(TradeVolume, 16);
	MKTDT_PRT_float(TotalValueTraded, 16);
	MKTDT_PRT_float(PreClosePx, 11);
	MKTDT_PRT_float(OpenPrice, 11);
	MKTDT_PRT_float(HighPrice, 11);
	MKTDT_PRT_float(LowPrice, 11);
	MKTDT_PRT_float(TradePrice, 11);
	MKTDT_PRT_float(ClosePx, 11);
	MKTDT_PRT_float(BuyPrice1, 11);
	MKTDT_PRT_int32(BuyVolume1, 12);
	MKTDT_PRT_float(SellPrice1, 11);
	MKTDT_PRT_int32(SellVolume1, 12);
	MKTDT_PRT_float(BuyPrice2, 11);
	MKTDT_PRT_int32(BuyVolume2, 12);
	MKTDT_PRT_float(SellPrice2, 11);
	MKTDT_PRT_int32(SellVolume2, 12);
	MKTDT_PRT_float(BuyPrice3, 11);
	MKTDT_PRT_int32(BuyVolume3, 12);
	MKTDT_PRT_float(SellPrice3, 11);
	MKTDT_PRT_int32(SellVolume3, 12);
	MKTDT_PRT_float(BuyPrice4, 11);
	MKTDT_PRT_int32(BuyVolume4, 12);
	MKTDT_PRT_float(SellPrice4, 11);
	MKTDT_PRT_int32(SellVolume4, 12);
	MKTDT_PRT_float(BuyPrice5, 11);
	MKTDT_PRT_int32(BuyVolume5, 12);
	MKTDT_PRT_float(SellPrice5, 11);
	MKTDT_PRT_int32(SellVolume5, 12);
}

void mktdt_line::print(const mktdt_line_MD002& m) {
	print(mktdt_line_MD00X(m));
	MKTDT_PRT_char(TradingPhaseCode, 8);
	MKTDT_PRT_char_last(Timestamp, 12);
}

void mktdt_line::print(const mktdt_line_MD004& m) {
	print(mktdt_line_MD00X(m));
	MKTDT_PRT_float(PreCloseOPV, 11);
	MKTDT_PRT_float(IOPV, 11);
	MKTDT_PRT_char(TradingPhaseCode, 8);
	MKTDT_PRT_char_last(Timestamp, 12);
}

///////////////////// 测试程序
#pragma warning(disable :4996)
void test_parse_mktdt90() {
	const char* filename = "/home/ll/Desktop/mktdt90.txt";
	const int32_t size = 1024 * 1024 * 4; // 4M
	char* buf = new char[size];
	FILE* pfile = fopen(filename, "r");
	if (NULL == pfile) {
		delete[] buf;
		fprintf(stderr, "open file error. \n");
		return;
	}
	int32_t rsize = (int32_t)fread(buf, sizeof(char), size, pfile);
	fprintf(stdout, "read size[%d] \n", rsize);

	char checkresult[3];
	mktdt_line  mktdt90;
	mktdt90.checksum(buf, rsize - 4, checkresult);

	int32_t pos = 0;
	int32_t len = 0;
	mktdt_line_header header;
	if (!mktdt90.parse(buf, len, header)) {
		return;
	}
	pos += len;
	mktdt90.print(header);

	mktdt_line_MD001 md001;
	for (int i = 2; i <= 276; ++i) {
		if (!mktdt90.parse(buf + pos, len, md001)) {
			return;
		}
		pos += len;
		mktdt90.print(md001);
	}

	mktdt_line_MD002 md002;
	for (int i = 277; i <= 1290; ++i) {
		if (!mktdt90.parse(buf + pos, len, md002)) {
			return;
		}
		pos += len;
		mktdt90.print(md002);
	}

	mktdt_line_MD003 md003;
	for (int i = 1291; i <= 2690; ++i) {
		if (!mktdt90.parse(buf + pos, len, md003)) {
			return;
		}
		pos += len;
		mktdt90.print(md003);
	}

	mktdt_line_MD004 md004;
	for (int i = 2691; i <= 3003; ++i) {
		if (!mktdt90.parse(buf + pos, len, md004)) {
			return;
		}
		pos += len;
		mktdt90.print(md004);
	}

	mktdt_line_trailer mdtail;
	if (!mktdt90.parse(buf + pos, len, mdtail)) {
		return;
	}
	pos += len;
	mktdt90.print(mdtail);

	if (strncmp(checkresult, mdtail.CheckSum, 3) == 0) {
		std::cout << "checkresult: same." << std::endl;
	}

	fclose(pfile);
	delete[] buf;
}
