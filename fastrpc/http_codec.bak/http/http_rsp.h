#ifndef _HTTP_SERVER_2_0_HTTP_RSP_H_
#define _HTTP_SERVER_2_0_HTTP_RSP_H_

#include "http_head.h"

//////////////////////////////////////////////////////////////////////////

namespace httpsvr
{
	class CHttpStatusLine
	{
	public:
		CHttpStatusLine() : _http_version_primary (1)
			, _http_version_seconday(0), _status_code(200){}
		~CHttpStatusLine(){}
		
		void make_line();

		std::string _line;
		unsigned _http_version_primary;
		unsigned _http_version_seconday;
		unsigned _status_code;
	};
	
	class CHttpRspBody
	{
	public:
		CHttpRspBody() : _buf(NULL), _buf_size(0), _data_len(0){}
		~CHttpRspBody(){if (_buf) delete [] _buf;}

		int infuse(const char* data, unsigned length);
		
		char* _buf;
		unsigned _buf_size;
		unsigned _data_len;
	};

	//
	//	react operation: set http protocol version, set status code
	//	set header option
	//	infuse cgi return data --> header --> body
	//	set header option again
	//

	//
	//	the empty line between header and body, decided by body maker
	//	that is to say, if statis page, write "\r\n<html>data</html>"
	//	if cgi, write "SET-COOKIE: aaa=bbb\r\n\r\n<html>data</html>"
	//

	class CHttpRsp
	{
	public:
		CHttpRsp(){}
		~CHttpRsp(){}

		int output(char* buffer, unsigned buf_size, unsigned& data_len);
		int simple_rsp(unsigned status_code);
		int input(const char* data, unsigned length);	//	not the only way to infuse data

		//	first part, headline
		CHttpStatusLine _status_line;
		//	second part, header
		CHttpHeader _header;
		//	third part, body
		CHttpRspBody _body;
	};
}

//////////////////////////////////////////////////////////////////////////
#endif//_HTTP_SERVER_2_0_HTTP_RSP_H_
///:~
