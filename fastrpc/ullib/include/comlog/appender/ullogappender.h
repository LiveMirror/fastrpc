/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ullogappender.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file appender/ullogappender.h
 * @author feimat(com@baidu.com)
 * @date 2008/03/30 21:34:22
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __APPENDER_ULLOGAPPENDER_H_
#define  __APPENDER_ULLOGAPPENDER_H_

#include "comlog/appender/appender.h"

namespace comspace
{

class UlLogAppender : Appender
{
	Appender *_wfapp;
	Appender *_ntapp;
	Appender *_sfapp;

public:
	int open(void *);
	int close(void *);
	int print(Event *evt);
	int binprint(void *, int siz);


protected:
	UlLogAppender();
	int syncid(void *);
public:
	static Appender * getAppender(com_device_t &dev);
	static Appender * tryAppender(com_device_t &dev);
protected:
	friend class Appender;
};

};

#endif  //__APPENDER/ULLOGAPPENDER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
