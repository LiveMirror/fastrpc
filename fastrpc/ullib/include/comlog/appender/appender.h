/***************************************************************************
* 
* Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
* $Id: appender.h,v 1.2.22.3 2010/04/05 13:30:46 zhang_rui Exp $ 
* 
**************************************************************************/



/**
* @file appender.h
* @author feimat(feimat@baidu.com)
* @date 2008/02/12 23:54:09
* @version $Revision: 1.2.22.3 $ 
* @brief 
*  
**/


#ifndef  __APPENDER__APPENDER_H_
#define  __APPENDER__APPENDER_H_

#include "comlog/comlog.h"

namespace comspace
{
class Appender
{
public:
	com_device_t _device;	//设备名		  /**<        */
	int  _id;			//设备句柄		  /**<        */
	int _open;		  /**<        */
	Layout *_layout;				//layout		  /**<        */
	//u_int64 _mask;					//日志等级掩码		  /**<        */
	unsigned long long _mask;					//日志等级掩码		  /**<        */
	pthread_mutex_t _lock;		  /**<        */
	Appender * _bkappender;		  /**<        */

public:	
	/**
	 * @brief destructor
	 *
	**/
	virtual ~Appender();
	/**
	 * @brief set layout
	 *
	 * @param [in/out] layout   : Layout*
	 * @return  int 
	 * @retval   
	**/
	int setLayout(Layout *layout);	//设置图层
	/**
	 * @brief set device info
	 *
	 * @param [in/out] dev   : com_device_t&
	 * @return  int 
	 * @retval   
	**/
	int setDeviceInfo(com_device_t &dev);
	
	/**
	 * @brief reset mask
	 *
	 * @return  int 
	 * @retval   
	**/
	int resetMask();	//重置日志等级掩码，默认配置是支持系统日志等级的打印
	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	int emptyMask();	//清空日志等级的掩码，这样就不支持打印任何日志等级
	/**
	 * @brief 
	 *
	 * @param [in/out] id   : int
	 * @return  int 
	 * @retval   
	**/
	int addMask(int id);	//添加支持一个日志等级
	/**
	 * @brief 
	 *
	 * @param [in/out] id   : int
	 * @return  int 
	 * @retval   
	**/
	int delMask(int id);	//减少支持一个日志等级
	/**
	 * @brief 
	 *
	 * @param [in/out] id   : int
	 * @return  bool 
	 * @retval   
	**/
	bool unInMask(int id);	//是否在yanma中

	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	virtual int open(void *) = 0;	//打开设备句柄
	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	virtual int close(void *) = 0;	//关闭设备句柄
	//stop(): 停止接收日志。FileAppender可以无视此消息。
	//但AsyncFileAppender/NetAppender等异步打印的日志，此时应退出自己的线程
	//主要作用是清零线程计数器（comlog.cpp中的g_close_atomc）
	virtual int stop(){return 0;}

	virtual int reset(){return 0;}

	/**
	 * @brief print
	 *
	 * @param [in/out] evt   : Event*
	 * @return  int 
	 * @retval   
	**/
	virtual int print(Event *evt) = 0;	//将日志实际追加到设备中
	/**
	 * @brief binprint
	**/
	virtual int binprint(void *, int siz) = 0;	//输入二进制数据

	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	virtual int flush();	//立刻将数据写入硬盘
protected:
	/**
	 * @brief creator
	**/
	Appender(); 

public:
	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	virtual int syncid(void *);	//检测当前id和设备是否一致，如果不一致根据设备名更新id，返回0表示成功，其他失败。

protected:

	//根据传入的参数，获取一个设备
//	static pthread_mutex_t getlock;
public:
	/**
	 * @brief get appender
	 *
	 * @param [in/out] dev   : com_device_t&
	 * @return  Appender* 
	 * @retval   
	**/
	static Appender * getAppender(com_device_t &dev);
	/**
	 * @brief 
	 *
	 * @param [in/out] dev   : com_device_t&
	 * @return  Appender* 
	 * @retval   
	**/
	static Appender * tryAppender(com_device_t &dev);
	//为close日志的时候提供的资源统一释放
	/**
	 * @brief 
	 *
	 * @return  int 
	 * @retval   
	**/
	static int destroyAppenders();
	friend class AppenderFactory;		  /**<        */
};

};
#endif  //__APPENDER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
