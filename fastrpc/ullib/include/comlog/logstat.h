/***************************************************************************
* 
* Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
* $Id: logstat.h,v 1.2.22.2 2010/04/05 13:30:46 zhang_rui Exp $ 
* 
**************************************************************************/



/**
* @file logstat.h
* @author feimat(feimat@feimat.com)
* @date 2008/02/12 23:54:03
* @version $Revision: 1.2.22.2 $ 
* @brief 
*  
**/


#ifndef  __LOGSTAT_H_
#define  __LOGSTAT_H_

#include <ul_def.h>

//////#ifndef u_int64
////#define u_int64 unsigned long long
//#endif

namespace comspace
{

class LogStat
{
protected:
	//判断一个日志等级是否可以打印的时候，只需要
	//(1<<level) & _level即可
	unsigned long long _level;	//当前的系统日志等级，用bitmap形式存储，等级过来要跟它&一下
	int _tty;

	char _conf_file[1024];	//配置文件名
	//配置解析的方法
	//int (*_load)(LogStat *);

public:
	//定期从配置文件中load 配置数据用于运行时态动态调整日志管理数据
	//conf为配置文件所在位置
	//在调用这个函数的时候会判断_pid线程是否创建，如果没有创建，创建这个线程。
	//将该句柄注册到定期load的线程扫描数据中
	//int setAutoUpdate(const char *conf, int (*load)(LogStat *));
	//设置系统等级和用户等级，系统等级是一个int，用户等级是以逗号或者空格分割的自定义日志名
	int setLevel(u_int sysmask, char *userlevel);
	inline int setTty(u_int tty) {
		return (_tty = tty);
	}
	inline bool unAccept(int level) {
		return ((((unsigned long long)1)<<level) & _level) == (unsigned long long)0;
	}

private:
	LogStat();

public:
	//创建一个新的句柄
	static LogStat *createLogStat();
	//销毁创建的所有LogStat
	//static int destroyLogStats();

	friend class Category;
};


//extern LogStat * g_def_logstat;LogStat * comlog_get_def_logstat();void comlog_set_def_logstat(LogStat *st);
};
#endif  //__LOGSTAT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
