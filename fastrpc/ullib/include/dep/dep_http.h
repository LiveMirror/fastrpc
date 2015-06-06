/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: dep_http.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/



/**
 * @file ul_http.h
 * @author com-dev(com-dev@baidu.com)
 * @date 2007/12/13 22:30:33
 * @version $Revision: 1.2 $ 
 * @brief 旧版http库，只做旧功能维护，不做功能性更新升级。 
 */



#ifndef __DEP_HTTPLIB_H__
#define	__DEP_HTTPLIB_H__



#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ul_net.h"



#define    MAXURL        256	 /**< Max url length */
#define    MAXHOST       64		 /**< Max url host part length */
#define    MAXFILE        192	 /**< Max url file part length */
#define    MAXPAGE        256000 /**< Max http page lenght */
#define    MAXHEAD        4096	 /**< Mac http head length */
#define    MAXGETCMD      1256	 /**< Max http command length */
#define    MAXIP          16	 /**< Max ip string length */
#define    MAXPORTS        8
/* Write, read and connection timeout for socket */
#define    WRITETIMEOUT   60
#define    READTIMEOUT    300
#define    CONNECTTIMEOUT 30
#define    RETRYTIME      1		 /**< Retry times for tcp connection */
#define    SITE_LEN 100
#define    PORT_LEN 10
#define MAX_ETAG_LEN 64
#define METHOD_GET 1
#define METHOD_CHK 2
#define LASTMOD_FLAG 1
#define ETAG_FLAG 2



/**
 * 本结构体已经过期,不推荐使用
 */
typedef struct __page_item {
	int method;
	int orilen;
	int head_flag;
	char url[MAXURL];
	char ip[MAXIP];
	char protocol[4];
	int pagesize;
	int status;
	int conlen;
	char lastmod[MAXLINE];
	char contyp[MAXLINE];
	char trnenc[MAXLINE];
	char etag[MAX_ETAG_LEN];
	char redir[MAXURL];
	char *page;
	int proxy_flag;
} PAGE_ITEM;



/**
 * @brief 判断URL是否合法
 *
 * @param url 输入的url
 * @return 1 合法; 0 非法
 * @deprecated 本函数不推荐使用，请使用ullib/ul_check_url
 */
extern int check_URL(char *url);



/**
 * @brief  判断域名是否合法，是否是如下格式
 * - <hostname> ::= <name>*["."<name>]
 * - <name>  ::= <letter>[*[<letter-or-digit-or-hyphen>]<letter-or-digit>]
 * - <hyphen> ::= '-'-or-'_'
 *
 * @param host_name 输入的待判断域名 
 *
 * @return 1 合法; 0 非法 
 * @note 本函数已经过期,不推荐使用,有更新版本 see ul_url.h
 */
extern int check_hostname(char *host_name);



/**
 * @brief 取出meta标签中的链接
 *
 * @param page 含<meta url=redir ...> 的buff
 * @param size page长度
 * @param redir meta标签中的链接
 * @return 
 * - 1 成功获取链接
 * - 0 失败
 * @note size 不能超过1024 
 * @see 
 */
int isconredir(char *page, int size, char *redir);



/**
 * @brief 抓取网页内容 
 *
 * @param url 请求的url链接
 * @param ip 如果ip非空,使用此ip进行http连接,避免dns解析过程 
 * @param pagesize 
 * - in, page的长度.  *pagesize=0,只获取head,不获取页面内容; 
 * - out, 返回获得的页面长度 *
 * @param head 存放http返回头的buff, 最长:4096
 * @param page 存放页面内容的buff,大小由pagesize 指定
 * @param redirurl 存放跳转链接的buff,最长:256
 * @param lastmod 存放Last-Modified的buff,未指定最大长度...
 * @return
 * - <0 有错误
 * - >0 网页http返回状态码
 * @deprecated 本函数已经过期,不推荐使用,有更新版本 see libsrc/http
 */
extern int ul_gethttpcontent(char *url, char *ip, int *pagesize, char *head, char *page,
							 char *redirurl, char *lastmod);



/**
 * @brief 抓取网页，并经过chunked转换
 *
 * @param pageitem 
 * - in, 传入url,ip,port等信息
 * - out, 传出取得的网页
 * @return 
 * - >0 网页http返回状态码
 * - -1 失败
 * - -2 超时
 * @note 与::ul_gethttpcontent_new_notpt的区别是，抓取网页的get命令不同，解析chunk的方式也有区别。 
 * @see ul_gethttpcontent_new_notpt
 */
extern int ul_gethttpcontent_new(PAGE_ITEM * pageitem);



/**
 * @brief 抓取网页，并经过chunked转换
 *
 * @param pageitem 
 * - in, 传入url,ip,port等信息
 * - out, 传出取得的网页
 * @return 
 * - >0 网页http返回状态码
 * - -1 失败
 * - -2 超时
 * @note 与ul_gethttpcontent_new的区别是，抓取网页的get命令不同，增加了If-Modified-Since和If-None-Match在命令里,
 *		解析chunk的方式也有区别
 */
extern int ul_gethttpcontent_new_notpt(PAGE_ITEM * pageitem);

#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
