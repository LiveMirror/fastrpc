// xcore_json.h
// 
#include "tconnd_xcore_define.h"
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

#ifndef _XCORE_STR_PARSER_H_
#define _XCORE_STR_PARSER_H_


namespace xcore {

/////////////////////////////////////////////////////////////////////
// class XStrParser
/////////////////////////////////////////////////////////////////////
class XStrParser
{
public:
	XStrParser();

	XStrParser(const char* buff, uint32 length);

	XStrParser(const XStrParser& other);

	XStrParser& operator=(const XStrParser& other);

	~XStrParser();

	void attach(const char* buff, uint32 length);

	void   chop_head_whitespace();
	void   chop_tail_whitespace();
	void   chop_whitespace();

	const char*  position() const { return m_pos; }
	const char*  start() const { return m_buff; }
	const char*  end() const { return m_end; }
	
	uint32 remain() const { return (uint32)(m_end - m_pos); }
	bool   eof() const { return m_pos >= m_end; }
	void   reset() { m_pos = m_buff; }
	bool   set_pos(const char* pos);
	
	bool   is_whitespace() const;
	bool   is_digit() const;
	bool   is_alpha() const;
	bool   is_hex() const;

	uint32 skip_n(uint32 n);
	uint32 skip_char(uint8 ch);
	uint32 skip_within(const char* incharset);
	uint32 skip_without(const char* outcharset);
	uint32 skip_whitespace();
	uint32 skip_nonwhitespace();

	char   getch();
	bool   getch_digit(uint8& digit);
	bool   getch_hex(uint8& hex);
	bool   getch_unicode(uint32& unicode);
	bool   getstr_by_quotation(string& result);
	bool   getstr_by_sign(string& result, char lsign, char rsign, bool with_sign = false);
	string getstr_n(uint32 n = (uint32)-1);
	string getstr_within(const char* incharset);
	string getstr_without(const char* outcharset);

	int64  get_integer();
	double get_fractional();
	uint8  get_uint8();
	uint32 get_uint32();
	uint64 get_uint64();
	uint64 get_hex();

	const char*  findchar(char ch) const;
	const char*  findchar(char ch, const char* before) const;
	const char*  findchar_within(const char* incharset) const;
	const char*  findchar_without(const char* outcharset) const;
	const char*  findstr(const char* str) const;
	
private:
	void __clear();

private:
	const char*    m_buff;
	const char*    m_pos;
	const char*    m_end;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_STR_PARSER_H_

#ifndef _XCORE_JSON_H_
#define _XCORE_JSON_H_

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XJsonValue
///////////////////////////////////////////////////////////////////////////////
class XJsonValue
{
public:
	typedef vector<XJsonValue> ArrayValues;
	typedef map<string, XJsonValue> ObjectValues;
	typedef vector<XJsonValue>::iterator ArrayIterator;
	typedef map<string, XJsonValue>::iterator ObjectIterator;
	typedef enum
	{
		nullValue = 0, ///< 'null' value
		intValue,      ///< signed integer value
		uintValue,     ///< unsigned integer value
		realValue,     ///< double value
		stringValue,   ///< UTF-8 string value
		booleanValue,  ///< bool value
		arrayValue,    ///< array value (ordered list)
		objectValue    ///< object value (collection of name/value pairs).
	} ValueType;

public:
	XJsonValue(ValueType type = nullValue);
	XJsonValue(int64 value);
	XJsonValue(uint64 value);
	XJsonValue(double value);
	XJsonValue(const string& value);
	XJsonValue(bool value);

	XJsonValue(const XJsonValue& other);
	XJsonValue& operator=(const XJsonValue& other);
	XJsonValue& operator[](const string& key);
	bool operator==(XJsonValue& other);
	bool operator!=(XJsonValue& other);
    bool has(const string& key);

	ValueType& type();
	void       clear();

	int64&  as_int();
	uint64& as_uint();
	double& as_real();
	string& as_string();
	bool&   as_bool();
	ArrayValues&  as_array();
	ObjectValues& as_objects();

private:
	ValueType    m_type;
	int64        m_intValue;
	uint64       m_uintValue;
	double       m_realValue;
	string       m_strValue;
	bool         m_boolValue;
	ArrayValues  m_array;
	ObjectValues m_objects;

public:
	string       m_beforeComments;
	string       m_samelineComments;
	string       m_afterComments;
};

///////////////////////////////////////////////////////////////////////////////
// class XJsonReader
///////////////////////////////////////////////////////////////////////////////
class XJsonReader
{
public:
	XJsonReader();

	bool parse(const string& doc, XJsonValue& root, bool collectComments = true);

	bool parse(const char* beginDoc, const char* endDoc, XJsonValue& root, bool collectComments = true);

	string error_info();

protected:
	bool read_value(XJsonValue& value);
	bool read_number(XJsonValue& value);
	bool read_string(XJsonValue& value);
	bool read_array(XJsonValue& value);
	bool read_objects(XJsonValue& value);
	bool read_comment(string& comment);
	bool read_cstyle_comment(string& comment);
	bool read_cppstyle_comment(string& comment);

private:
	string m_doc;
	string m_error;
	bool   m_collectComments;
	XStrParser m_parser;
};

///////////////////////////////////////////////////////////////////////////////
// class XJsonWriter
///////////////////////////////////////////////////////////////////////////////
class XJsonWriter
{
public:
	XJsonWriter(bool enableYAMLCompatibility = false);

	string fast_write(XJsonValue& root);

	string styled_write(XJsonValue& root);

protected:
	void write_value(XJsonValue& value);
	bool is_multi_line(XJsonValue& value);
	void write_margin();

private:
	string m_doc;
	bool   m_enableYAMLCompatibility;
	bool   m_styled;
	uint32 m_depth;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_JSON_H_
