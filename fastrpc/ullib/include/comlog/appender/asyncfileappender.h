/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: asyncfileappender.h,v 1.4 2009/09/18 11:02:57 sun_xiao Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file asyncfileappender.h
 * @author feimat(com@feimat.com)
 * @date 2008/08/28 18:06:09
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __ASYNCFILEAPPENDER_H_
#define  __ASYNCFILEAPPENDER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>

#include "comlog/appender/appender.h"
#include "comlog/appender/fileappender.h"
#include "comlog/appender/netappender.h"
#include "comlog/xqueue.h"

#ifdef _XLOG_MEMPOOL_USED_
#include "comlog/comempool/mempool.h"
#endif

namespace comspace 
{

class AsyncFileAppender : public Appender
{
	struct stat _stat;
	char _file[COM_MAXFILENAME];
	int _page;
	time_t _lastcutime;

	typedef NetAppender::netdata_t _netdata_t;
	typedef void * vptr_t;
	XSigQu<vptr_t> _queue;
	pthread_t _tid;

	int _run;
    int _stop;
    
	static const int AFILE_MAXREADITEMS = 1024;
	vptr_t _readbuf[AFILE_MAXREADITEMS];
	iovec afio[AFILE_MAXREADITEMS];

	static const int AFILE_DEFPOPTO = 5;
	static const int AFILE_QUEUESIZE = 20*1024;

#ifdef _XLOG_MEMPOOL_USED_
	comspace::MemPool _mempool;
#endif
public:
	int open(void *);
	int close(void *);
	int print(Event *evt);
	int binprint(void *, int siz);
	int writeData();//like run
	//int flush();
	~AsyncFileAppender();
protected:
	int syncid(void *);
	int erasedata(int);

	AsyncFileAppender();

public:
	static Appender *getAppender(com_device_t &dev);
	static Appender *tryAppender(com_device_t &dev);
protected:
	friend class Appender;
	friend class FileAppender;
	friend class UlLogAppender;
	friend class NetAppender;
};

};












#endif  //__ASYNCFILEAPPENDER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
