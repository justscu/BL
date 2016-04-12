#ifndef __PARSE_MKTDT90_H__
#define __PARSE_MKTDT90_H__

#include <iostream>
#include "../../env.h"

typedef  int int32_t;

#define MKTDT_END_section    0x7C   // "|"
#define MKTDT_END_line       0x0A   // "\n"
#define MKTDT_point          0x2E   // "."
#define MKTDT_blank          0x20   // 空格

// 定义float，分为整数和小数部分
#define MKTDT_DECL_float(a, deci_len)     int32_t a##_inte, a##_deci, a##_deci_len = deci_len

struct mktdt_line_header {
	int32_t CheckSumLine;
	char    BeginString[6];
	char    Version[8];
	int32_t BodyLength;
	int32_t TotNumTradeReports;
	int32_t MDReportID;
	char    SenderCompID[6];
	char    MDTime[21];
	int32_t MDUpdateType;
	char    MDSesStatus[8];
};

// trailer
struct mktdt_line_trailer {
	int32_t CheckSumLine;
	char    EndString[7];
	char    CheckSum[3];
};

// MD001
struct mktdt_line_MD001 {
	int32_t     CheckSumLine;
	char        MDStreamID[5];
	char        SecurityID[6];
	char        Symbol[8];
	int32_t     TradeVolume;
	MKTDT_DECL_float(TotalValueTraded, 2);
	MKTDT_DECL_float(PreClosePx, 4);
	MKTDT_DECL_float(OpenPrice, 4);
	MKTDT_DECL_float(HighPrice, 4);
	MKTDT_DECL_float(LowPrice, 4);
	MKTDT_DECL_float(TradePrice, 4);
	MKTDT_DECL_float(ClosePx, 4);
	char TradingPhaseCode[8];
	char Timestamp[12];
};

// MD00X(2,3,4)
struct mktdt_line_MD00X {
	int32_t      CheckSumLine;
	char         MDStreamID[5];
	char         SecurityID[6];
	char         Symbol[8];
	int32_t      TradeVolume;
	MKTDT_DECL_float(TotalValueTraded, 2);
	MKTDT_DECL_float(PreClosePx, 3);
	MKTDT_DECL_float(OpenPrice, 3);
	MKTDT_DECL_float(HighPrice, 3);
	MKTDT_DECL_float(LowPrice, 3);
	MKTDT_DECL_float(TradePrice, 3);
	MKTDT_DECL_float(ClosePx, 3);
	MKTDT_DECL_float(BuyPrice1, 3);
	int32_t          BuyVolume1;
	MKTDT_DECL_float(SellPrice1, 3);
	int32_t(SellVolume1);
	MKTDT_DECL_float(BuyPrice2, 3);
	int32_t          BuyVolume2;
	MKTDT_DECL_float(SellPrice2, 3);
	int32_t(SellVolume2);
	MKTDT_DECL_float(BuyPrice3, 3);
	int32_t          BuyVolume3;
	MKTDT_DECL_float(SellPrice3, 3);
	int32_t(SellVolume3);
	MKTDT_DECL_float(BuyPrice4, 3);
	int32_t          BuyVolume4;
	MKTDT_DECL_float(SellPrice4, 3);
	int32_t(SellVolume4);
	MKTDT_DECL_float(BuyPrice5, 3);
	int32_t          BuyVolume5;
	MKTDT_DECL_float(SellPrice5, 3);
	int32_t(SellVolume5);
};

// MD002
struct mktdt_line_MD002 : public mktdt_line_MD00X {
	char TradingPhaseCode[8];
	char Timestamp[12];
};

// MD003
typedef mktdt_line_MD002 mktdt_line_MD003;

// MD004
struct mktdt_line_MD004 : public mktdt_line_MD00X {
	MKTDT_DECL_float(PreCloseOPV, 3); // only MD004
	MKTDT_DECL_float(IOPV, 3); // only MD004
	char             TradingPhaseCode[8];
	char             Timestamp[12];
};

// 行情接口-line
class mktdt_line {
public:
	// 解析成功时，返回true，每次解析一行
	// len为本次解析的长度(包含结束符的长度)
	bool parse(const char* src, int32_t& len, mktdt_line_header&  m);
	bool parse(const char* src, int32_t& len, mktdt_line_trailer& m);
	bool parse(const char* src, int32_t& len, mktdt_line_MD001&   m);
	bool parse(const char* src, int32_t& len, mktdt_line_MD002&   m);
	bool parse(const char* src, int32_t& len, mktdt_line_MD004&   m);

	// 对整个文件计算校验码
	bool checksum(const char* src, int32_t src_len, char* result);
	// 对一行计算校验码，行结尾符为0x0A
	// src_len  为src的长度
	// len      为本次校验的长度(包含结束符的长度)
	bool checksum(const char* src, int32_t src_len, int32_t& len, int32_t& result);
	inline
		bool find_end(const char* src, int32_t& len) {
		// 找到行结束符号
		while (src[len - 1] != MKTDT_END_line) {
			++len;
		}
		return true;
	}

	void print(const mktdt_line_header&  m);
	void print(const mktdt_line_trailer& m);
	void print(const mktdt_line_MD001&   m);
	void print(const mktdt_line_MD002&   m);
	void print(const mktdt_line_MD004&   m);
private:
	template<class T>
	bool parse(const char* src, int32_t& len, T& m);
	void print(const mktdt_line_MD00X&  m);
};


// test
void test_parse_mktdt90();
#endif // !__PARSE_MKTDT90_H__
