#ifndef _SVR_NS_HEADER_H_
#define _SVR_NS_HEADER_H_


#include <vector>
#include <errno.h>
#include <string>
#include <sys/time.h>



using namespace std;


/**
* @author feimat@baidu.com
*
* 
* <pre>
* 网络读写函数
* </pre>
**/

typedef enum _NS_RETURN_STATUS {
    _ASVR_NSHEAD_RET_SUCCESS       =   0, ///<读写OK
    _ASVR_NSHEAD_RET_EPARAM        =  -1, ///<参数有问题
    _ASVR_NSHEAD_RET_EBODYLEN      =  -2, ///<变长数据长度有问题
    _ASVR_NSHEAD_RET_WRITE         =  -3, ///<写的问题
    _ASVR_NSHEAD_RET_READ          =  -4, ///<读消息体失败，具体错误看errno
    _ASVR_NSHEAD_RET_READHEAD      =  -5, ///<读消息头失败, 具体错误看errno
    _ASVR_NSHEAD_RET_WRITEHEAD     =  -6, ///<写消息头失败, 可能是对方将连接关闭了
    _ASVR_NSHEAD_RET_PEARCLOSE     =  -7, ///<对端关闭连接
    _ASVR_NSHEAD_RET_ETIMEDOUT     =  -8, ///<读写超时
    _ASVR_NSHEAD_RET_EMAGICNUM     =  -9, ///<magic_num不匹配
    _ASVR_NSHEAD_RET_UNKNOWN	     =  -10
} enumResStatus;


typedef struct _ns_header
{
    unsigned short id;              // id
    unsigned short version;         // 版本号
    unsigned int log_id;          // (M)由apache产生的logid，贯穿一次请求的所有网络交互
    char provider[16];    // (M)客户端标识，建议命名方式：产品名-模块名，比如"sp-ui"
    unsigned int retcode;           //magic_num 我们作为return code
    unsigned int flow_no;           //reserved 我们用做flow_no
    unsigned int body_len;        //(M)head后请求数据的总长度
} CProtoHeader;

typedef struct _ns_header_rs{
    CProtoHeader head;		///
    int retcode;			///<响应状态
}CProtoHeaderRes;


#endif

