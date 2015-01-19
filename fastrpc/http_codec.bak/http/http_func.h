#ifndef _HTTP_FUNCTION_H_
#define _HTTP_FUNCTION_H_

#include <cstdlib>
#include <cctype>
#include <string>

#include "http_head.h"
#include "limits.h"

//////////////////////////////////////////////////////////////////////////

namespace httpsvr
{
	namespace func
	{
		//	return non-zero for error
		//	return 0 on success, if line_len == 0, no line-feed('\n'), return
		int get_line(const char* data, unsigned data_len
			, unsigned& line_len, unsigned& tail_len);

		bool path_up_limit(const std::string& path);

		int get_cgi_from_path(const std::string& path
			, const std::string& cgi_dir_path, std::string& cgi_name);

		//	fourth, find the "name: value"
		//	for(;;) find the ':', set name = substr(o,pos)
		//	if ":" , set value = substr(pos+1, line_len - tail_len - pos)
		int get_name_value(const char* data, unsigned data_len
			, unsigned& name_len, unsigned& value_offset, unsigned& value_len);

		int get_first_word(const char* data, unsigned data_len
			, unsigned& word_len);

		//把 url 里 %xx 转成 buf内容
		int decode_string(const char* data, unsigned data_len,
			char* buf, unsigned buf_size, unsigned& result_len);

//		int parse_uri(const char* data, unsigned data_len
//			, std::string& user, std::string& site, std::string& path
//			, std::string& port, std::string& arg);

		//获取url 里面的参数： path/file?arg
		int parse_uri(const char* data, unsigned data_len
			, std::string& path, std::string& file, std::string& arg);
	}
}

//////////////////////////////////////////////////////////////////////////
#endif//_HTTP_FUNCTION_H_
///:~
