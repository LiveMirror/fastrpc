#ifndef _HTTP_SERVER_2_0_HTTP_REQ_H_
#define _HTTP_SERVER_2_0_HTTP_REQ_H_

#include <string>
#include <map>
#include "http_head.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////

namespace httpsvr
{
	enum en_http_request_type
	{
		C_HTTP_REQ_TYPE_GET					= 10001,
			C_HTTP_REQ_TYPE_POST			= 10002,
	};
	enum en_http_request_object_type
	{
		C_HTTP_REQ_TYPE_CGI					= 20001,
			C_HTTP_REQ_TYPE_STATIC_PAGE		= 20002,
	};
	enum en_http_request_error
	{
		C_HTTP_REQ_ERR_PARSE_TOO_MUCH_DATA	= 1000001,
			C_HTTP_ERR_INVALID_REQUEST_LINE	= 1000002,
	};
	
	//请求信息, GET, POST
	class CHttpRequestLine
	{
	public:
		CHttpRequestLine();
		~CHttpRequestLine();

		bool full() const {return _full;}
		int infuse(const char* data, unsigned data_len, unsigned& skip_len);

		int req_type() const {return _req_type;}
		std::string path() const {return _path;}	//	/xx/yy/zz/qq.xxx
		std::string argument() const {return _arg;}		//	xx=oo&yy=qq&zz=pp
		int version() const {return _ver;}
		std::string get(const std::string& name);

		bool _full;
		std::string _line;
		int _req_type;
		std::string _path;
		std::string _file;
		std::string _arg;
		int _ver;
		std::map<string, string> url_args;
		bool _init_url_args;
	};

	//身体信息
	class CReqBody	//	attacher class, only mark the offset on another buffer
	{
	public:
		CReqBody(){_content_length = 0xffffffff; _infused_length = 0;_buf=NULL;}
		~CReqBody(){}

		bool full(){return _content_length <= _infused_length;}
		int set_len(unsigned len){_content_length = len; return 0;}
		int set_buf(char *buf){_buf = buf; return 0;}

		int infuse(const char* data, unsigned data_len, unsigned& skip_len);

		unsigned _content_length;
		unsigned _infused_length;
		char *_buf;
	};

	class CHttpReq
	{
	public:
		CHttpReq();
		~CHttpReq();

		//注：不可重入
		int input(const void* data, unsigned data_len);
		bool inited() const{return  _inited;}
		int output(char* buf, unsigned buf_size, unsigned& data_len);

	public:
		//	original data
		unsigned _buf_size;
		char* _buf;
		unsigned _data_len;

		//	parse procedure variables
		bool _inited;
		unsigned _parsed_len;
		
	public:
		//	first part, headline
		CHttpRequestLine _req_line;
		//	second part, header
		CHttpHeader _header;
		//	third part, body
		unsigned _content_offset;
		CReqBody _body;
		//	http protocol infomation	//	if type have no req-line

		int append_data(const void* data, unsigned data_len);
		int get_url_args(map<string, string> & url_args, string & url);
	};
}

//////////////////////////////////////////////////////////////////////////
#endif//_HTTP_SERVER_2_0_HTTP_REQ_H_
