// xcore_json.cpp
// jsoncpp 

#include "tconnd_xcore_json.h"
#include "tconnd_xcore_common.h"

#include <math.h>
#include <string.h>

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
	//const char* pos = m_pos;
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

namespace xcore {

static string __json_value_to_quoted_string(const string& str);

///////////////////////////////////////////////////////////////////////////////
// class XJsonValue
///////////////////////////////////////////////////////////////////////////////
XJsonValue::XJsonValue(ValueType type)
	: m_type(type)
	, m_intValue(0)
	, m_uintValue(0)
	, m_realValue(0.0)
	, m_boolValue(false)
{
	// empty
}

XJsonValue::XJsonValue(int64 value)
	: m_type(intValue)
	, m_intValue(value)
	, m_uintValue(0)
	, m_realValue(0.0)
	, m_boolValue(false)
{
	// empty
}

XJsonValue::XJsonValue(uint64 value)
	: m_type(uintValue)
	, m_intValue(0)
	, m_uintValue(value)
	, m_realValue(0.0)
	, m_boolValue(false)
{
	// empty
}

XJsonValue::XJsonValue(double value)
	: m_type(realValue)
	, m_intValue(0)
	, m_uintValue(0)
	, m_realValue(value)
	, m_boolValue(false)
{
	// empty
}

XJsonValue::XJsonValue(const string& value)
	: m_type(stringValue)
	, m_intValue(0)
	, m_uintValue(0)
	, m_realValue(0.0)
	, m_strValue(value)
	, m_boolValue(false)
{
	// empty
}

XJsonValue::XJsonValue(bool value)
	: m_type(booleanValue)
	, m_intValue(0)
	, m_uintValue(0)
	, m_realValue(0.0)
	, m_boolValue(value)
{
	// empty
}

XJsonValue::XJsonValue(const XJsonValue& other)
{
	this->operator=(other);
}

XJsonValue& XJsonValue::operator=(const XJsonValue& other)
{
	if(this != &other)
	{
		m_type = other.m_type;
		m_intValue = other.m_intValue;
		m_uintValue = other.m_uintValue;
		m_realValue = other.m_realValue;
		m_strValue = other.m_strValue;
		m_boolValue = other.m_boolValue;
		m_array = other.m_array;
		m_objects = other.m_objects;
		m_beforeComments = other.m_beforeComments;
		m_samelineComments = other.m_samelineComments;
		m_afterComments = other.m_afterComments;
	}
	return *this;
}

XJsonValue& XJsonValue::operator[](const string& key)
{
    return (this->as_objects()[key]);
}

bool XJsonValue::has(const string& key)
{
    ObjectValues& objv = this->as_objects();
    if (objv.end() != objv.find(key)) return true;
    return false;
}

bool XJsonValue::operator==(XJsonValue& other)
{
	if (m_type != other.m_type) return false;
	switch (m_type)
	{
	case nullValue:
		return true;
		break;
	case intValue:
		return m_intValue == other.m_intValue;
		break;
	case uintValue:
		return m_uintValue == other.m_uintValue;
		break;
	case realValue:
		return m_realValue == other.m_realValue;
		break;
	case stringValue:
		return m_strValue == other.m_strValue;
		break;
	case booleanValue:
		return m_boolValue == other.m_boolValue;
		break;
	case arrayValue:
		if (m_array.size() != other.m_array.size()) return false;
		for (size_t i = 0; i < m_array.size(); i++)
		{
			if (m_array[i] != other.m_array[i]) return false;
		}
		return true;
		break;
	case objectValue:
		if (m_objects.size() != other.m_objects.size()) return false;
		for (ObjectIterator it1 = m_objects.begin(), it2 = other.m_objects.begin();
			 it1 != m_objects.end(); 
			 ++it1, ++it2)
		{
			if (it1->first != it2->first) return false;
			if (it2->second != it2->second) return false;
		}
		return true;
		break;
	default:
		return false;
	}
	return false;
}

bool XJsonValue::operator!=(XJsonValue& other)
{
	return !operator==(other);
}

XJsonValue::ValueType& XJsonValue::type()
{
	return m_type;
}

void XJsonValue::clear()
{
	m_type = nullValue;
	m_intValue = 0;
	m_uintValue = 0;
	m_realValue = 0.0;
	m_strValue = "";
	m_boolValue = false;
	m_array.clear();
	m_objects.clear();
	m_beforeComments.clear();
	m_samelineComments.clear();
	m_afterComments.clear();
}

int64& XJsonValue::as_int()
{
	ASSERT(m_type == intValue);
	return m_intValue;
}

uint64& XJsonValue::as_uint()
{
	ASSERT(m_type == uintValue);
	return m_uintValue;
}

double& XJsonValue::as_real()
{
	ASSERT(m_type == realValue);
	return m_realValue;
}

string& XJsonValue::as_string()
{
	ASSERT(m_type == stringValue);
	return m_strValue;
}

bool& XJsonValue::as_bool()
{
	ASSERT(m_type == booleanValue);
	return m_boolValue;
}

XJsonValue::ArrayValues&  XJsonValue::as_array()
{
	ASSERT(m_type == arrayValue);
	return m_array;
}

XJsonValue::ObjectValues& XJsonValue::as_objects()
{
	ASSERT(m_type == objectValue);
	return m_objects;
}

///////////////////////////////////////////////////////////////////////////////
// class XJsonReader
///////////////////////////////////////////////////////////////////////////////
XJsonReader::XJsonReader()
	: m_collectComments(true)
{
	// empty
}

bool XJsonReader::parse(const string& doc, XJsonValue& root, bool collectComments)
{
	return parse(doc.c_str(), doc.c_str() + doc.size(), root, collectComments);
}

bool XJsonReader::parse(const char* beginDoc, const char* endDoc, XJsonValue& root, bool collectComments)
{
	ASSERT(beginDoc);
	ASSERT(endDoc);
	ASSERT(endDoc >= beginDoc);
	m_doc.assign(beginDoc, endDoc);
	m_collectComments = collectComments;
	m_parser.attach(beginDoc, (uint32)(endDoc - beginDoc));
	root.clear();
	string comment;

	// 处理前面的注释
	while (!m_parser.eof())
	{
		m_parser.skip_whitespace();
		if (*m_parser.position() != '/') break;
		if (!read_comment(comment)) return false;
		if (m_collectComments)
		{
			root.m_beforeComments += comment;
			m_parser.skip_within("\t ");
			if (m_parser.skip_within("\r\n") > 0)
				root.m_beforeComments += "\n";
		}
	}
	if (m_parser.eof()) return false; // json为空，视为无效
	
	if (!read_value(root)) return false;
	
	// 处理后面的注释
	while (!m_parser.eof())
	{
		m_parser.skip_whitespace();
		if (*m_parser.position() != '/') break;
		if (!read_comment(comment)) return false;
		if (m_collectComments)
		{
			root.m_afterComments += comment;
			root.m_afterComments += "\n";
		}
	}

	return true;
}

string XJsonReader::error_info()
{
	uint32 line = 1;
	uint32 column = 1;
	const char* pos = NULL;
	XStrParser parser(m_parser.start(), m_parser.position() - m_parser.start());
	while ((pos = parser.findchar('\n')) != parser.end())
	{
		parser.set_pos(pos + 1);
		line++;
	}
	column = m_parser.position() - parser.position();
	char buf[1024];
	sprintf(buf, "%s at line: %u column: %u.", m_error.c_str(), line, column);
	return buf;
}

bool XJsonReader::read_value(XJsonValue& value)
{
	bool result = true;
	string strTmp;
	value.clear();

	// 处理前面的注释
	while (!m_parser.eof())
	{
		m_parser.skip_whitespace();
		if (*m_parser.position() != '/') break;
		if (!read_comment(strTmp)) return false;
		if (m_collectComments)
		{
			value.m_beforeComments += strTmp;
			m_parser.skip_within("\t ");
			if (m_parser.skip_within("\r\n") > 0)
				value.m_beforeComments += "\n";
		}
	}
	if (m_parser.eof()) return true;

	// 处理值
	switch (*m_parser.position())
	{
	case '{':
		result = read_objects(value);
		break;
	case '[':
		result = read_array(value);
		break;
	case '"':
		result = read_string(value);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
		result = read_number(value);
		break;
	case 't':
		strTmp = m_parser.getstr_within("true");
		result = (strTmp == "true");
		if (result)
		{
			value.type() = XJsonValue::booleanValue;
			value.as_bool() = result;
		}
		break;
	case 'f':
		strTmp = m_parser.getstr_within("false");
		result = (strTmp == "false");
		if (result)
		{
			value.type() = XJsonValue::booleanValue;
			value.as_bool() = !result;
		}
		break;
	case 'n':
		strTmp = m_parser.getstr_within("null");
		result = (strTmp == "null");
		break;
	default:
		result = false;
		break;
	}
	if (!result)
	{
		if (m_error.empty()) m_error = "Unknown value type";
		return false;
	}
	
	// 处理同行注释
	while (!m_parser.eof())
	{
		m_parser.skip_within("\t ");
		if (*m_parser.position() != '/') break;
		if (!read_comment(strTmp)) return false;
		if (m_collectComments)
		{
			value.m_samelineComments += strTmp;
			m_parser.skip_within("\t ");
			if (m_parser.skip_within("\r\n") > 0)
			{
				value.m_samelineComments += "\n";
				break;
			}
		}
	}

	return true;
}

bool XJsonReader::read_number(XJsonValue& value)
{
	bool   has_frac = false;
	double fractional = 0.0;
	int64 power = 0;
	int64 num = 0;
	int64 signum = 1;

	if (*m_parser.position() == '-')
	{
		signum = -1;
		m_parser.skip_n(1);
	}
	if (m_parser.eof() || !m_parser.is_digit())
	{
		m_error = "Invalid number";
		return false;
	}
	
	num = m_parser.get_integer();
	if (!m_parser.eof() && *m_parser.position() == '.')
	{
		has_frac = true;
		m_parser.skip_n(1);
		fractional = m_parser.get_fractional();
	}
	if (!m_parser.eof() && (*m_parser.position() == 'e' || *m_parser.position() == 'E'))
	{
		m_parser.skip_n(1);
		power = m_parser.get_integer();
	}

	if (!has_frac && power == 0)
	{
		if (signum >= 0)
		{
			value.type() = XJsonValue::uintValue;
			value.as_uint() = (uint64)num;
		}
		else
		{
			value.type() = XJsonValue::intValue;
			value.as_int() = -num;
		}
	}
	else
	{
		value.type() = XJsonValue::realValue;
		value.as_real() = (double)(num * signum);
		if (signum >= 0)
		{
			value.as_real() += fractional;
		}
		else
		{
			value.as_real() -= fractional;
		}

		if (power >= 0)
		{
			for (int64 i = 0; i < power; i++)
			{
				value.as_real() *= 10.0;
			}
		}
		else if (power < 0)
		{
			for (int64 i = 0; i > power; i--)
			{
				value.as_real() /= 10.0;
			}
		}
	}
	return true;
}

bool XJsonReader::read_string(XJsonValue& value)
{
	XStrParser parser(m_parser);
	value.type() = XJsonValue::stringValue;
	if (!parser.getstr_by_quotation(value.as_string()))
	{
		m_error = "Invalid quoted string";
		return false;
	}
	m_parser = parser;
	return true;
}

bool XJsonReader::read_array(XJsonValue& value)
{
	m_parser.skip_char('[');
	value.type() = XJsonValue::arrayValue;
	m_parser.skip_whitespace();
	if (*m_parser.position() == ']')
	{
		m_parser.skip_char(']');
		return true;
	}

	while (!m_parser.eof())
	{
		value.as_array().push_back(XJsonValue());
		XJsonValue& subValue = value.as_array().back();
		if (!read_value(subValue)) return false;
		
		m_parser.skip_whitespace();
		char ch = m_parser.getch();
		if (ch == ',') continue;
		if (ch == ']') return true;
		
		m_error = "Miss ']' of array";
		return false;
	}

	m_error = "Miss ']' of array";
	return false;
}

bool XJsonReader::read_objects(XJsonValue& value)
{
	m_parser.skip_char('{');
	value.type() = XJsonValue::objectValue;
	m_parser.skip_whitespace();
	if (*m_parser.position() == '}')
	{
		m_parser.skip_char('}');
		return true;
	}

	while (!m_parser.eof())
	{
		XJsonValue tokenName;
		if (!read_value(tokenName)) return false;
		if (tokenName.type() != XJsonValue::stringValue)
		{
			m_error = "Invalid key name of object";
			return false;
		}
		
		m_parser.skip_whitespace();
		if (m_parser.getch() != ':')
		{
			m_error = "Miss ':' of object";
			return false;
		}

		XJsonValue& objValue = value.as_objects()[tokenName.as_string()];
		if (!read_value(objValue)) return false;
		if (!tokenName.m_beforeComments.empty())
		{
			objValue.m_beforeComments = tokenName.m_beforeComments;
		}

		m_parser.skip_whitespace();
		char ch = m_parser.getch();
		if (ch == ',') continue;
		if (ch == '}') return true;

		m_error = "Miss '}' of object";
		return false;
	}

	m_error = "Miss '}' of object";
	return false;
}

bool XJsonReader::read_comment(string& comment)
{
	if (m_parser.remain() < 2)
	{
		m_error = "Invalid comment";
		return false;
	}
	if (*(m_parser.position() + 1) == '*')
	{
		return read_cstyle_comment(comment);
	}
	else if (*(m_parser.position() + 1) == '/')
	{
		return read_cppstyle_comment(comment);
	}

	m_error = "Invalid comment";
	return false;
}

bool XJsonReader::read_cstyle_comment(string& comment)
{
	XStrParser parser(m_parser);
	const char* pos = parser.findstr("*/");
	if (pos == parser.end())
	{
		m_error = "Invalid comment";
		return false;
	}

	comment.assign(parser.position(), pos + 2);
	m_parser.set_pos(pos + 2);
	return true;
}

bool XJsonReader::read_cppstyle_comment(string& comment)
{
	const char* pos = m_parser.findchar_within("\r\n");
	comment.assign(m_parser.position(), pos);
	m_parser.set_pos(pos);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// class XJsonWriter
///////////////////////////////////////////////////////////////////////////////
XJsonWriter::XJsonWriter(bool enableYAMLCompatibility)
	: m_enableYAMLCompatibility(enableYAMLCompatibility)
	, m_styled(false)
	, m_depth(0)
{
	// empty
}

string XJsonWriter::fast_write(XJsonValue& root)
{
	m_styled = false;
	m_depth = 0;
	m_doc.clear();
	write_value(root);
	return m_doc;
}

string XJsonWriter::styled_write(XJsonValue& root)
{
	m_styled = true;
	m_depth = 0;
	m_doc.clear();
	write_value(root);
	return m_doc;
}

void XJsonWriter::write_value(XJsonValue& value)
{
	bool ismultiline = m_styled && is_multi_line(value);
	if (m_styled && !value.m_beforeComments.empty()) m_doc += value.m_beforeComments;
	switch (value.type())
	{
	case XJsonValue::nullValue:
		m_doc += "null";
		break;
	case XJsonValue::intValue:
		m_doc += XStrUtil::to_str(value.as_int());
		break;
	case XJsonValue::uintValue:
		m_doc += XStrUtil::to_str(value.as_uint());
		break;
	case XJsonValue::realValue:
		m_doc += XStrUtil::to_str(value.as_real(), "%#.16g");
		break;
	case XJsonValue::stringValue:
		m_doc += __json_value_to_quoted_string(value.as_string());
		break;
	case XJsonValue::booleanValue:
		m_doc += value.as_bool() ? "true" : "false";
		break;
	case XJsonValue::arrayValue:
		if (ismultiline) write_margin();
		m_doc += "[";
		m_depth++;
		for (XJsonValue::ArrayIterator it = value.as_array().begin();
			 it != value.as_array().end();
			 ++it)
		{
			if (it != value.as_array().begin())
			{
				m_doc += ",";
				if (m_styled) m_doc += " ";
			}
			if (ismultiline && !(*it).m_beforeComments.empty())
			{
				write_margin();
				m_doc += (*it).m_beforeComments;
			}
			if (ismultiline && !is_multi_line(*it)) write_margin();
			string tmp;
			tmp.swap((*it).m_beforeComments);
			write_value(*it);
			(*it).m_beforeComments.swap(tmp);
		}
		m_depth--;
		if (ismultiline) write_margin();
		m_doc += "]";
		break;
	case XJsonValue::objectValue:
		if (ismultiline) write_margin();
		m_doc += "{";
		m_depth++;
		for (XJsonValue::ObjectIterator it = value.as_objects().begin(); 
			 it != value.as_objects().end(); 
			 ++it)
		{
			if (it != value.as_objects().begin())
			{
				m_doc += ",";
				if (m_styled) m_doc += " ";
			}
			if (ismultiline && !it->second.m_beforeComments.empty())
			{
				write_margin();
				m_doc += it->second.m_beforeComments;
			}
			if (ismultiline) write_margin();
			m_doc += __json_value_to_quoted_string(it->first);
			m_doc += (m_enableYAMLCompatibility || m_styled) ? ": " : ":";
			string tmp;
			tmp.swap(it->second.m_beforeComments);
			write_value(it->second);
			it->second.m_beforeComments.swap(tmp);
		}
		m_depth--;
		if (ismultiline) write_margin();
		m_doc += "}";
		break;
	default:
		break;
	}
	if (m_styled && !value.m_samelineComments.empty()) m_doc += value.m_samelineComments;
	if (m_styled && !value.m_afterComments.empty())
	{
		write_margin();
		m_doc += value.m_afterComments;
	}

	return;
}

void XJsonWriter::write_margin()
{
	if (m_doc.empty()) return;
	m_doc += "\n";
	for (uint32 i = 0; i < m_depth; i++)
		m_doc += "    ";
}

bool XJsonWriter::is_multi_line(XJsonValue& value)
{
	if (value.type() == XJsonValue::arrayValue)
	{
		size_t length = 0;
		for (XJsonValue::ArrayIterator it = value.as_array().begin();
			 it != value.as_array().end();
			 ++it)
		{
			if ((*it).type() == XJsonValue::arrayValue) return true;
			if ((*it).type() == XJsonValue::objectValue) return true;
			if (!(*it).m_beforeComments.empty()) return true;
			if (!(*it).m_samelineComments.empty()) return true;
			if (!(*it).m_afterComments.empty()) return true;
			if ((*it).type() == XJsonValue::stringValue)
			{
				length += (*it).as_string().size();
				length += 4; // , []
			}
			else if ((*it).type() == XJsonValue::realValue)
			{
				length += 18;
				length += 4; // , []
			}
			else // 其他类型按四个字符计算
			{
				length += 4;
				length += 4; // , []
			}
		}
		if (length + m_depth * 4 >= 80) return true;
	}
	if (value.type() == XJsonValue::objectValue)
	{
		size_t length = 0;
		for (XJsonValue::ObjectIterator it = value.as_objects().begin();
			 it != value.as_objects().end();
			 ++it)
		{
			if (it->second.type() == XJsonValue::arrayValue) return true;
			if (it->second.type() == XJsonValue::objectValue) return true;
			if (!it->second.m_beforeComments.empty()) return true;
			if (!it->second.m_samelineComments.empty()) return true;
			if (!it->second.m_afterComments.empty()) return true;
			if (it->second.type() == XJsonValue::stringValue)
			{
				length += it->first.size();
				length += it->second.as_string().size();
				length += 6; // : , {}
			}
			else if (it->second.type() == XJsonValue::realValue)
			{
				length += 18;
				length += 4; // , []
			}
			else // 其他类型按四个字符计算
			{
				length += it->first.size();
				length += 4;
				length += 6; // : , {}
			}
		}
		if (length + m_depth * 4 >= 80) return true;
	}
	return false;
}

string __json_value_to_quoted_string(const string& str)
{
	string result = "\"";
	for (size_t i = 0; i < str.size(); i++)
	{
		switch(str[i])
		{
		case '\"':
			result += "\\\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		case '\b':
			result += "\\b";
			break;
		case '\f':
			result += "\\f";
			break;
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		case '\t':
			result += "\\t";
			break;
		default:
			result += str[i];
		}
	}
	result += "\"";
	return result;
}


} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

static std::string readInputTestFile( const char *path )
{
	FILE *file = fopen( path, "rb" );
	if ( !file )
		return std::string("");
	fseek( file, 0, SEEK_END );
	long size = ftell( file );
	fseek( file, 0, SEEK_SET );
	std::string text;
	char *buffer = new char[size+1];
	buffer[size] = 0;
	if ( fread( buffer, 1, size, file ) == (unsigned long)size )
		text = buffer;
	fclose( file );
	delete[] buffer;
	return text;
}

bool xcore_test_json()
{
	XJsonValue root;
	XJsonReader reader;
	XJsonWriter writer;
	printf("sizeof(XJsonValue) is %d\n", sizeof(XJsonValue));
	//VERIFY(reader.parse("12345.234E+20", root));
	//VERIFY(reader.parse("-0.12345678e7", root));
	//VERIFY(reader.parse("\"\\uD834\\uDD1E\"", root));
	//VERIFY(reader.parse("\"http:\/\/jsoncpp.sourceforge.net\/\"", root));
	string text;
#if 0
	char buf[10240];
	fgets(buf, 10239, stdin);
	text = buf;
#else
	char buf[10240];
	printf("input file: ");
	fgets(buf, 10239, stdin);
	string tmp = buf;
	XStrUtil::chop(tmp);
	text = readInputTestFile(tmp.c_str());
	printf("@@@file text:\n%s\n", text.c_str());
#endif
	if (reader.parse(text, root))
	{
		string result = writer.styled_write(root);
		printf("###:\n%s\n", result.c_str());
	}
	else
	{
		printf("###: parser failed (%s)\n", reader.error_info().c_str());
	}
	
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
