/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: sendsvr.h,v 1.3.2.1 2010/03/25 08:18:08 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file sendsvr.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/03/07 18:25:47
 * @version $Revision: 1.3.2.1 $ 
 * @brief
 *  
 **/


#ifndef  __SENDSVR_H_
#define  __SENDSVR_H_

#include "comlog/xqueue.h"
#include "comlog/sendsvr/checksvr.h"
namespace comspace
{
class Appender;
class SendSvr
{
public:
	static const int MAXREADITEMS = 640;
	static const int DEFPOPTO = 5;
	static const int MAXSVRNAMESIZE = 1024;
	static const int DEFMAXAPPENDER = 1024;
	static const int DEFQUEUESIZE = 20 * 1024;
	static const int CHECKTIMEOUT = 10000;

public:
	struct server_t
	{
		char host[COM_MAXHOSTNAME];
		char path[COM_MAXFILENAME];
		int port;
		int sock;
		bool isvalid;
	};

	typedef void * vptr_t;
	pthread_mutex_t _lock;
	XSigQu<vptr_t> _queue;
	vptr_t _readbuf[MAXREADITEMS];
	
	server_t _svrs[COM_MAXDUPSERVER];
	char _name[MAXSVRNAMESIZE];
	int _svrsiz;

	Appender *_inappender[DEFMAXAPPENDER];
	int _inappsize;

	//char _sendbuf[1024*640];
	char _sendbuf[1<<10];

	int _wtto;
	int _rdto;
	int _coto;

	int _run;

	pthread_t _tid;
public:
	SendSvr();
	int registAppender(Appender *app);
	virtual int push(vptr_t);
	//ip:port ip:port
	virtual int createServer(const char *iplist, bool bCheckAlive = true);
	virtual int run(){return 0;};
	virtual int erasedata(int items);
	int _checkalive(int ms);
	int checkalive(int ms);
	int tryalive(int ms);
	int isRun(){ return _run; }
	virtual ~SendSvr();
	virtual int stop();
public:
//	static CheckSvr g_checkSvr;
	static CheckSvr * comlog_get_gcheckSvr();
	static void comlog_del_gcheckSvr();
//	static pthread_mutex_t g_sendsvr_lock;
	static pthread_mutex_t * get_sendsvr_lock();

protected:
	virtual int initServer(const char *iplist) = 0;

protected:
	//int connect();
	int close_safe(int pos);
};
}
#endif  //__SENDSVR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
