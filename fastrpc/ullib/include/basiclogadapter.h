/***************************************************************************
 * 
 * Copyright (c) 2009 feimat.com, Inc. All Rights Reserved
 * $Id: basiclogadapter.h,v 1.4 2009/09/18 11:02:57 sun_xiao Exp $ 
 * 
 **************************************************************************/



/**
 * @file basiclogadapter.h
 * @author feimat(com@feimat.com)
 * @date 2009/03/05 16:46:36
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __BASICLOGADAPTER_H_
#define  __BASICLOGADAPTER_H_

#include "com_log.h"

//为mask添加一个日志等级,mask应该是一个uint32_t
#define COMLOG_MASK(mask, level) do{\
	(mask) |= (unsigned int)(1) << (level);\
}while(0)

//为mask添加5个默认的系统日志等级,mask是一个uint32_t
#define COMLOG_DEFAULT_MASK(mask) do{\
	COMLOG_MASK(mask, COMLOG_FATAL);\
	COMLOG_MASK(mask, COMLOG_WARNING);\
	COMLOG_MASK(mask, COMLOG_NOTICE);\
	COMLOG_MASK(mask, COMLOG_TRACE);\
	COMLOG_MASK(mask, COMLOG_DEBUG);\
}while(0)

/**
 * @brief 日志接口，向下调用comlog或者ullog打印日志
 *        BasicLogAdapter不会释放任何资源（包括自己创建的资源）
 *        一般来说，BasicLogAdapter的生命周期是整个进程
 *        如要释放，只能使用com_closelog()
 *
 * 		  程序默认使用ullog打印日志
 *        当调用setCategory, setDevice, setLayout等函数，
 *        会自动切换到comlog的规则
 *
 *        setLevel是总控开关，对comlog/ullog均生效。（自定义日志不受约束）
 *
 *        如果要打印自定义等级的日志，必须显式调用setLogStat，示例：
 *        BasicLogAdapter *p = new BasicLogAdapter();
 *        p->setDevice(xxx);//注册Device
 *        comspace::LogStat lstat = comspace::LogStat::createLogStat();//需要在setDevice之后调用
 *        //因为createLogStat必须在comlog被初始化之后才可以使用，所以必须先执行setDevice，里面会做初始化
 *        uint32_t level = 0;
 *        COMLOG_DEFAULT_MASK(level);
 *        lstat.setLevel(level, "SelfName1, SelfName2"); //这个SelfName1也需要在配置文件中的selfdefine中定义
 *        p->setLogStat(lstat);
 *        p->write("SelfName1", "This is a self level log...");
 *        //或者：
 *        int self = comspace::getLogId("SelfName1");
 *        p->write(self, "This is also a self level log...");
 *        
 */
class BasicLogAdapter{
	protected:
		//实际的日志打印句柄
		comspace :: Category * _cate;
		//系统日志等级
		int _level;
	public:
		BasicLogAdapter();
		/**
		 * @brief 直接设置Category句柄?comlog)
		 *
		 * @param [in] cate   : comspace::Category*
		 * @return  int 0成功，其余失败
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 16:58:57
		**/
		int setCategory(comspace::Category * cate);
		/**
		 * @brief 添加comlog的设备 
		 *
		 * @param [in] dev   : com_device_t* 设备
		 * @param [in] num   : int 设备数
		 * @return  int -1出错，否则返回成功添加的设备数量
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:01:07
		**/
		int setDevice(com_device_t * dev, int num);
		/**
		 * @brief 使用logstat设置日志等级控制，只对comlog生效。
		 *
		 * @param [in] logstat   : comspace::LogStat*
		 * @return  int 0成功，其余失败
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:04:07
		**/
		int setLogStat(comspace::LogStat * logstat);
		/**
		 * @brief 设置日志打印的总控等级，不论是comlog打印还是ullog打印，均生效。
		 *
		 * @param [in] level   : int
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:05:03
		**/
		int setLevel(int level);
        /**
         * @brief 获取日志等级
         *
         * @return  int 
         * @retval   
         * @author baonh
         * @date 2009/07/25 21:59:36
        **/
        int getLevel();
		/**
		 * @brief 打印日志的标准接口
		 *
		 * @param [in] level   : int 日志等级
		 * @param [in] fmt   : const char* 日志内容
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:06:03
		**/
		int write(int level, const char * fmt, ...) __attribute__ ((format (printf,3,4)));
		/**
		 * @brief 打印自定义级别的日志
		 *
		 * @param [in] level   : const char* 日志等级名
		 * @param [in] fmt   : const char* 日志内容
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:06:32
		**/
		int write(const char * level, const char * fmt, ...) __attribute__ ((format (printf,3,4)));
		//----- 以下接口相当于write的包装---
		/**
		 * @brief 打印fatal日志
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:08:29
		**/
		int fatal(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief 打印warning日志
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:08:49
		**/
		int warning(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief 打印notice日志
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:03
		**/
		int notice(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief 打印trace日志
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:14
		**/
		int trace(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief 打印debug日志
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:26
		**/
		int debug(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
	protected:
		/**
		 * @brief 真正向下调用的日志接口，接收参数为已经打包好的va_list
		 *
		 * @param [in] level   : int
		 * @param [in] fmt   : const char*
		 * @param [in] args   : va_list
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:37
		**/
		int write_args(int level, const char * fmt, va_list args);
		comspace :: Category * tryCategory();
		static int _instance;
		static pthread_mutex_t _lock;
};

















#endif  //__BASICLOGADAPTER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
