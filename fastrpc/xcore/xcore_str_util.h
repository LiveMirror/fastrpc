// 2009-02-04
// xcore_str_util.h
// 
// 包装一些方便字符串操作的函数


#ifndef _XCORE_STR_UTIL_H_
#define _XCORE_STR_UTIL_H_

#include "xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XStrUtil
///////////////////////////////////////////////////////////////////////////////
class XStrUtil
{
public:
	// 去除字符串头(或尾)中在字符集中指定的字符
	static string& chop_head(string &strSrc, const char *pcszCharSet = " \t\r\n");
	static string& chop_tail(string &strSrc, const char *pcszCharSet = " \t\r\n");
	static string& chop(string &strSrc, const char *pcszCharSet = " \t\r\n");

	// 字符串转大写(或小写)
	static void to_upper(char *pszSrc);
	static void to_lower(char *pszSrc);
	static void to_upper(string &strSrc);
	static void to_lower(string &strSrc);

	// 替换
	static void replace(char* str, char oldch, char newch);
	static void replace(char* str, const char* oldCharSet, char newch);

	// 区分大小写比较
	static int compare(const char* pszSrc1, const char* pszSrc2, int length = -1);
	static int compare(const string &str1, const string &str2, int length = -1);

	// 不区分大小写比较
	static int compare_nocase(const char* pszSrc1, const char* pszSrc2, int length = -1);
	static int compare_nocase(const string &str1, const string &str2, int length = -1);

	// 根据字符集中指定的分隔字符分解源字符串,并放置到vector中
	// nMaxCount指定期望得到的行数,解析到maxCount将终止并返回,不会继续解析;设为-1表示解析所有
	static uint32 split(const string &strSrc, vector<string>& vItems, const char *pcszCharSet = " \r\n\t", int nMaxCount = -1);

	// 字符串转整数
	static bool	  to_int(const string &strSrc, int &nValue, int radix = 10);
	static int	  to_int_def(const string &strSrc, int def = -1, int radix = 10);
	static int	  try_to_int_def(const string &strSrc, int def = -1, int radix = 10);
	static bool	  to_uint(const string &strSrc, uint32 &uValue, int radix = 10);
	static uint32 to_uint_def(const string &strSrc, uint32 def = 0, int radix = 10);
	static uint32 try_to_uint_def(const string &strSrc, uint32 def = 0, int radix = 10);

	// 字符串转浮点型数
	static bool   to_float(const string &strSrc, double &value);
	static double to_float_def(const string &strSrc, double def = 0.0);
	static double try_to_float_def(const string &strSrc, double def = 0.0);

	// 数值转字符串
	static string to_str(int nVal, const char* cpszFormat = NULL/*"%d"*/);
	static string to_str(uint32 uVal, const char* cpszFormat = NULL/*"%u"*/);
	static string to_str(int64 nlVal, const char* cpszFormat = NULL/*"%lld"*/);
	static string to_str(uint64 ulVal, const char* cpszFormat = NULL/*"%llu"*/);
	static string to_str(double fVal, const char* cpszFormat = NULL/*"%f"*/);

	// string hash
	static uint32 hash_code(const char* str);
	static uint32 murmur_hash(const void* key, uint32 len);

	// dump data
	static void dump(string& result, const void* pdata, uint32 length);
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_STR_UTIL_H_
