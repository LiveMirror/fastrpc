/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: loghead.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file loghead.h
 * @author feimat(com@feimat.com)
 * @date 2008/09/12 12:30:24
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __LOGHEAD_H_
#define  __LOGHEAD_H_

#include <unistd.h>
#include<sys/uio.h>
namespace comspace{

const unsigned int LOGHEAD_MAGICNUM = 0xfb709394;

/// 读写标志位，所有读写函数默认LOGHEAD_CHECK_MAGICNUM
typedef enum _LOGHEAD_FLAGS_T {
    LOGHEAD_CHECK_NONE        =    0,
    LOGHEAD_CHECK_MAGICNUM    = 0x01,    ///<检查magic_num是否正确
    LOGHEAD_CHECK_PARAM       = 0x02,    ///<检查参数是否正确
} LOGHEAD_FLAGS_T;


/// 返回错误码 = LOGHEAD_RET_SUCCESS成功, <0失败
typedef enum _LOGHEAD_RET_ERROR_T {
    LOGHEAD_RET_SUCCESS       =   0, ///<读写OK
    LOGHEAD_RET_EPARAM        =  -1, ///<参数有问题
    LOGHEAD_RET_EBODYLEN      =  -2, ///<变长数据长度有问题
    LOGHEAD_RET_WRITE         =  -3, ///<写的问题
    LOGHEAD_RET_READ          =  -4, ///<读消息体失败，具体错误看errno
    LOGHEAD_RET_READHEAD      =  -5, ///<读消息头失败, 具体错误看errno
    LOGHEAD_RET_WRITEHEAD     =  -6, ///<写消息头失败, 可能是对方将连接关闭了
    LOGHEAD_RET_PEARCLOSE     =  -7, ///<对端关闭连接
    LOGHEAD_RET_ETIMEDOUT     =  -8, ///<读写超时
    LOGHEAD_RET_EMAGICNUM     =  -9, ///<magic_num不匹配
    LOGHEAD_RET_UNKNOWN	     =  -10
} LOGHEAD_RET_ERROR_T;

/**
 * ns产品线网络交互统一的包头，注释包含为(M)的为必须遵循的规范
 */
typedef struct _loghead_t
{
    unsigned short id;              ///<id
    unsigned short version;         ///<版本号
    ///(M)由apache产生的logid，贯穿一次请求的所有网络交互
    unsigned int   log_id;
    ///(M)客户端标识，建议命名方式：产品名-模块名，比如"sp-ui", "mp3-as"
    char           provider[16];
    ///(M)特殊标识，标识一个包的起始
    unsigned int   magic_num;
    unsigned int   reserved;       ///<保留
    ///(M)head后请求数据的总长度
    unsigned int   body_len;
} loghead_t;

/** 
 * @brief 读数据, 实际读取数据 head->body_len
 * 
 * @param sock socket 句柄
 * @param buf 读缓冲区
 * @param buf_size 读入缓冲区的最大大小
 * @param timeout 超时
 * @param flags 定义如上
 * 
 * @return 
 */
int loghead_read(int sock, void *buf, size_t buf_size, int timeout,
	        unsigned flags = LOGHEAD_CHECK_MAGICNUM);



/**
 * 写数据, 一次写完所有数据。发送总长度为sizeof(loghead_t)+head->body_len;
 * @return 成功返回0，异常返回对应的错误码
 */
int loghead_write(int sock, loghead_t *head, int timeout,
	unsigned flags = LOGHEAD_CHECK_MAGICNUM);


}











#endif  //__LOGHEAD_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
