// 2009-02-04
// xcore_str_util.cpp
// 
// 包装一些方便字符串操作的函数


#include "xcore_str_util.h"
#include <math.h>

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XStrUtil
///////////////////////////////////////////////////////////////////////////////
string& XStrUtil::chop_head(string &strSrc, const char *pcszCharSet)
{
	if (pcszCharSet == NULL) return strSrc;
	size_t pos = strSrc.find_first_not_of(pcszCharSet);
	return strSrc.erase(0, pos);
}

string& XStrUtil::chop_tail(string &strSrc, const char *pcszCharSet)
{
	if (pcszCharSet == NULL) return strSrc;
	size_t pos = strSrc.find_last_not_of(pcszCharSet);
	if (pos == string::npos)
	{
		strSrc.clear();
		return strSrc;
	}
	return strSrc.erase(++pos);
}

string& XStrUtil::chop(string &strSrc, const char *pcszCharSet)
{
	chop_head(strSrc, pcszCharSet);
	return chop_tail(strSrc, pcszCharSet);
}

void XStrUtil::to_upper(char *pszSrc)
{
	if (pszSrc == NULL) return;
	char *pos = (char *)pszSrc;
	char diff = 'A' - 'a';
	while (*pos != '\0')
	{
		if ('a' <= *pos && *pos <= 'z')
		{
			*pos += diff;
		}
		pos++;
	}
	return;
}

void XStrUtil::to_lower(char *pszSrc)
{
	if (pszSrc == NULL) return;
	char *pos = (char *)pszSrc;
	char diff = 'A' - 'a';
	while (*pos != '\0')
	{
		if ('A' <= *pos && *pos <= 'Z')
		{
			*pos -= diff;
		}
		pos++;
	}
	return;
}

void XStrUtil::to_lower(string &strSrc)
{
	return to_lower((char *)strSrc.c_str());
}

void XStrUtil::to_upper(string &strSrc)
{
	return to_upper((char *)strSrc.c_str());
}

void XStrUtil::replace(char* str, char oldch, char newch)
{
	if (str == NULL || oldch == newch) return;
	char* pos = str;
	while (*pos) // (*pos != '\0')
	{
		if (*pos == oldch) *pos = newch;
		pos++;
	}
	return;
}

void XStrUtil::replace(char* str, const char* oldCharSet, char newch)
{
	if (str == NULL || oldCharSet == NULL) return;
	char* pos = str;
	const char* p = NULL;
	while (*pos) // (*pos != '\0')
	{
		for (p = oldCharSet; *p; p++)
		{
			if (*pos == *p)
			{
				*pos = newch;
				break;
			}
		}
		pos++;
	}
	return;
}

int XStrUtil::compare(const char* pszSrc1, const char* pszSrc2, int length)
{
	ASSERT(pszSrc1 && pszSrc2);

	int ret = 0;
	const char *left = pszSrc1;
	const char *right = pszSrc2;

	while ((length != 0) && (*left != '\0') && (*right != '\0'))
	{
		if (length > 0) length--;
		ret = *left++ - *right++;
		if (ret != 0) return ret;
	}
	if (length == 0) return 0;
	return (*left - *right);
}

int XStrUtil::compare(const string &str1, const string &str2, int length)
{
	return compare(str1.c_str(), str2.c_str(), length);
}

int XStrUtil::compare_nocase(const char* pszSrc1, const char* pszSrc2, int length)
{
	ASSERT(pszSrc1 && pszSrc2);

	int ret = 0;
	const char *left = pszSrc1;
	const char *right = pszSrc2;

	while ((length != 0) && (*left != '\0') && (*right != '\0'))
	{
		if (length > 0) length--;
		ret = ::tolower(*left++) - ::tolower(*right++);
		if (ret != 0) return ret;
	}
	if (length == 0) return 0;
	return (*left - *right);
}

int XStrUtil::compare_nocase(const string &str1, const string &str2, int length/* = -1*/)
{
	return compare_nocase(str1.c_str(), str2.c_str(), length);
}

uint32 XStrUtil::split(const string &strSrc, vector<string> &vItems, const char *pcszCharSet/* = " \r\n\t"*/, int nMaxCount/* = -1*/)
{
	vItems.clear();

	size_t pos_begin = 0;
	size_t pos_end = 0;
	int count = 0;
	while (pos_end != string::npos)
	{
		pos_begin = strSrc.find_first_not_of(pcszCharSet, pos_end);
		if (pos_begin == string::npos) break;
		pos_end = strSrc.find_first_of(pcszCharSet, pos_begin);
		string strTmp(strSrc, pos_begin, pos_end - pos_begin);
		if (!strTmp.empty())
		{
			count++;
			vItems.push_back(strTmp);
		}
		if (nMaxCount > 0 && count >= nMaxCount)
		{
			break;
		}
	}
	return (uint32)vItems.size();
}

