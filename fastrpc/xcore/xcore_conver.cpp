// 2011-10-11
// xcore_conver.cpp
// 
// 字符集转换

#include "xcore_conver.h"

namespace xcore {
	
///////////////////////////////////////////////////////////////////////////////
// character set conversion functions
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__
bool gbk_to_utf8(string& result, const string& str)
{
	// Convert to Unicode.
	int nULen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if (nULen <= 0) return false;
	wstring wideStr_(nULen, 0);
	int nULen2 = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), (LPWSTR)wideStr_.c_str(), nULen);
	if (nULen != nULen2) return false;

	// Convert to UTF-8.
	int nUTF8Len = ::WideCharToMultiByte(CP_UTF8, 0, wideStr_.c_str(), wideStr_.size(), NULL, 0, NULL, NULL);
	if (nUTF8Len <= 0) return false;
	result.assign(nUTF8Len, 0);
	int nUTF8Len2 = ::WideCharToMultiByte(CP_UTF8, 0, wideStr_.c_str(), wideStr_.size(), (LPSTR)result.c_str(), nUTF8Len, NULL, NULL);
	if (nUTF8Len != nUTF8Len2) return false;
	return true;
}

bool gbk_to_unicode(wstring& result, const string& str)
{
	// Convert to Unicode.
	int nULen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if (nULen <= 0) return false;
	wstring wideStr_(nULen, 0);
	result.assign(nULen, 0);
	int nULen2 = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), (LPWSTR)result.c_str(), nULen);
	if (nULen != nULen2) return false;
	return true;
}

bool utf8_to_gbk(string& result, const string& str)
{
	// Convert to Unicode.
	int nULen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
	if (nULen <= 0) return false;
	wstring wideStr_(nULen, 0);
	int nULen2 = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), (LPWSTR)wideStr_.c_str(), nULen);
	if (nULen != nULen2) return false;

	// Convert to UTF-8.
	int nUTF8Len = ::WideCharToMultiByte(CP_ACP, 0, wideStr_.c_str(), wideStr_.size(), NULL, 0, NULL, NULL);
	if (nUTF8Len <= 0) return false;
	result.assign(nUTF8Len, 0);
	int nUTF8Len2 = ::WideCharToMultiByte(CP_ACP, 0, wideStr_.c_str(), wideStr_.size(), (LPSTR)result.c_str(), nUTF8Len, NULL, NULL);
	if (nUTF8Len != nUTF8Len2) return false;
	return true;
}

bool utf8_to_unicode(wstring& result, const string& str)
{
	// Convert to Unicode.
	int nULen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
	if (nULen <= 0) return false;
	result.assign(nULen, 0);
	int nULen2 = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), (LPWSTR)result.c_str(), nULen);
	if (nULen != nULen2) return false;
	return true;
}

bool unicode_to_gbk(string& result, const wstring& str)
{
	// Convert to UTF-8.
	int nUTF8Len = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	if (nUTF8Len <= 0) return false;
	result.assign(nUTF8Len, 0);
	int nUTF8Len2 = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), (LPSTR)result.c_str(), nUTF8Len, NULL, NULL);
	if (nUTF8Len != nUTF8Len2) return false;
	return true;
}

bool unicode_to_utf8(string& result, const wstring& str)
{
	// Convert to UTF-8.
	int nUTF8Len = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	if (nUTF8Len <= 0) return false;
	result.assign(nUTF8Len, 0);
	int nUTF8Len2 = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), (LPSTR)result.c_str(), nUTF8Len, NULL, NULL);
	if (nUTF8Len != nUTF8Len2) return false;
	return true;
}
#endif//__WINDOWS__

#ifdef __GNUC__
bool gbk_to_utf8(string& result, const string& str)
{
	result.clear();

	iconv_t icd = iconv_open("UTF-8", "GBK");
	if (icd == (iconv_t)-1) return false;

	char buf[1024];
	size_t nSrcLen = str.size();
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 1024;
		char* out_ = buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, 1024 - nDstLen);
	}
	iconv_close(icd);
	return true;
}

bool gbk_to_unicode(wstring& result, const string& str)
{
	result.clear();

	iconv_t icd = iconv_open("UNICODE", "GBK");
	if (icd == (iconv_t)-1) return false;

	wchar_t buf[1024];
	size_t nSrcLen = str.size();
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 2048;
		char* out_ = (char*)buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, (2048 - nDstLen) / 2);
	}
	iconv_close(icd);
	return true;
}

bool utf8_to_gbk(string& result, const string& str)
{
	result.clear();

	iconv_t icd = iconv_open("GBK", "UTF-8");
	if (icd == (iconv_t)-1) return false;

	char buf[1024];
	size_t nSrcLen = str.size();
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 1024;
		char* out_ = buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, 1024 - nDstLen);
	}
	iconv_close(icd);
	return true;
}

