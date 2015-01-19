/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: ul_log.h,v 1.2.40.1 2010/04/15 06:38:17 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_log.h
 * @author baonh(baonh@baidu.com)
 * @version $Revision: 1.2.40.1 $ 
 * @brief  日志库 
 * 
 * 日志的使用如下
 * 
 * 多线程程序:
 *
 * 主进程              子线程 1         子线程 2 ...
 *
 *  ul_openlog
 *
 *      |
 *
 *      ------------------------------------
 *
 *      |               |               |
 *
 *  ul_writelog  ul_openlog_r       ul_openlog_r
 *
 *     ...              |               |
 *
 *     ...       ul_writelog        ul_writelog
 *
 *     ...           ...              ...
 *
 *               ul_closelog_r      ul_closelog_r
 *
 *                      |               |
 *
 *      --------------------------------------
 *
 *      |
 *
 *  ul_closelog
 *
 * 单进程
 *
 *  ul_openlog
 *
 *      |
 *
 *  ul_writelog
 *
 *     ...
 *
 *      |
 *
 *  ul_closelog
 *
 *  如果在ul_openlog前调用ul_writelog,
 *  日志信息会被输出到标准出错(stderr)上,日志库支持多线程呈现,但不支持多进程程序,
 *  多进程中使用请保证每个进程使用的是不用的日志文件
 *
 **/

#ifndef __UL_LOG_H__
#define __UL_LOG_H__

#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <syslog.h>
#include <signal.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

#include "ul_def.h"
#include "dep/dep_log.h"

#define ERR_NOSPACE		-1
#define ERR_OPENFILE	-2
#define ERR_EVENT		-3
#define ERR_FILEFULL	-4

#define LOG_FILE_NUM	2048		      /**< 最多可同时打开的文件数      */
#define LOG_BUFF_SIZE_EX 2048			  /**< 一条日志buff大小       */

#define MAX_FILE_SIZE		2045		  /**< 最大文件大小单位(MB)      */
#define MAX_FILENAME_LEN	1024		  /**< 日志文件名的最大长度       */


/** @brief 事件类型  */
#define UL_LOG_WFSTART	-2
#define UL_LOG_START	-1
#define UL_LOG_END		UL_LOG_START
#define UL_LOG_WFEND	UL_LOG_WFSTART

#define UL_LOG_NONE		0
#define UL_LOG_FATAL	0x01    /**<   fatal errors */
#define UL_LOG_WARNING	0x02    /**<   exceptional events */
#define UL_LOG_NOTICE   0x04    /**<   informational notices */
#define UL_LOG_TRACE	0x08    /**<   program tracing */
#define UL_LOG_DEBUG	0x10    /**<   full debugging */
#define UL_LOG_ALL		0xff    /**<   everything     */

#define UL_LOG_SELF_BEGIN   0x100
#define UL_LOG_SELF_END     0x107
#define UL_LOG_SELF_MASK    0xff

/* ul_log_t  log_sepc */
#define UL_LOGTTY		0x02    /**<   日志在输出到日志文件的同时输出到标准出错(stderr)中 */
#define UL_LOGNEWFILE	0x08    /**<   创建新的日志文件,可以使每个线程都把日志打到不同文件中*/
#define UL_LOGSIZESPLIT 0x10    /**<  按大小分割日志文件，不回滚*/
/* ul_file_t  flag */
#define UL_FILE_TRUNCATE	0x01
#define UL_FILE_FULL		0x02

#define UL_LOG_RECORD	15	

#define MAX_SELF_DEF_LOG 8		  /**< 单个线中自定义日志的日志数上限      */
/** @brief 日志相关参数 */

struct ul_logstat_t {
        int events;		/**< 需要打的日志级别 0-15 */
        int to_syslog;	/**< 输出到syslog 中的日志级别, 0-15 */
        int spec;		/**< 扩展开关 0 or @ref UL_LOGTTY or @ref UL_LOGNEWFILE */
};
typedef struct ul_logstat_t ul_logstat_t;

struct ul_file_t {
	FILE *fp;							/**< 文件句柄 */
	int  flag;							/**< 标志  @ref UL_FILE_TRUNCATE | @ref UL_FILE_FULL */
	int  ref_cnt;						/**< 引用计数 */
	int  max_size;						/**< 文件可以记录的最大长度 */
	pthread_mutex_t file_lock;			/**< 写文件锁 */
	char file_name[MAX_FILENAME_LEN+1];	/**< 文件名字 */
};
typedef struct ul_file_t ul_file_t;

