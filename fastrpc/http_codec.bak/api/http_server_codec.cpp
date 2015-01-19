#include <stdio.h>
#include <errno.h>

#include "http_server_codec.h"
#include "http_req.h"
#include "http_respond.h"
#include "http_base_str.h"
#include "json/json.h"


using namespace std;
using namespace httpsvr;


/*match str form begin*/
char* match_str_begin(const char* src,unsigned srclen,const char* matchstr,unsigned matchstrlen)
{
	if(src == NULL || matchstr == NULL || srclen <= 0 || matchstrlen <= 0)
	{
		return NULL;
	}
	unsigned k,j;
	char* p_str = (char*)src;
	for(unsigned i = 0; i < srclen; i++)
	{
		j = i;
		k = 0;
		while(j < srclen && (tolower(p_str[j]) == tolower(matchstr[k])))
		{
			j++;
			k++;
			if( k == matchstrlen)
			{
				return (p_str + j - matchstrlen);
			}
		}
	}
	return NULL;
}

/*
0       还需要继续接收
-1      失败
*/
int http_complete_func(const char* p_data, unsigned int data_len)
{
	if(p_data == NULL || data_len < 4 || data_len<= 0)
	{
		return 0;
	}
	/*first match http head end*/
	int head_len = 0;
	char *p_head_end = match_str_begin(p_data,data_len,"\r\n\r\n",4);
	if(p_head_end == NULL)
	{
		p_head_end = match_str_begin(p_data,data_len,"\n\n",2);
		if ( p_head_end == NULL ) return 0;
		else head_len = p_head_end + 2 - p_data;
	}
	else
	{
		head_len = p_head_end + 4 - p_data;
	}

	//get Content-Length:
	char* pcontent_length = match_str_begin(p_data,data_len,"Content-Length:",15);
	if(pcontent_length != NULL)
	{
		int content_length = atoi(pcontent_length + 15);
		if( (int)(data_len) < content_length + head_len)
		{
			return 0;
		}
		else
		{
			return content_length + head_len;
		}
	}
	else
	{
		// 没 Content-Length ，那就是只有头部了
		return head_len;
	}
	return -1;
}

int ServerHttpCodec::decode_head(const string &buffer)
{
    if(buffer.length()>MAX_BUFFER_SIZE)
    {
        return ERR_MAX_BUFFER;
    }

    int ret;
    ret = http_req.input(buffer.c_str() , buffer.length());
    //解码http请求
    if (ret || http_req.inited() != true) 
    {
        return ERR_HTTP_DECODE;
    }

    //检测请求类型
    string strCmd = get_cmd_string();

#ifdef _USE_HTTP_PROTO_
    return 0;
#endif

    if( strCmd.empty() )
    {
        return ERR_HTTP_CMD_TYPE;
    }

    return 0;
}


string ServerHttpCodec::get_cmd_string()
{
    return http_req._req_line._file;
}

string ServerHttpCodec::get_argu(const string &argu)
{
    return http_req._req_line.get(argu);
}

int ServerHttpCodec::decode_download_file_req(string &fname)
{
    fname.clear();
    if ( get_cmd_string()!="download" ) return -1;
    
    fname = get_argu("fname");
    return 0;
}

int ServerHttpCodec::encode_download_file_rsp(const int &retcode,const string &fdata,string &buffer)
{
    string http_body="";
    if( retcode!=0 )
    {
        Json::Value root;
        root[ARG_RETCODE]=to_str(retcode);
        http_body=root.toStyledString();
        Chttp_simple_rsp::instance()->Forbidden(buffer ,http_body ,HTTP_CONTENT_TYPE_HTML);
        return 0;
    }
    // 把 fdata 数据放到 body 返回
    Chttp_simple_rsp::instance()->NormalResponse(buffer,fdata,HTTP_CONTENT_TYPE_HTML,0);
    return 0;
}

int ServerHttpCodec::decode_upload_file_req(string &fname,string &fdata)
{
	fname.clear();
	if ( get_cmd_string()!="upload" ) return -1;

	fname = get_argu("fname");

	fdata.assign(http_req._body._buf ,http_req._body._content_length);
	return 0;
}
int ServerHttpCodec::encode_upload_file_rsp(const int &retcode,string &buffer)
{
	string http_body="";

	Json::Value root;
	root[ARG_RETCODE]=to_str(retcode);
	http_body=root.toStyledString();
	Chttp_simple_rsp::instance()->NormalResponse(buffer ,http_body ,HTTP_CONTENT_TYPE_HTML,0);
	
	return 0;
}

int ServerHttpCodec::decode_sendfile_download_file_req(string &fname,long &offset,unsigned &len)
{
    fname.clear();
    if ( get_cmd_string()!="download" ) return -1;

    fname = get_argu("fname");
    string strOff = get_argu("offset");
    string strLen = get_argu("size");

    offset = atol(strOff.c_str());
    len = (unsigned)(atol(strLen.c_str()));
    return 0;
}
int ServerHttpCodec::encode_sendfile_download_file_rsp(const int &retcode,unsigned filelen,string &buffer)
{
    string http_body="";
    if( retcode!=0 || filelen<=0 )
    {
        Json::Value root;
        root[ARG_RETCODE]=to_str(retcode);
        http_body=root.toStyledString();
        Chttp_simple_rsp::instance()->Forbidden(buffer ,http_body ,HTTP_CONTENT_TYPE_HTML);
        return 0;
    }
    buffer = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_str(filelen) + "\r\n\r\n";
    return 0;
}

int ServerHttpCodec::decode_async_proxy_req(string &strWen)
{
    strWen.clear();
    if ( get_cmd_string()!="echo" ) return -1;

    strWen = get_argu("text");
    return 0;
}
int ServerHttpCodec::encode_async_proxy_rsp(const string &strRspWen,string &buffer)
{
    string http_body=strRspWen;
    Chttp_simple_rsp::instance()->NormalResponse(buffer ,http_body ,HTTP_CONTENT_TYPE_HTML,0);
    return 0;
}

int ServerHttpCodec::encode_back_proto(int ret,unsigned flow,const string &strRsp,string &buffer)
{
    buffer = "GET / HTTP/1.0\r\nRetCode: " + to_str(ret) + "\r\nFlowNo: " + to_str(flow) +
        "\r\nContent-Length: " + to_str(strRsp.length()) + "\r\n\r\n";

    buffer.append(strRsp);

    return 0;
}


int ServerHttpCodec::decode_web_upload_req(string &fname,string &body)
{
    fname = get_argu("fname");
    body.assign(http_req._body._buf , http_req._body._content_length);

    return 0;
}

int ServerHttpCodec::encode_web_upload_rsp(const string &data , string &buffer)
{
    Chttp_simple_rsp::instance()->NormalResponse(buffer ,data ,HTTP_CONTENT_TYPE_BINARY,0);
    return 0;
}
