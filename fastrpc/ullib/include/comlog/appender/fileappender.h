/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: fileappender.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file fileappender.h
 * @author feimat(com@baidu.com)
 * @date 2008/03/18 22:15:24
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __FILEAPPENDER_H_
#define  __FILEAPPENDER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "comlog/appender/appender.h"

namespace comspace 
{

class FileAppender : public Appender
{
	struct stat _stat;
	char _file[COM_MAXFILENAME];
	int _page;
	time_t _lastcutime;

public:
	int open(void *);
	int close(void *);
	int print(Event *evt);
	int binprint(void *, int siz);
	int reset();
	~FileAppender();
protected:
	int syncid(void *);

	FileAppender();

public:
	static Appender *getAppender(com_device_t &dev);
	static Appender *tryAppender(com_device_t &dev);
protected:
	friend class Appender;
	//friend class UlFileAppender;
	friend class UlLogAppender;
	friend class NetAppender;
};

};












#endif  //__FILEAPPENDER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
