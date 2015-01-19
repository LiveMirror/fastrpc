/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: dep_exlink.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/



/**
 * @file ul_exlink.h
 * @author com-dev (com-dev@baidu.com)
 * @date 2007/12/14 11:07:50
 * @version $Revision: 1.2 $ 
 * @brief url萃取 
 */



#ifndef __DEP_EXLINK_H__
#define __DEP_EXLINK_H__
#include "ul_url.h"

#define  UL_MAX_LINK_NUM     2000
#define  UL_MAX_TEXT_LEN   100

#define UL_METHOD_BAIDU		0
#define UL_METHOD_WGET16	1
#define UL_METHOD_WGET18	2


typedef struct _link_info_t {
	char domain[UL_MAX_SITE_LEN];
	char port[UL_MAX_PORT_LEN];
	char path[UL_MAX_PATH_LEN];
	char text[UL_MAX_TEXT_LEN];
	char url[UL_MAX_URL_LEN];
} link_info_t;

typedef struct _link_info_t_ex {
	char domain[UL_MAX_SITE_LEN_EX];
	char port[UL_MAX_PORT_LEN];
	char path[UL_MAX_PATH_LEN_EX];
	char text[UL_MAX_TEXT_LEN];
	char url[UL_MAX_URL_LEN_EX];
} link_info_t_ex;


/**
 * 根据url和相对的url，合成新的url
 * 
 * @param[in]  url 主url
 * @param[in]  relurl 相关的url或相对url
 * @param[in]  absurl 返回数据的buf地址
 * @param[out] absurl 返回合成的url
 * @return 操作结果
 * - 1  成功
 * - 0  失败
 *  @note 注意输出buffer url的空间需要足够大，通常要大于UL_MAX_URL_LEN为合适
 */
int ul_absolute_url(char *url, char *relurl, char *absurl);



/**
 * 根据url对应的page内容，得到新的url(萃取连接)
 * 
 * @param[in]  url       page的url
 * @param[in]  page      page的内容
 * @param[in]  pagesize  page的大小，最大不能超过256K(>0)
 * @param[in]  link_info 返回link信息的缓冲结构指针
 * @param[in]  num       能够存储的url信息的个数(>0)
 * @param[in]  method    解析的方法
 * - UL_METHOD_BAIDU 
 * - UL_METHOD_WGET16
 * - UL_METHOD_WGET18	
 * @param[out]  link_info 返回解析出的新的url的结构
 * @return 操作结果
 * - >=0   成功，返回萃取成功的url数
 * - <0    失败
 * @note 使用UL_METHOD_BAIDU method运行时，函数运行内部需要额外申请256K的栈空间。
 */
int extract_link(char *url, char *page, int pagesize, link_info_t * link_info, int num, int method);
#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