bool utf8_to_unicode(wstring& result, const string& str)
{
	result.clear();

	iconv_t icd = iconv_open("UNICODE", "UTF-8");
	if (icd == (iconv_t)-1) return false;

	wchar_t buf[1024];
	size_t nSrcLen = str.size();
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 2048;
		char* out_ = (char*)buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, (2048 - nDstLen) / 2);
	}
	iconv_close(icd);
	return true;
}

bool unicode_to_gbk(string& result, const wstring& str)
{
	result.clear();

	iconv_t icd = iconv_open("GBK", "UNICODE");
	if (icd == (iconv_t)-1) return false;

	char buf[1024];
	size_t nSrcLen = str.size() * 2;
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 1024;
		char* out_ = (char*)buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, 1024 - nDstLen);
	}
	iconv_close(icd);
	return true;
}

bool unicode_to_utf8(string& result, const wstring& str)
{
	result.clear();

	iconv_t icd = iconv_open("UTF-8", "UNICODE");
	if (icd == (iconv_t)-1) return false;

	char buf[1024];
	size_t nSrcLen = str.size() * 2;
	char* in_ = (char *)str.c_str();
	while (nSrcLen > 0)
	{
		size_t nDstLen = 1024;
		char* out_ = (char*)buf;
		size_t ret = iconv(icd, &in_, &nSrcLen, &out_, &nDstLen);
		if (ret == (size_t)-1 && errno != E2BIG)
		{
			iconv_close(icd);
			return false;
		}
		result.append(buf, 1024 - nDstLen);
	}
	iconv_close(icd);
	return true;
}

#endif//__GNUC__

}//namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_conver()
{
	bool bl = false;

	string result1, result2;
	wstring wresult;
	string str = "我iconv命令是用来转换文件的编码方式，比如可以将UTF8编码的转换成GB18030的编码，反过来也行。"
				 "JDK中也提供了类似的工具native2ascii。Linux下的iconv开发库包括iconv_open,iconv_close,iconv等C函数，"
				 "可以用来在C/C++程序中很方便的转换字符编码，这在抓取网页的程序中很有用处，而iconv命令在调试此类程序时用得着!"
				 "iconv命令是用来转换文件的编码方式，比如可以将UTF8编码的转换成GB18030的编码，反过来也行。"
				 "JDK中也提供了类似的工具native2ascii。Linux下的iconv开发库包括iconv_open,iconv_close,iconv等C函数，"
				 "可以用来在C/C++程序中很方便的转换字符编码，这在抓取网页的程序中很有用处，而iconv命令在调试此类程序时用得着!"
				 "iconv命令是用来转换文件的编码方式，比如可以将UTF8编码的转换成GB18030的编码，反过来也行。"
				 "JDK中也提供了类似的工具native2ascii。Linux下的iconv开发库包括iconv_open,iconv_close,iconv等C函数，"
				 "可以用来在C/C++程序中很方便的转换字符编码，这在抓取网页的程序中很有用处，而iconv命令在调试此类程序时用得着!";
	
	bl = xcore::gbk_to_utf8(result1, str);
	ASSERT(bl);
	bl = xcore::utf8_to_gbk(result2, result1);
	ASSERT(bl);
	ASSERT(str == result2);

	bl = xcore::gbk_to_unicode(wresult, str);
	ASSERT(bl);
	bl = xcore::unicode_to_gbk(result2, wresult);
	ASSERT(bl);
	ASSERT(str == result2);

	wstring tmp;
	tmp.push_back(wresult.at(0));
	bl = xcore::unicode_to_gbk(result2, tmp);
	ASSERT(bl);
	ASSERT(result2 == "我");

	bl = xcore::gbk_to_unicode(wresult, str);
	ASSERT(bl);
	bl = xcore::unicode_to_utf8(result2, wresult);
	ASSERT(bl);
	bl = xcore::utf8_to_gbk(result1, result2);
	ASSERT(str == result1);

	bl = xcore::gbk_to_utf8(result1, str);
	ASSERT(bl);
	bl = xcore::utf8_to_unicode(wresult, result1);
	ASSERT(bl);
	bl = xcore::unicode_to_gbk(result2, wresult);
	ASSERT(str == result2);

	FILE* fd = fopen("test/gbk.txt", "rt");
	if (fd)
	{
		char buf[1024];
		string strText;
		size_t len = 0;
		while((len = fread(buf, 1, 1023, fd)) > 0)
		{
			buf[len] = '\0';
			strText += buf;
		}
		fclose(fd);
		string resultText1, resultText2;
		bl = xcore::gbk_to_utf8(resultText1, strText);
		ASSERT(bl);
		fd = fopen("test/utf8.txt", "wt+");
		ASSERT(fd);
		fwrite(resultText1.c_str(), 1, resultText1.size(), fd);
		fclose(fd);
		bl = xcore::utf8_to_gbk(resultText2, resultText1);
		ASSERT(bl);
		fd = fopen("test/gbk2.txt", "wt+");
		ASSERT(fd);
		fwrite(resultText2.c_str(), 1, resultText2.size(), fd);
		fclose(fd);
	}

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
