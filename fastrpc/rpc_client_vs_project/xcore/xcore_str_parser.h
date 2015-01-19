// date: 2011-04-25
// file: xcore_str_parser.h
// auth: 
// desc: »º³åÇø½âÎö¸¨ÖúÀà


#ifndef _XCORE_STR_PARSER_H_
#define _XCORE_STR_PARSER_H_

#include "xcore_define.h"

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