bool XStrUtil::to_int(const string &strSrc, int &nValue, int radix/* = 10*/)
{
	char* endPtr = 0;
	string str = strSrc;
	
	chop(str);
	if (str.empty()) return false;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

int XStrUtil::to_int_def(const string &strSrc, int def/* = -1*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	int nValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return nValue;
}

int XStrUtil::try_to_int_def(const string &strSrc, int def/* = -1*/, int radix/* = 10*/)
 {
	char* endPtr = 0;
	int nValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return nValue;
}

bool XStrUtil::to_uint(const string &strSrc, uint32 &uValue, int radix/* = 10*/)
{
	char* endPtr = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return false;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

uint32 XStrUtil::to_uint_def(const string &strSrc, uint32 def/* = 0*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	uint32 uValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return uValue;
}

uint32 XStrUtil::try_to_uint_def(const string &strSrc, uint32 def/* = 0*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	uint32 uValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return uValue;
}

bool XStrUtil::to_float(const string &strSrc, double &value)
{
	char* endPtr = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return false;

	errno = 0;
	value = strtod(str.c_str(), &endPtr);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

double XStrUtil::to_float_def(const string &strSrc, double def/* = 0.0*/)
{
	char* endPtr = 0;
	double fValue = 0.0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	fValue = strtod(str.c_str(), &endPtr);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return def;
	return fValue;
}

double	XStrUtil::try_to_float_def(const string &strSrc, double def/* = 0.0*/)
{
	char* endPtr = 0;
	double fValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	fValue = strtod(str.c_str(), &endPtr);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return def;
	return fValue;
}

string XStrUtil::to_str(int nVal, const char* cpszFormat)
{
	char buf[128];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 100) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, nVal);
	else
		sprintf(buf, "%d", nVal);
	return buf;
}

string XStrUtil::to_str(uint32 uVal, const char* cpszFormat)
{
	char buf[128];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 100) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, uVal);
	else
		sprintf(buf, "%u", uVal);
	return buf;
}

string XStrUtil::to_str(int64 nlVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, nlVal);
	else
		sprintf(buf, "%lld", (long long int)nlVal);
	return buf;
}

string XStrUtil::to_str(uint64 ulVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, ulVal);
	else
		sprintf(buf, "%llu", (long long int)ulVal);
	return buf;
}

string XStrUtil::to_str(double fVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, fVal);
	else
		sprintf(buf, "%f", fVal);
	return buf;
}

uint32 XStrUtil::hash_code(const char* str)
{
	if (str == NULL) return 0;

	uint32 h = 0;
	while(*str)
	{
		h = 31 * h + (*str++);
	}
	return h;
}

uint32 XStrUtil::murmur_hash(const void* key, uint32 len)
{
	if (key == NULL) return 0;

	const uint32 m = 0X5BD1E995;
	const uint32 r = 24;
	const uint32 seed = 97;
	uint32 h = seed ^ len;

	// Mix 4 bytes at a time into the hash
	const BYTE* data = (const BYTE *)key;
	while(len >= 4)
	{
		uint32 k = *(uint32 *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

static char __dump_view(char ch)
{
	if (ch <= 31 || ch >= 127) return '.';
	return ch;
}

void XStrUtil::dump(string& result, const void* pdata, uint32 length)
{
	result.clear();
	if (pdata == NULL || length == 0) return;

	char buf[128];
	const uint8* src = (const uint8*)pdata;
	for ( ; length >= 16; length -= 16, src += 16)
	{
		sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X    ",
				src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
				src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15]);
		result += buf;

		sprintf(buf, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n",
				__dump_view(src[0]), __dump_view(src[1]), __dump_view(src[2]), __dump_view(src[3]),
				__dump_view(src[4]), __dump_view(src[5]), __dump_view(src[6]), __dump_view(src[7]),
				__dump_view(src[8]), __dump_view(src[9]), __dump_view(src[10]), __dump_view(src[11]),
				__dump_view(src[12]), __dump_view(src[13]), __dump_view(src[14]), __dump_view(src[15]));
		result += buf;
	}

	for (uint32 i = 0; i < length; i++)
	{
		sprintf(buf, "%02X ", src[i]);
		result += buf;
	}
	if (length % 16) result.append((16 - length) * 3 + 3, ' ');

	for (uint32 i = 0; i < length; i++)
	{
		result += __dump_view(src[i]);
	}
	if (length % 16) result += "\r\n";
	return;
}

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_str_util()
{
	string str = XStrUtil::to_str(5, "this is:%d\n");
	ASSERT(str == "this is:5\n");
	printf("%s", str.c_str());

	string result;
	XStrUtil::dump(result, str.c_str(), 321);
	printf("str dump is:\n%s", result.c_str());
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
