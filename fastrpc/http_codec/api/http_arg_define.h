#ifndef _HTTP_SVR_ARGUMENT_DEFINE_H_
#define _HTTP_SVR_ARGUMENT_DEFINE_H_

#include<string>
#include<vector>

using namespace std;

/**
* @author feimat@baidu.com
*
* 
* <pre>
* Http 公用协议类代码
*
* </pre>
**/




//////////////////////////////////////////////////////////////////////////
#define ARG_RETCODE                 "retcode"
#define HTTP_CONTENT_TYPE_BINARY	"application/octet-stream"
#define HTTP_CONTENT_TYPE_HTML	    "text/html"

#define MAX_BUFFER_SIZE             (24 * 1024 * 1024)
#define ERR_MAX_BUFFER              -100
#define ERR_HTTP_DECODE             -101
#define ERR_HTTP_CMD_TYPE           -102


#endif

