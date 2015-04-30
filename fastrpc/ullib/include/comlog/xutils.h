/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: xutils.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file xutils.h
 * @author feimat(com@feimat.com)
 * @date 2008/03/16 18:28:32
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __XUTILS_H_
#define  __XUTILS_H_

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "comlog.h"
#include "sendsvr/sendsvr.h"

namespace comspace
{

int x_creat_sign_f64(char* psrc,int slen,u_int* sign1,u_int * sign2);
int xfmtime(char *buf, int siz, timeval &tv);
int xfmtime(char *buf, int siz, time_t tv);
int save_backup(const char *file, char *dest, int siz, const char *fmt = NULL, bool recur = true);
int xstrcpy(char *dest, int siz, const char *src);
int xstrcpy(char *dest, int destsiz, const char *src, int srcsiz);

//返回值为读取了多少个items
int xparserSvrConf(const char *ip, SendSvr::server_t *vecs, int siz);
int deviceNetToFile(com_device_t &in, com_device_t &out);
int xparserSelfLog(const char *str, char (*res)[COM_MAXLEVELNAMESIZE], int items);

typedef void (*xsigfunc_t)(int);
xsigfunc_t xsignal(int signo, xsigfunc_t func);

class xAutoLock
{
public:
    xAutoLock(pthread_mutex_t *m) : mutex(m) {
        pthread_mutex_lock(mutex);
    }
    ~xAutoLock() {
        pthread_mutex_unlock(mutex);
    }
private:
    xAutoLock(); // forbidden
    pthread_mutex_t *mutex;
};

#define XDIFFTIME(s, e) \
	(int)(((e.tv_sec - s.tv_sec) * 1000 + (e.tv_usec - s.tv_usec) / 1000))


void safe_msleep(int ms);


}





#endif  //__XUTILS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
