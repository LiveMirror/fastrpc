// date: 2011-04-25
// file: xcore_str_parser.cpp
// auth: 
// desc: »º³åÇø½âÎö¸¨ÖúÀà


#include "xcore_str_parser.h"

namespace xcore {

static bool __is_whitespace(char ch);
static bool __is_digit(char ch);
static bool __is_alpha(char ch);
static bool __is_hex(char ch);
static const char* __findchar(const char* str, char ch);
static string __unicode_to_utf8(uint32 unicode);
static const char* __empty_str = "";


/////////////////////////////////////////////////////////////////////
// class XStrParser
/////////////////////////////////////////////////////////////////////
XStrParser::XStrParser()
	: m_buff(__empty_str)
	, m_pos(__empty_str)
	, m_end(__empty_str)
{
	// empty
}

XStrParser::XStrParser(const char* buff, uint32 length)
	: m_buff(buff)
	, m_pos(buff)
	, m_end(buff + length)
{
	if (m_buff == NULL)
	{
		__clear();
	}
}

XStrParser::XStrParser(const XStrParser& other)
	: m_buff(other.m_buff)
	, m_pos(other.m_pos)
	, m_end(other.m_end)
{
	// empty
}

XStrParser& XStrParser::operator=(const XStrParser& other)
{
	if (&other != this)
	{
		m_buff = other.m_buff;
		m_pos = other.m_pos;
		m_end = other.m_end;
	}
	return *this;
}

XStrParser::~XStrParser()
{
	__clear();
}

void XStrParser::attach(const char* buff, uint32 length)
{
	if (buff)
	{
		m_buff = buff;
		m_pos = m_buff;
		m_end = m_buff + length;
	}
	else
	{
		__clear();
	}
	return;
}

void XStrParser::chop_head_whitespace()
{
	while (m_buff < m_end && __is_whitespace(*m_buff))
	{
		m_buff++;
	}
	if (m_pos < m_buff)	m_pos = m_buff;
	return;
}

void XStrParser::chop_tail_whitespace()
{
	while (m_buff < m_end && __is_whitespace(*(m_end - 1)))
	{
		m_end--;
	}
	if (m_pos > m_end) m_pos = m_end;
	return;
}

void XStrParser::chop_whitespace()
{
	chop_head_whitespace();
	chop_tail_whitespace();
	return;
}

bool XStrParser::set_pos(const char* pos)
{
	ASSERT(pos >= m_buff && pos <= m_end);
	if (pos < m_buff) return false;
	if (pos > m_end) return false;
	m_pos = pos;
	return true;
}

bool XStrParser::is_whitespace() const
{
	return !eof() && __is_whitespace(*m_pos);
}

bool XStrParser::is_digit() const
{
	return !eof() && __is_digit(*m_pos);
}

bool XStrParser::is_alpha() const
{
	return !eof() && __is_alpha(*m_pos);
}

bool XStrParser::is_hex() const
{
	return !eof() && __is_hex(*m_pos);
}

uint32 XStrParser::skip_n(uint32 n)
{
	if (m_pos + n > m_end)
	{
		n = (uint32)(m_end - m_pos);
	}
	m_pos += n;
	return n;
}

uint32 XStrParser::skip_char(uint8 ch)
{
	if (eof()) return 0;
	if (*m_pos != ch) return 0;
	m_pos++;
	return 1;
}

uint32 XStrParser::skip_within(const char* incharset)
{
	uint32 count = 0;
	if (incharset == NULL) return 0;
	while (!eof() && __findchar(incharset, *m_pos))
	{
		m_pos++;
		count++;
	}
	return count;
}

uint32 XStrParser::skip_without(const char* outcharset)
{
	uint32 count = 0;
	if (outcharset == NULL) return 0;
	while (!eof() && !__findchar(outcharset, *m_pos))
	{
		m_pos++;
		count++;
	}
	return count;
}

uint32 XStrParser::skip_whitespace()
{
	uint32 count = 0;
	while (!eof() && __is_whitespace(*m_pos))
	{
		m_pos++;
		count++;
	}
	return count;
}

uint32 XStrParser::skip_nonwhitespace()
{
	uint32 count = 0;
	while (!eof() && !__is_whitespace(*m_pos))
	{
		m_pos++;
		count++;
	}
	return count;
}

char XStrParser::getch()
{
	if (eof()) return 0;
	return *m_pos++;
}

bool XStrParser::getch_digit(uint8& digit)
{
	digit = 0;
	if (eof()) return false;
	if (!is_digit()) return false;
	digit = *m_pos++ - '0';
	return true;
}

bool XStrParser::getch_hex(uint8& hex)
{
	hex = 0;
	if (eof()) return false;
	char ch = *m_pos;
	if ('0' <= ch && ch <= '9')
		hex = ch - '0';
	else if ('a' <= ch && ch <= 'f')
		hex = ch - 'a' + 10;
	else if ('A' <= ch && ch <= 'F')
		hex = ch - 'A' + 10;
	else
		return false;

	m_pos++;
	return true;
}

bool XStrParser::getch_unicode(uint32& unicode)
{
	unicode = 0;
	if (remain() < 6) return false;
	if (*m_pos != '\\' || *(m_pos + 1) != 'u') return false;
	m_pos += 2;
	
	uint8 hex = 0;
	for (int i = 0; i < 4; i++)
	{
		if (!getch_hex(hex)) return false;
		unicode <<= 4;
		unicode += hex;
	}
	if (unicode >= 0xD800 && unicode <= 0xDBFF)
	{
		uint32 surrogatePair = 0;
		if (remain() < 6) return false;
		if (*m_pos != '\\' || *(m_pos + 1) != 'u') return false;
		m_pos += 2;
		for (int i = 0; i < 4; i++)
		{
			if (!getch_hex(hex)) return false;
			surrogatePair <<= 4;
			surrogatePair += hex;
		}
		unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
	}
	return true;
}

string XStrParser::getstr_n(uint32 n)
{
	if (n > (uint32)(m_end - m_pos))
	{
		n = (uint32)(m_end - m_pos);
	}
	const char* pos = m_pos;
	m_pos += n;
	return string(pos, n);
}

bool XStrParser::getstr_by_quotation(string& result)
{
	result.clear();
	if (eof()) return false;
	if (*m_pos != '"') return false;
	m_pos++;

	while (!eof())
	{
		char ch = *m_pos++;
		if (ch == '\\')
		{
			if (eof()) return false;
			ch = *m_pos++;
			switch ( ch )
			{
			case '"':  result += '"';  break;
			case '/':  result += '/';  break;
			case '\\': result += '\\'; break;
			case 'b':  result += '\b'; break;
			case 'f':  result += '\f'; break;
			case 'n':  result += '\n'; break;
			case 'r':  result += '\r'; break;
			case 't':  result += '\t'; break;
			case 'u':
				{
					m_pos -= 2;
					uint32 unicode = 0;
					if (!getch_unicode(unicode)) return false;
					result += __unicode_to_utf8(unicode);
				}
				break;
			default:
				return false;
			}
		}
		else if (ch == '"')
		{
			return true;
		}
		else
		{
			result += ch;
		}
	}

	return false;
}

bool XStrParser::getstr_by_sign(string& result, char lsign, char rsign, bool with_sign)
{
	result.clear();
	if (eof()) return false;
	if (lsign != *m_pos) return false;
	
	const char* pos = m_pos;
	m_pos++;
	while (!eof() && *m_pos != rsign)
	{
		m_pos++;
	}
	if (eof()) return false;
	m_pos++;
	
	if (with_sign)
		result.assign(pos, m_pos);
	else
		result.assign(pos + 1, m_pos - 1);
	return true;
}

string XStrParser::getstr_within(const char* incharset)
{
	const char* pos = m_pos;
	if (incharset == NULL) return __empty_str;
	while (!eof() && __findchar(incharset, *m_pos))
	{
		m_pos++;
	}
	return string(pos, m_pos);
}

string XStrParser::getstr_without(const char* outcharset)
{
	const char* pos = m_pos;
	if (outcharset == NULL) return __empty_str;
	while (!eof() && !__findchar(outcharset, *m_pos))
	{
		m_pos++;
	}
	return string(pos, m_pos);
}

int64 XStrParser::get_integer()
{
	const char* pos = m_pos;
	skip_whitespace();
	if (eof())
	{
		m_pos = pos;
		return 0;
	}
	
	int64 signum = 1;
	if (*m_pos == '+')
	{
		skip_n(1);
	}
	else if (*m_pos == '-')
	{
		signum = -1;
		skip_n(1);
	}
	skip_whitespace();
	if (!is_digit())
	{
		m_pos = pos;
		return 0;
	}

	int64 ret = 0;
	while (!eof() && __is_digit(*m_pos))
	{
		// 0xffffffffffffffff = 18446744073709551615
		uint8 num = (uint8)(*m_pos - '0');
		if (ret > 1844674407370955161LL) break;
		if (ret == 1844674407370955161LL && num > 5) break;
		ret *= 10;
		ret += num;
		m_pos++;
	}

	return ret * signum;
}

double XStrParser::get_fractional()
{
	double factor = 10.0;
	double result = 0.0;
	const char* pos = m_pos;
	skip_whitespace();
	while (!eof() && __is_digit(*m_pos))
	{
		result += (double)(*m_pos - '0') / factor;
		factor *= 10.0;
		m_pos++;
	}
	return result;
}

uint8 XStrParser::get_uint8()
{
	uint8 ret = 0;
	skip_whitespace();
	while (!eof() && __is_digit(*m_pos))
	{
		// 0xff = 255
		uint8 num = (uint8)(*m_pos - '0');
		if (ret > 25) break;
		if (ret == 25 && num > 5) break;
		ret *= 10;
		ret += num;
		m_pos++;
	}
	return ret;
}

uint32 XStrParser::get_uint32()
{
	uint32 ret = 0;
	skip_whitespace();
	while (!eof() && __is_digit(*m_pos))
	{
		// 0xffffffff = 4294967295
		uint8 num = (uint8)(*m_pos - '0');
		if (ret > 429496729) break;
		if (ret == 429496729 && num > 5) break;
		ret *= 10;
		ret += num;
		m_pos++;
	}
	return ret;
}

uint64 XStrParser::get_uint64()
{
	uint64 ret = 0;
	skip_whitespace();
	while (!eof() && __is_digit(*m_pos))
	{
		// 0xffffffffffffffff = 18446744073709551615
		uint8 num = (uint8)(*m_pos - '0');
		if (ret > 1844674407370955161ULL) break;
		if (ret == 1844674407370955161ULL && num > 5) break;
		ret *= 10;
		ret += num;
		m_pos++;
	}
	return ret;
}

uint64 XStrParser::get_hex()
{
	uint64 ret = 0;
	skip_whitespace();
	if ((remain() > 2) && 
		((*m_pos == '0' && *(m_pos + 1) == 'x') || 
		 (*m_pos == '0' && *(m_pos + 1) == 'X')))
		 m_pos += 2;

	while (!eof() && __is_hex(*m_pos))
	{
		// max = 0xffffffffffffffff
		uint8 num = 0;
		char ch = *m_pos;
		if ('0' <= ch && ch <= '9')
			num = (uint8)(ch - '0');
		else if ('a' <= ch && ch <= 'f')
			num = (uint8)(ch - 'a' + 10);
		else //if ('A' <= ch && ch <= 'F')
			num = (uint8)(ch - 'A' + 10);
		if (ret >= 0X1000000000000000ULL) break;
		ret <<= 1;
		ret |= num;
		m_pos++;
	}
	return ret;
}

const char* XStrParser::findchar(char ch) const
{
	const char* pos = m_pos;
	while (pos < m_end && *pos != ch)
	{
		pos++;
	}
	return pos;
}

const char* XStrParser::findchar(char ch, const char* before) const
{
	ASSERT(before >= m_buff && before <= m_end);
	ASSERT(m_pos < before);
	const char* before_ = before;
	if (before_ > m_end) before_ = m_end;
	if (before_ < m_buff) return before;
	if (m_pos >= before_) return before;
	const char* pos = m_pos;
	while (pos < before_ && *pos != ch)
	{
		pos++;
	}
	if (pos == before_) return before;
	return pos;
}

const char* XStrParser::findchar_within(const char* incharset) const
{
	if (incharset == NULL) return m_end;
	const char* pos = m_pos;
	while (pos < m_end && !__findchar(incharset, *pos))
	{
		pos++;
	}
	return pos;
}

const char* XStrParser::findchar_without(const char* outcharset) const
{
	if (outcharset == NULL) return m_end;
	const char* pos = m_pos;
	while (pos < m_end && __findchar(outcharset, *pos))
	{
		pos++;
	}
	return pos;
}

const char* XStrParser::findstr(const char* str) const
{
	if (str == NULL) return m_end;
	uint32 len = (uint32)strlen(str);
	if (len == 0) return m_pos;
	if ((uint32)(m_end - m_pos) < len) return m_end;

	const char* pos = m_pos;
	const char* end = m_end - len;
	for ( ; pos <= end; pos++)
	{
		if (0 == memcmp(str, pos, len))
		{
			return pos;
		}
	}
	return m_end;
}

void XStrParser::__clear()
{
	m_buff = __empty_str;
	m_pos = __empty_str;
	m_end = __empty_str;
	return;
}

bool __is_whitespace(char ch)
{
	return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}

bool __is_digit(char ch)
{
	return ('0' <= ch && ch <= '9');
}

bool __is_alpha(char ch)
{
	if ('a' <= ch && ch <= 'z') return true;
	if ('A' <= ch && ch <= 'Z') return true;
	return false;
}

bool __is_hex(char ch)
{
	if ('0' <= ch && ch <= '9') return true;
	if ('a' <= ch && ch <= 'f') return true;
	if ('A' <= ch && ch <= 'F') return true;
	return false;
}

const char* __findchar(const char* str, char ch)
{
	while (*str != '\0')
	{
		if (*str == ch)
			return str;
		else
			str++;
	}
	return NULL;
}

// Converts a unicode code-point to UTF-8.
string __unicode_to_utf8(uint32 unicode)
{
	string result;

	// based on description from http://en.wikipedia.org/wiki/UTF-8

	if (unicode <= 0x7f) 
	{
		result.resize(1);
		result[0] = static_cast<char>(unicode);
	} 
	else if (unicode <= 0x7FF) 
	{
		result.resize(2);
		result[1] = static_cast<char>(0x80 | (0x3f & unicode));
		result[0] = static_cast<char>(0xC0 | (0x1f & (unicode >> 6)));
	} 
	else if (unicode <= 0xFFFF) 
	{
		result.resize(3);
		result[2] = static_cast<char>(0x80 | (0x3f & unicode));
		result[1] = 0x80 | static_cast<char>((0x3f & (unicode >> 6)));
		result[0] = 0xE0 | static_cast<char>((0xf & (unicode >> 12)));
	}
	else if (unicode <= 0x10FFFF) 
	{
		result.resize(4);
		result[3] = static_cast<char>(0x80 | (0x3f & unicode));
		result[2] = static_cast<char>(0x80 | (0x3f & (unicode >> 6)));
		result[1] = static_cast<char>(0x80 | (0x3f & (unicode >> 12)));
		result[0] = static_cast<char>(0xF0 | (0x7 & (unicode >> 18)));
	}

	return result;
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_str_parser()
{
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