struct ul_log_t {
	char used;							/**< 0-未使用  1-已使用 */
	ul_file_t *pf;						/**< log */
	ul_file_t *pf_wf;					/**< log.wf */
	pthread_t tid;						/**< 线程id  实际使用上是使用gettid,而非pthread_self*/
	int  mask;							/**< 可以记录的事件的掩码 */
	int  log_syslog;					/**< 输出到系统日志的事件掩码 */
	int  log_spec;						/**< UL_LOGTTY | UL_LOGNEWFILE */
	ul_file_t *spf[MAX_SELF_DEF_LOG];	/**< 自定义日志文件句柄 */
};
typedef struct ul_log_t ul_log_t;

struct Ul_log_self {					/**< 兼容旧模式，定义不加 _t       */
	char name[MAX_SELF_DEF_LOG][PATH_SIZE];		/**< 自定义日志文件名，系统自动在文件名后加后缀.sdf */
	char flags[MAX_SELF_DEF_LOG];				/**<决定当前自定义的日志是否输出,设置为1则生成自定义日志,0则不生成 */
	int  log_number;							/**< 自定义文件的数目,当设置为0时,不生成自定义文件 */
}; //自定义日志的设置,可在ul_openlog以及ul_openlog_r中作为参数传入,设置UL_LOGNEWFILE时,自定义日志与正常日志一样,
			 //为线程文件,否则,自定义日志为进程级日志
typedef struct Ul_log_self ul_log_self_t;


/**
 * @brief 打开日志文件（log和log.wf）并初始化日志对象(包括attach共享内存)
 *
 * @param [in] log_path : 日志文件所在目录 
 * @param [in] log_procname : 日志文件名前缀。如果文件名中包含'_'，则截断为'_'之前的字符串
 * @param [in] l_stat : 日志相关参数(用来设置log的特性)
 * @param [in] maxlen : 单个日志文件的最大长度（unit: MB）
 * @param [in] self :  设置自定义log,具体使用方式请参见结构说明
 * @return 0成功，-1失败
 * @note 退出时需要调用ul_closelog释放资源
 * @see ul_closelog ul_openlog_r ul_closelog_r
 */
extern int ul_openlog(const char *log_path, const char *log_procname, ul_logstat_t  *l_stat,
		int maxlen, Ul_log_self *self = NULL);


/**
 * @brief 打印指定级别的字符串到对应的日志文件
 *
 * @param [in] event : 日志级别 (include UL_LOG_DEBUG,UL_LOG_TRACE,UL_LOG_NOTICE,UL_LOG_WARNING,UL_LOG_FATAL)
 * @param [in] fmt : 格式串
 * @param [in] ... : 打印参数列表
 * @return 0成功，<0失败
 * @note 日志长度经过扩展，线程级日志可支持最大为2048字节，共享内存中的log信息在超过640字节依然会截断。
 */
#ifdef  _UN_WRITELOG_FORMAT_CHECK
extern int ul_writelog(const int event, const char* fmt, ... );
#else
extern int ul_writelog(const int event, const char* fmt, ... ) __attribute__ ((format (printf,2,3)));
#endif

/**
 * @brief 关闭日志文件并释放相关资源
 *
 * @param [in] iserr : 在日志中记录结束状态。0为正常，非0为不正常
 * @return 0成功，-1失败
 */
extern int ul_closelog(int iserr);

/**
 * @brief 为本线程打开日志
 *
 * @param [in] threadname 用于打印的线程名字(如果设置了UL_LOGNEWFILE,会根据threadname创建新的log文件)
 * @param [in] l_stat 日志相关参数
 * @param [in] self : 设置自定义log,具体使用方式请参见结构说明
 * @return 0成功，-1失败
 * @see ul_closelog_r ul_openlog ul_closelog
 * @note 当线程退出以后，要使用ul_closelog_r来释放资源
 */
extern int ul_openlog_r(const char *threadname, ul_logstat_t  *l_stat, Ul_log_self *self = NULL);

/**
 * @brief 关闭本线程的日志
 *
 * @param [in] iserr : 在日志中记录结束状态。0为正常，非0为不正常
 * @return 0成功，-1失败
 */
extern int ul_closelog_r(int iserr);

/**
 * @brief 获得全局日志等级
 *
 * @return  int 
**/
extern int ul_get_log_level();

/**
 * @brief 日志等级是否在接收范围内
 *
 * @return  int 
**/
extern int ul_log_enabled(int level);

#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
