/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: checksvr.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file checksvr.h
 * @author feimat(com@baidu.com)
 * @date 2008/03/19 21:38:49
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __CHECKSVR_H_
#define  __CHECKSVR_H_

#include "comlog/comlog.h"
#include <pthread.h>

namespace comspace
{

class SendSvr;
class CheckSvr
{
	SendSvr *_servers[COM_MAXSERVERSUPPORT];
	int _size;
	pthread_mutex_t _lock;
	pthread_t _tid;
	int _sleeptime;
public:
	CheckSvr();
	int registerServer(SendSvr *);
	int check();
	void clear();

	int checkOver(int waittime);
};
}

#endif  //__CHECKSVR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
