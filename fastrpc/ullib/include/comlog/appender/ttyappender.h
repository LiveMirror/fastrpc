/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: ttyappender.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file appender/ttyappender.h
 * @author feimat(com@feimat.com)
 * @date 2008/03/16 20:29:57
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __APPENDER_TTYAPPENDER_H_
#define  __APPENDER_TTYAPPENDER_H_

#include "comlog/appender/appender.h"

namespace comspace
{

class TtyAppender : public Appender
{
public:
	TtyAppender();
	int open(void *);
	int close(void *);
	int print(Event *evt);
	int binprint(void *, int);

public:
	static Appender *getAppender(com_device_t &dev);
	static Appender *tryAppender(com_device_t &dev);
protected:
	friend class Appender;
};

};













#endif  //__APPENDER/TTYAPPENDER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
