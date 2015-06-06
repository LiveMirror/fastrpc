/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: comsig.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file comsig.h
 * @author feimat(com@baidu.com)
 * @date 2008/03/31 12:02:33
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __COMSIG_H_
#define  __COMSIG_H_

struct com_logstat_t;

namespace comspace
{

class LogStat;

typedef int (*cb_loadconf)(com_logstat_t *lg, const char *path, const char *file);

//动态管理日志等级
int reg_logstat(LogStat *, cb_loadconf lc, const char *path, const char *file);
//按comlog的配置文件分隔自动load配置文件
int reg_logstat(LogStat *, const char *path, const char *file);

int reg_deal();

};

#endif  //__COMSIG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
