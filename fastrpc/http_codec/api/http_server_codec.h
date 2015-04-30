#ifndef _HTTP_SVR_BASE_CODEC_H_
#define _HTTP_SVR_BASE_CODEC_H_ 

#include <stdio.h>
#include <string>
#include "http_arg_define.h"
#include "http_req.h"

using namespace std;
using namespace httpsvr;


/**
* @author feimat@feimat.com
*
* 
* <pre>
* Http 公用协议类代码
*
* </pre>
**/

/*
	HTTP协议的检验函数
	0       还需要继续接收
	-1      失败
*/
int http_complete_func(const char* p_data, unsigned int data_len);


class ServerHttpCodec
{
public:

    // 先解出Http Header
    int decode_head(const string &buffer);

    // 返回http://127.0.0.1/download?name=xxxx中的 download 参数，我们称为命令号
    string get_cmd_string();
    // 返回http://127.0.0.1/download?name=xxxx中的 name 参数
    string get_argu(const string &argu);

    //**************协议例子：下载文件********************
    //http://127.0.0.1/download?fname=xxxx
    int decode_download_file_req(string &fname);
    int encode_download_file_rsp(const int &retcode,const string &fdata,string &buffer);

	//**************协议例子：上传文件********************
	//http://127.0.0.1/upload?fname=xxxx
	int decode_upload_file_req(string &fname,string &fdata);
	int encode_upload_file_rsp(const int &retcode,string &buffer);

    //**************协议例子：使用sendfile增强性能的下载文件********************
    //http://127.0.0.1/download?fname=xxxx
    int decode_sendfile_download_file_req(string &fname,long &offset,unsigned &len);
    // 只返回头部，文件内容又sendfile发送
    int encode_sendfile_download_file_rsp(const int &retcode,unsigned filelen,string &buffer);

    //**************协议例子：异步实现流程事务化的例子********************
    /*
        http://127.0.0.1/echo?text=xxxx

        后台组件传输的协议都是这样的格式

        FlowNo:111111\r\n\r\n
        Argument in Http Body

    */
    int decode_async_proxy_req(string &strWen);     // client req
    int encode_async_proxy_rsp(const string &strRspWen,string &buffer); // client rsp

    //**************协议例子：异步实现流程事务化的例子********************
    /*
        http://127.0.0.1/upload?fname=xxxx
        BODY

        注意，我们使用string作为body，但body可能包含二进制内容
        所以对于body的赋值应该是这样的： body.assign(szData,szLen)
        从body拷贝内容应该是memcpy(szData,body.c_str(),body.length())
    */
    int decode_web_upload_req(string &fname,string &body);
    int encode_web_upload_rsp(const string &data , string &buffer);

    int encode_back_proto(int ret,unsigned flow,const string &strRsp,string &buffer);

private:
    CHttpReq http_req;
};

#endif
