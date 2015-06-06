/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: logcore.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file logcore.h
 * @author feimat(com@baidu.com)
 * @date 2008/03/16 20:14:08
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __LOGCORE_H_
#define  __LOGCORE_H_

#include "comlog.h"
#include "xdebug.h"

namespace comspace
{

//根据日志顶级获取日志名
/**
 * @brief 根据日志等级 返回日志名
 *
 * @param [in/out] logid   : u_int
 * @return  const char* 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/25 16:37:05
**/
const char *getLogName(u_int logid);
/**
 * @brief 根据日志名，返回日志id
 *
 * @param [in/out] logname   : const char*
 * @return  const u_int 
 * @retval   返回0表示失败
 * @see 
 * @author feimat
 * @date 2008/03/25 16:37:49
**/
u_int getLogId(const char *logname);
//返回对应的日志等级
int createSelfLog(const char *lvlname);

bool is_init_log();
int init_log();
int init_log_r();
int com_wait_close(int waittime);
int com_close_appender();
int com_reset_appender();
int close_log(int atonce=1000);
int reset_log();
int close_log_r();

}



#endif  //__LOGCORE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
