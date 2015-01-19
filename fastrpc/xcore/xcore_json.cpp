// 2012-05-21
// xcore_json.cpp
// 
// 对json编解码的支持

#include "xcore_json.h"
#include "xcore_str_util.h"
#include <math.h>


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
