/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: namemg.h,v 1.2.22.2 2010/04/05 13:30:46 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
/**
 * @file namemg.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/02/13 10:56:50
 * @version $Revision: 1.2.22.2 $ 
 * @brief 
 *  
 **/

#ifndef  __NAMEMG_H_
#define  __NAMEMG_H_

#include "xhash.h"
#include "appender/appender.h"
#include "sendsvr/sendsvr.h"
#include "logstat.h"

//////#ifndef u_int64
////#define u_int64 unsigned long long
//#endif

namespace comspace
{

static const int type_category = 1;
static const int type_appender = 2;
static const int type_layout = 3;
static const int type_sendsvr = 4;
static const int type_logstat = 5;

//多读一写线程安全
class NameMg
{
private:
	struct node_t
	{
		int type;
		void *ptr;
	};
	pthread_mutex_t _lock;
	comspace::hash_map<unsigned long long, node_t> _hash;
	static void __delete_node__(const node_t &t, void *);
	static void __delete_sendsvr__(const node_t &t, void *);
	static void __close_appender__(const node_t &t, void *);
	static void __reset_appender__(const node_t &t, void *);
public:
	int create();
	int destroy();
	int closeAppender();
	int resetAppender();
	unsigned long long getKey(int tag, const char *name);
	int set(const char *name, Appender * app);
	//int get(const char *name, Appender * &app);
	int set(const char *name, SendSvr * svr);
	//int get(const char *name, SendSvr * &svr);
	int set(const char *name, LogStat * logstat);
	//int get(const char *name, LogStat * &logstat);
	int set(const char *name, Category * cate);
	//int get(const char *name, Category * &cate);
	int set(const char *name, Layout * layout);
	//int get(const char *name, Layout * &layout);

	void * get(int tag, const char *name);
private:
	int set(int tag, const char *name, void *); 
};

//extern NameMg g_nameMg;
NameMg * comlog_get_nameMg();
void comlog_del_nameMg();
}
#endif  //__NAMEMG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
