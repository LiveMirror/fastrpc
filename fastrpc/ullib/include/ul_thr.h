/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: ul_thr.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_thr.h
 * @author baonh(baonh@baidu.com)
 * @date 2007/12/10 14:40:56
 * @version $Revision: 1.2 $ 
 * @brief 多线程库
 *  
 * 该函数库中的函数是对linuxthreads库函数的包装，增加了一定的参数合法性检查和错误日志打印功能, 同时将errno设置为出错返回值返回值
 *
 **/

//**********************************************************
//          Wrap system normal function Utility 1.0
//
//  Description:
//    This utility defines thread functions which were wraped.
//
// Author: Wang Chengqing
// 		   cwang@baidu.com
//**********************************************************

# ifndef __UL_THRWRAPLIB_H__
# define __UL_THRWRAPLIB_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

//*******************************************************
// Wrap function about pthread_attr_destroy()
//*******************************************************
int	ul_pthread_attr_destroy(pthread_attr_t *attr);

//*******************************************************
// Wrap function about pthread_attr_init()
//*******************************************************
int	ul_pthread_attr_init(pthread_attr_t *attr);

//*******************************************************
// Wrap function about pthread_attr_setdetachstate()
//*******************************************************
int	ul_pthread_attr_setdetachstate(pthread_attr_t *attr, int detach);

//*******************************************************
// Wrap function about pthread_attr_setscope()
//*******************************************************
int	ul_pthread_attr_setscope(pthread_attr_t *attr, int scope);

//*******************************************************
// Wrap function about pthread_create()
//*******************************************************
int ul_pthread_create(pthread_t *tid, const pthread_attr_t *attr,
			   void * (*func)(void *), void *arg);

//*******************************************************
// Wrap function about pthread_join()
//*******************************************************
int	ul_pthread_join(pthread_t tid, void **status);

//*******************************************************
// Wrap function about pthread_detach()
//*******************************************************
int	ul_pthread_detach(pthread_t tid);

//*******************************************************
// Wrap function about pthread_kill()
//*******************************************************
int ul_pthread_kill(pthread_t tid, int signo);

//*******************************************************
// Wrap function about pthread_self()
//*******************************************************
pthread_t ul_pthread_self(void);

//*******************************************************
// Wrap function about pthread_exit()
//*******************************************************
void	ul_pthread_exit(void *status);

//******************************************************
// Wrap function about pthread_exit()
//*******************************************************
int	ul_pthread_setcancelstate(int state, int *oldstate);

//*******************************************************
// Wrap function about pthread_mutex_init()
//*******************************************************
int ul_pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr);

//*******************************************************
// Wrap function about pthread_mutex_destroy()
//*******************************************************
int	ul_pthread_mutex_destroy(pthread_mutex_t *mptr);

//*******************************************************
// Wrap function about pthread_mutex_lock()
//*******************************************************
int ul_pthread_mutex_lock(pthread_mutex_t *mptr);

//*******************************************************
// Wrap function about pthread_mutex_unlock()
//*******************************************************
int	ul_pthread_mutex_unlock(pthread_mutex_t *mptr);

//*******************************************************
// Wrap function about pthread_cond_init()
//*******************************************************
int	ul_pthread_cond_init(pthread_cond_t *cptr, pthread_condattr_t *cond_attr);

//*******************************************************
// Wrap function about pthread_cond_destroy()
//*******************************************************
int	ul_pthread_cond_destroy(pthread_cond_t *cptr);

//*******************************************************
// Wrap function about pthread_cond_broadcast()
//*******************************************************
int	ul_pthread_cond_broadcast(pthread_cond_t *cptr);

//*******************************************************
// Wrap function about pthread_cond_signal()
//*******************************************************
int	ul_pthread_cond_signal(pthread_cond_t *cptr);

//*******************************************************
// Wrap function about pthread_cond_wait()
//*******************************************************
int	ul_pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr);

//*******************************************************
// Wrap function about pthread_cond_timedwait()
//*******************************************************
int ul_pthread_cond_timedwait(pthread_cond_t *cptr, pthread_mutex_t *mptr,
					   const struct timespec *tsptr);

//*******************************************************
// Wrap function about pthread_mutexattr_init()
//*******************************************************
int	ul_pthread_mutexattr_init(pthread_mutexattr_t *attr);

//*******************************************************
// Wrap function about pthread_mutex_destroy()
//*******************************************************
int	ul_pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

//*******************************************************
// Wrap function about pthread_condattr_init()
//*******************************************************
int	ul_pthread_condattr_init(pthread_condattr_t *attr);

//*******************************************************
// Wrap function about pthread_condattr_destroy()
//*******************************************************
int	ul_pthread_condattr_destroy(pthread_condattr_t *attr);

//*******************************************************
// Wrap function about pthread_once()
//*******************************************************
int	ul_pthread_once(pthread_once_t *ptr, void (*func)(void));

//*******************************************************
// Wrap function about pthread_key_create()
//*******************************************************
int	ul_pthread_key_create(pthread_key_t *key, void (*func)(void *));

//*******************************************************
// Wrap function about pthread_setspecific()
//*******************************************************
int	ul_pthread_setspecific(pthread_key_t key, const void *value);

//*******************************************************
// Wrap function about pthread_getspecific()
//*******************************************************
void *ul_pthread_getspecific(pthread_key_t key);

#endif

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
