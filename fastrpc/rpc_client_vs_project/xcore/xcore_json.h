// 2012-05-21
// xcore_json.h
// 
// 对json编解码的支持

#ifndef _XCORE_JSON_H_
#define _XCORE_JSON_H_

#include "xcore_define.h"
#include "xcore_str_parser.h"

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
	bool operator==(XJsonValue& other);
	bool operator!=(XJsonValue& other);

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
