/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: comlogsvr.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file comlogsvr.h
 * @author baonh(baonh@baidu.com)
 * @version $Revision: 1.2 $ 
 * @brief comlog服务端 客户端接口头文件
 *  
 **/


#ifndef  __COMLOGSVR_H_
#define  __COMLOGSVR_H_

#include "ul_def.h"
const u_int COMLOG_MODULE_MAXLEN = 32;		  /**< 模块名最大长度       */
const u_int COMLOG_FILENAME_MAXLEN = 32;	  /**< 文件名最大长度       */
const u_int COMLOG_AUTH_MAXLEN = 32;		  /**< 验证码最大长度       */
const u_int COMLOG_RESERVED_MAXLEN = 8;		  /**< 保留字最大长度       */



enum {		  /**< 服务端发送的命令号,填写于客户端的nshead头的reserved字段       */
	COMLOG_CREATE = 1,		  /**< 打开日志服务器文件       */
	COMLOG_CLOSE = 2,		  /**< 关闭日志服务器文件       */
	COMLOG_FLUSH = 3,		  /**< 日志flush，把缓冲写入文件       */
	COMLOG_SET = 4,	          /**< 设置参数的命令       */
	COMLOG_CHECK = 5,         /**< 日志条数同步检查       */
	COMLOG_PRINT = 6          /**< 打日志       */
};

enum {		  /**< 服务段返回的错误号       */
	COMLOG_SEND_SUCCESS = 0,		  /**< 发送命令 操作成功       */
	COMLOG_SEND_FAIL = -1		      /**< 发送命令 操作失败      */
};


enum {		  /**< 压缩存储       */
	COMLOG_UNCOMPRESS = 0,		/**< 日志不压缩存储     */  
	COMLOG_COMPRESS = 1		    /**< 日志压缩存储       */
};

#pragma pack(push, 4)
typedef struct _logsvr_req_t {		  /**< 客户端发送的请求头,在nshead头发送之后       */
	char path[COMLOG_MODULE_MAXLEN];	   /**< 模块名       */
	char filename[COMLOG_FILENAME_MAXLEN]; /**< 打印的文件名 */
	char auth[COMLOG_AUTH_MAXLEN];         /**< 验证码，当多个服务端日志需要打到一个日志文件中时使用*/
										   /**< 不需要使用时使用空串。 */
	u_int handle;		                       /**< 日志的句柄，由CREATE命令生成，写日志时需要提供 */
	int compress;		                   /**< 是否压缩@ref COMLOG_COMPRESS,@ref COMLOG_UNCOMPRESS*/
	char reserved[COMLOG_RESERVED_MAXLEN]; /**< 保留字 */
} logsvr_req_t;


typedef struct _logitem_t {	  /**< 客户端传输的单条日志结构，多个跟在客户端发送的请求头后       */
	u_int handle; /**< 日志的句柄，由COMLOG_CREATE命令生成，写日志时需要提供 */
	u_int time;   /**< 日志的时间 */
	u_int size;   /**< 日志的长度 */
	char log[0];	  /**< 实际日志数据       */
} logitem_t;

typedef struct _logsvr_res_t{		  /**< 服务端根据命令返回的结果       */
	int error_code;  /**< 返回结果的错误号，@ref COMLOG_SEND_SUCCESS和 @ref COMLOG_SEND_FAIL */
	u_int handle;      /**< 日志句柄 */
    int count;     /**< 日志数 */
} logsvr_res_t;


typedef	struct _log_index_t {
		u_int min_time; 
		u_int max_time;
		u_int offset;  //文件偏移
} log_index_t;


#pragma pack(pop)

#endif  //__COMLOGSVR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
