#include "http_func.h"

using namespace std;
using namespace httpsvr;
using namespace httpsvr::func;

//////////////////////////////////////////////////////////////////////////

//	return non-zero for error
//	return 0 on success, if line_len == 0, no line-feed('\n'), return
int httpsvr::func::get_line(const char* data, unsigned data_len
                            , unsigned& line_len, unsigned& tail_len)
{
    for(unsigned i = 0; i < data_len; i++)
    {
        if(data[i] == '\n')
        {
            if (i == 0)
            {
                line_len = tail_len = 1;
                return 0;
            }
            else if (data[i-1] == '\r')
            {
                line_len = i+1;
                tail_len = 2;
                return 0;
            }
            else
            {
                line_len = i+1;
                tail_len = 1;
                return 0;
            }
        }
    }
    line_len = tail_len = 0;
    return 0;
}

bool httpsvr::func::path_up_limit(const std::string& path)
{
    if (path.find("..") != std::string::npos)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int httpsvr::func::get_cgi_from_path(const std::string& path
                                     , const std::string& cgi_dir_path, std::string& cgi_name)
{
    //	first, check if cgi-path
    unsigned pos = path.find(cgi_dir_path);
    if (pos != 0)
    {
        return -1;
    }

    //	second, get file name
    if (path.at(cgi_dir_path.length()-1) == '/')
    {
        cgi_name = path.substr(cgi_dir_path.length());
    }
    else if (path.at(cgi_dir_path.length()) == '/')
    {
        cgi_name = path.substr(cgi_dir_path.length() + 1);
    }
    else	//	not "cgi-dir + / + xxx" or "cgi-dir/ + xxx"
    {
        return -1;
    }

    //	third, no other sub_dir
    if (cgi_name.find('/') != std::string::npos)
    {
        return -1;
    }

    //	fourth,
    return 0;
}

//	fourth, find the "name: value"
//	for(;;) find the ':', set name = substr(o,pos)
//	if ":" , set value = substr(pos+1, line_len - tail_len - pos)
int httpsvr::func::get_name_value(const char* data, unsigned data_len
                                  , unsigned& name_len, unsigned& value_offset, unsigned& value_len)
{
    //	find the separator
    unsigned pos = 0;
    const char* tail = data + data_len;
    for(const char* ch = data; ch != tail; ch++)
    {
        if(*ch == ':')
        {
            pos = ch-data;
            break;
        }
    }

    //	find result
    if (pos == 0)	//	no ':', invalid head
        return C_HTTP_HEADER_ERROR_INVALID_FORMAT;

    //	name
    //static const unsigned C_MAX_NAME_LENGTH = 1<<12;
    //char sName[C_MAX_NAME_LENGTH];
    //for(unsigned i = 0; i < pos; i++)
    //{
    //	sName[i] = toupper(data[i]);
    //}
    //sName[pos] = 0;
    name_len = pos;

    //	value
    for(value_offset = pos+1
        ; !isspace(*(data + value_offset)) && value_offset < data_len
        ; value_offset++)
    {
    }
    value_len = data_len - value_offset;
    return 0;
}

int httpsvr::func::get_first_word(const char* data, unsigned data_len
                                  , unsigned& word_len)
{
    for(unsigned i = 0; i < data_len; i++)
    {
        if(isspace(data[i]))
        {
            word_len = i;
            return 0;
        }
    }
    word_len = 0;
    return 0;
}

//把 url 里 %xx 转成 buf内容
int httpsvr::func::decode_string(const char* data, unsigned data_len,
                                 char* buf, unsigned buf_size, unsigned& result_len)
{
    unsigned j = 0;
    for(unsigned i = 0; i < data_len && j < buf_size
        ; i++, j++)
    {
        if (data[i] == '%')
        {
            if (i+2 >= data_len)
            {
                return -1;	//	cant be decoded
            }
            char sPercentage[3] = {0};
            sPercentage[0] = data[i+1];
            sPercentage[1] = data[i+2];
            buf[j] = (char)strtol(sPercentage, (char **)0, 16);
            i += 2;
        }
        else
        {
            buf[j]=data[i];
        }
    }
    result_len = j;
    return 0;
}

int httpsvr::func::parse_uri(const char* data, unsigned data_len
                             , std::string& path, std::string& file, std::string& arg)
{
    if ( data[0]== '/' && data[1] == ' ' ) return 0;

    //	first, find the first "?", left is path
    unsigned uri_end = ULONG_MAX;
    for(unsigned i = 0; i < data_len; i++)
    {
        if (data[i] == '?')
        {
            uri_end = i;
            break;
        }
    }
    if (uri_end == ULONG_MAX)
    {
        uri_end = data_len;
    }

    //	third, find the last '/' in the left
    unsigned file_start = ULONG_MAX;
    for(unsigned i = uri_end; i != 0; i--)
    {
        if (data[i-1] == '/')
        {
            file_start = i-1;
            break;
        }
    }
    if (file_start == ULONG_MAX)
    {
        return -1;
    }
    else
    {
        path = string(data, file_start);
        file = string(data + file_start + 1, uri_end - file_start - 1);
    }

    //	fifth, convert the "%6D" into char arg, in the right
    if (uri_end == data_len)
    {
        arg = "";
    }
    else
    {
        static const unsigned C_MAX_ARG_SIZE = 1<<10;
        char sArg[C_MAX_ARG_SIZE];
        unsigned arg_len = 0;
        int ret = decode_string(data + uri_end + 1, data_len - uri_end - 1
                                , sArg, sizeof(sArg), arg_len);
        if (ret)
        {
            return ret;
        }
        sArg[arg_len] = 0;
        arg = sArg;
    }
    return 0;
}

//int httpsvr::func::parse_uri(const char* data, unsigned data_len
//			  , std::string& user, std::string& site, std::string& path
//			  , std::string& port, std::string& arg)
//{
//	//	first, find the first "//", the "http://"
//	unsigned uri_offset = ULONG_MAX;
//	for(unsigned i = 0; i < data_len; i++)
//	{
//		if (data[i] == '/')
//		{
//			if(data[i+1] == '/')
//			{
//				uri_offset = i;
//				break;
//			}
//		}
//	}
//	if (uri_offset == ULONG_MAX)
//	{
//		return -1;
//	}
//
//
//	//	second, find the first "?", left is root@site:port
//	unsigned uri_end = ULONG_MAX;
//	for(unsigned i = uri_offset; i < data_len; i++)
//	{
//		if (data[i] == '?')
//		{
//			uri_end = i;
//			break;
//		}
//	}
//	if (uri_end == ULONG_MAX)
//	{
//		uri_end = data_len;
//	}
//
//	//	third, find the "@" in the left
//	unsigned user_end = ULONG_MAX;
//	for(unsigned i = uri_offset; i < uri_end; i++)
//	{
//		if (data[i] == '@')
//		{
//			user_end = i;
//			break;
//		}
//	}
//	unsigned site_offset = uri_offset;
//	if (user_end == ULONG_MAX)
//	{
//		user = "";
//	}
//	else
//	{
//		user = std::string(data + uri_offset, user_end - uri_offset);
//		site_offset = user_end + 1;
//	}
//
//	//	fourth, find the ":" in the left
//	unsigned site_end = ULONG_MAX;
//	for(unsigned i = site_offset; i < uri_end; i++)
//	{
//		if (data[i] == ':')
//		{
//			site_end = i;
//			break;
//		}
//	}
//	if (site_end == ULONG_MAX)
//	{
//		site = std::string(data + site_offset, uri_end - site_offset);
//		port = "";
//	}
//	else
//	{
//		site = std::string(data + site_offset, site_end - site_offset);
//		port = std::string(data + site_end + 1, uri_end - site_end - 1);
//	}
//
//	//	fifth, convert the "%6D" into char arg, in the right
//	if (uri_end == data_len)
//	{
//		arg = "";
//	}
//	else
//	{
//		static const unsigned C_MAX_ARG_SIZE = 1<<10;
//		char sArg[C_MAX_ARG_SIZE];
//		unsigned arg_len = 0;
//		int ret = decode_string(data + uri_end + 1, data_len - uri_end - 1
//			, sArg, sizeof(sArg), arg_len);
//		if (ret)
//		{
//			return ret;
//		}
//		sArg[arg_len] = 0;
//		arg = sArg;
//	}
//	return 0;
//}

//////////////////////////////////////////////////////////////////////////
///:~
