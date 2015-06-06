/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: eventsvr.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file eventsvr.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/02/12 23:57:32
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __EVENTSVR_H_
#define  __EVENTSVR_H_
namespace comspace
{
/*
 * 当线程的命令队列，用来处理日志库的全集队列
 * 当前用到的就是注册一个定期在日志的回调，用来在运行时态动态修改日志
 */
class EventSvr
{
public:
	//处理的事件格式
	struct event_t
	{
		void *_param;	//回调函数的参数
		void *(*_callback)(void *param);	//处理事件的回调函数
	};

public:
	//添加线程处理时间
	int addEvent(event_t *evt);
	//销毁线程处理时间
	int removeEvent(event_t *evt);
	//将线程安全退出
	int safexit();

private:
	//启动线程，在第一次调用addevent的时候调用
	int startThread();
	EventSvr();

private:

	pthread_t _tid;	//线程id

	int _monitor_time;	//监控时间间隔
	bool _running;		//是否在运行
public:
	//获取全局唯一的句柄
	static LogThread * getInstance();
};
}
#endif  //__LOGTHREAD_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
