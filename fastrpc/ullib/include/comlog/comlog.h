/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: comlog.h,v 1.5.6.2 2010/04/05 13:30:46 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
/**
 * @file comlog.h
 * @author feimat(feimat@feimat.com)
 * @date 2008/01/30 17:21:07
 * @version $Revision: 1.5.6.2 $ 
 * @brief 
 *  
 **/


#ifndef  __COMLOG_H_
#define  __COMLOG_H_

#include "ul_def.h"
#include "ul_conf.h"

#define COM_USRLEVELSTART  			32		//用户等级id的其实位置
#define COM_LEVELNUM  				64		//最大支持的日志等级
#define COM_SELFLEVELNUM 			32		//最大支持的自定义日志等级数目
#define COM_MAXLEVELNAMESIZE 		1024		//自定义日志等级名的最大长度
#define COM_MAXDEVICENUM			1024		//最大支持的appender数
#define COM_MAXCATEGORYNAMESIZE		64		//category名字的最大长度
#define COM_MAXHOSTNAME				1024	//最大主机名:port/path
#define COM_MAXFILENAME				256		//最大文件名
#define COM_MAXDUPSERVER			32		//服务器冗余台数
#define COM_MAXSERVERSUPPORT 		32		//最大支持连接的server数目
#define COM_MODULENAMESIZE 			64		//模块名的最长长度
#define COM_MAXLOGFILE				2018	//每个日志文件支持的最大长度，单位m
#define COM_MAXCUTTIME				10000000 //最大日志切割时间，单位分?#define COM_MAXAPPENDERNAME			16		//Appender名字的最大长度
#define COM_RESERVEDSIZE			32		//com_device_t中保留字段的长度
#define COM_RESERVEDNUM             64      //com_device_t中保留扩展字段的个数

//标准配置名称
#define COMLOG_LEVEL				"COMLOG_LEVEL"	//日志等级
#define COMLOG_SELFDEFINE			"COMLOG_SELFDEFINE"	//接收的自定义日志
#define COMLOG_DEVICE_NUM			"COMLOG_DEVICE_NUM"	//存储日志的设备数
#define COMLOG_DEVICE				"COMLOG_DEVICE"	//前缀
#define COMLOG_PROCNAME				"COMLOG_PROCNAME"	//该模块的名字
#define COMLOG_LOGLENGTH			"COMLOG_LOGLENGTH" //日志最大长度
#define COMLOG_TIME_FORMAT			"COMLOG_TIME_FORMAT" //日志中时间部分的自定义格式
#define COMLOG_DFS                  "COMLOG_DFS"        //为分布式平台做的扩展
#if 0
//设备类型标志符号
#define COMLOG_DEVICE_FILE			"FILE"					//文件输出
#define COMLOG_DEVICE_NET			"NET"					//网络输出
#define COMLOG_DEVICE_TTY			"TTY"					//终端输出
#define COMLOG_DEVICE_ULLOG			"ULLOG"				//与ullog文件布署一致
#define COMLOG_DEVICE_ULNET			"ULNET"				//与ullog文件布署一致的网络日志
#define COMLOG_DEVICE_AFILE			"AFILE"				//异步（非阻塞）的文件输出
#endif
#define COMLOG_DEVICE_PATH			"PATH"					//设备路径
#define COMLOG_DEVICE_NAME 			"NAME"					//日志文件名
#define COMLOG_DEVICE_AUTH			"AUTH"					//合并授权
#define COMLOG_DEVICE_TYPE			"TYPE"					//设备类型
#define COMLOG_DEVICE_OPEN			"OPEN"					//是否启用这个设备
#define COMLOG_DEVICE_SIZE			"SIZE"					//日志大小
#define COMLOG_DEVICE_SPLITE_TYPE	"SPLITE_TYPE"			//分割类型
#define COMLOG_DEVICE_DATECUTTIME	"DATA_CUTTIME"			//切割周期 分钟为单位，大于0 默认60
#define COMLOG_DEVICE_COMPRESS		"COMPRESS"				//是否支持压缩
#define COMLOG_DEVICE_SYSLEVEL		"SYSLEVEL"				//支持的系统日志等级
#define COMLOG_DEVICE_SELFLEVEL		"SELFLEVEL"				//支持的自定义日志等级
#define COMLOG_DEVICE_LAYOUT		"LAYOUT"				//设备模版
#define COMLOG_DEVICE_LAYOUT_NDC	"LAYOUT_NDC"			//设置ndc模版
#define COMLOG_DEVICE_RESERVED1		"RESERVED1"				//保留字段
#define COMLOG_DEVICE_RESERVED2		"RESERVED2"
#define COMLOG_DEVICE_RESERVED3		"RESERVED3"
#define COMLOG_DEVICE_RESERVED4		"RESERVED4"
//////#ifndef u_int64
////#define u_int64 unsigned long long
//#endif
#if 0
enum {
	COM_DEVICESTART = 0,
	COM_FILEDEVICE = 1,	//文件设备
	COM_NETDEVICE = 2,	//网络设备
	COM_TTYDEVICE = 3,	//终端显示
	COM_ULLOGDEVICE = 4,	//与ul_log文件布署风格一致
	COM_ULNETDEVICE = 5,	//与ullog文件布署一致的网络日志
	COM_AFILEDEVICE = 6,
	COM_DEVICENUM		//支持的设备数
};
#endif
//配置SPLITE_TYPE时，配置项的值可以是下面的字符串，也可以使0，1，2的数字
#define COMLOG_DEVICE_TRUNCT 		"TRUNCT"	
#define COMLOG_DEVICE_SIZECUT 		"SIZECUT"
#define COMLOG_DEVICE_DATECUT		"DATECUT"
enum {
	COM_TRUNCT = 0,		//直接清空日志从打(0)
	COM_SIZECUT,	//按大小分割日志(1)
	COM_DATECUT,	//日期切割(2)
};

enum {
	LOG_NOT_DEFINED = 0, //未显式调用任何openlog/loadlog，默认ullog
	USING_COMLOG,//使用comlog
	USING_ULLOG,//使用ullog
};

#define COMLOG_FATAL 1
#define COMLOG_WARNING 2
#define COMLOG_NOTICE 4
#define COMLOG_TRACE 8
#define COMLOG_DEBUG 16

#define UL_LOGLINE(x) UL_LOGLINE_(x)
#define UL_LOGLINE_(x) #x

#define CFATAL_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_FATAL, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#define CWARNING_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_WARNING, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#define CNOTICE_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_NOTICE, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#ifdef CLOSE_TRACE_LOG
#define CTRACE_LOG(fmt, arg...) ((void *)(0))
#else
#define CTRACE_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_TRACE, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)
#endif

#ifdef CLOSE_DEBUG_LOG
#define CDEBUG_LOG(fmt, arg...) ((void *)(0))
#else
#define CDEBUG_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_DEBUG, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)
#endif

namespace comspace {
	class Layout;
	class Category;
	class Event;
	class Appender;
};

//添加支持的日志等级
#define COMLOG_ADDMASK(dev, loglvl) \
{ \
	(dev).log_mask |= ((unsigned long long)(1)<<loglvl);\
}
//删除支持的日志等级
#define COMLOG_DELMASK(dev, loglvl) \
{ \
	(dev).log_mask &= ~((unsigned long long)(1)<<loglvl); \
}
//清除日志的日志等级
#define COMLOG_CLEARMASK(dev) \
{ \
	(dev).log_mask = 0; \
}
//初始化建议的系统日志等级
#define COMLOG_SETSYSLOG(dev) \
{ \
	COMLOG_ADDMASK(dev, COMLOG_FATAL); \
	COMLOG_ADDMASK(dev, COMLOG_WARNING); \
	COMLOG_ADDMASK(dev, COMLOG_NOTICE); \
	COMLOG_ADDMASK(dev, COMLOG_TRACE); \
	COMLOG_ADDMASK(dev, COMLOG_DEBUG); \
}

//设备名
struct com_device_t
{
	char host[COM_MAXHOSTNAME];	//主机名, ip:port/path 如果主机名为空表示为本地文件
	char file[COM_MAXFILENAME];	//日志打印所在文件
	char auth[COM_MODULENAMESIZE];	//如果非空，那么相同文件名的日志会在同一台主机上合并
	char type[COM_MAXAPPENDERNAME]; //设备类型（appender类型）

	char reserved1[COM_RESERVEDSIZE];//保留字段
	char reserved2[COM_RESERVEDSIZE];
	char reserved3[COM_RESERVEDSIZE];
	char reserved4[COM_RESERVEDSIZE];
    int log_size;		//回滚日志大小 
	char open;			//是否启用
	char splite_type;	//TRUNCT表示直接回滚，SIZECUT 表示按大小分割，DATECUT表示按时间分割
	char compress;		//网络协议，是否压缩 0表示不压缩，1表示压缩
	int cuttime;		//切割时间周期，默认为0，整点切割
	unsigned long long log_mask;		//接收哪些等级日志，默认为0表示接收系统等级日志,ADD_MASK, DEL_MASK 宏来控制			
    //用于扩展的配置数组，考虑到4个reserved字段已经被使用，reserved字段不删除
    char name[COM_MAXAPPENDERNAME]; //配置中的名字
    unsigned int reserved_num;
    ul_confitem_t reservedext[COM_RESERVEDNUM];
    ul_confdata_t reservedconf;
	
	comspace::Layout *layout;		//自定义日志的格式，null表示采用默认格式
public:
	com_device_t();
};

int set_comdevice_default(com_device_t *dev);
int set_comdevice_selflog(com_device_t *dev, const char *logname);

struct com_logstat_t
{
	//跟16表示debug等级8表示trace等级4表示notice等级，2表示warning等级，1表示fatal等级
	int sysevents;			//系统日志等级
	//如果不设置，userevents[0] = 0; 表示打印所有自定义日志
	//设置的话 自定义等级名,自定义等级名；用逗号隔开，表示要打印
	char userevents[1024];	//自定义日志等级
	//1表示在终端打印日志，0表示不显示
	int ttydisp;			//是否在终端显示数据

	com_logstat_t();
};

/**
 * @brief 根据指定设备打开日志
 *
 * @param [in] procname 进程名
 * @param [in] dev   : device_t* 日志输出的设备集合
 * @param [in] dev_num   : int	设备数目
 * @param [in] stat   : logstat_t*	日志等级管理
 * @return  int 成功返回0，失败其他
 * @retval   
 * 	系统日志会根据当前等级向指定的设备打印，
 * 	比如创建了一个文件日志设备，那么它会创建name.log和name.log.wf两个文件
 * 	notice, trace, debug往name.log文件打，fatal, warning往name.log.wf打
 * @see 
 * @author feimat
 * @date 2008/01/30 18:43:11
**/
int com_openlog(const char *procname, com_device_t *dev, int dev_num, com_logstat_t *stat);//打开日志，并指定日志等级，和输出的日志设备

/**
 * @brief 根据配置装载日志，这样的话主线程不须要调用com_openlog
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/26 12:35:29
**/
int com_loadlog(const char *path, const char *file);

/**
 * @brief support multiprocess in bigpipe
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @param [in/out] id     : process id by app, not pid
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/05/24 16:39:29
**/
int com_loadlog_ex(const char *path, const char *file, int id);

/**
 * @brief 初始化线程数据
 *
 * @return  int  0成功，其他－1
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:21:56
**/
int com_openlog_r();


/**
 * @brief 关闭线程数据
 *
 * @return  int 0成功，其他-1
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:22:15
**/
int com_closelog_r();

/**
 * @brief 根据log名返回logid
 *
 * @param [in/out] selflog   : char*	log名
 * @return  int 返回的logid > 0，否者日志不存在 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/15 14:28:57
**/
int com_getlogid(char *selflog);

/**
 * @brief 增加新的设备，如果是主线程添加，是被全局共享，在子线程添加，是被子线程分享
 * 	注意，主线程添加后才create子线程有效，如果是在子线程create完之后添加的，仅是主线程独有的
 * 	如果设备已经存在，会根据selflog内容设置自定义属性
 * 	如果设备不存在会新增
 * 	如果需要设置多个自定义日子，请不断调用这个函数
 *
 * @param [in] dev   : com_device_t*
 * @param [in] nums	: 增加的设备数 
 * @param [in] selflog : 自定义日志名字，null表示没有自定义日子
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/15 14:24:13
**/
int com_setdevices(com_device_t *dev, int nums, char *selflog = NULL);

/**
 * @brief 关闭日志，主线程调用
 *
 * @param [in] waittime 等待日志打完的最大等待时间，如果超过这个时间日志还没发送到指定设备
 * 	强制关闭日志，丢失没有打印出去的日志。设置 0 表示不等待
 * 	 【注意】这里的waittime只对NetAppender有效，如果你使用的AsyncFileAppender或者NetcpAppender
 * 	 这样的异步打印的方式（他们都会启动一个单独的线程负责IO操作），
 * 	 则需要在调用com_closelog之前自行调用sleep函数以等待异步线程的打印操作。
 * 	 否则会强行退出，留在内存中的日志会被丢弃。
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:23:31
**/
int com_closelog(int waittime = 1000);//关闭日志

/**
 * @brief resetlog after fork(2) in multiprocess
 *
 * @return  int 
 * @retval   
 * @see the appender need implement its own reset, now it public/bigpipe-appender
 * has supported it.
 * @author wangshaoyan
 * @date 2012/06/10 22:58:31
**/
int com_resetlog();



/* 关闭该线程往指定appender_type的输出,不影响其它线程
* appender_type :  appender在配置中的类型 
* 返回0 成功 ， 非0 失败
*/
int com_setappendermask(char *appender_type);

/**
 * @brief 根据日志id打印日志
 *
 * @param [in] events   : int 日志等级id
 * @param [in] fmt   : const char* 要打印得日志信息
 * @return  int 成功返回0，其他失败
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 11:58:06
**/
int com_writelog(int events, const char *fmt, ...);//根据日志id打印日志

/**
 * @brief com_writelog_ex
 *
 * @param [in] events : int
 * @param [in] version : int
 * @param [in] type : const char* 
 * @param [in] fmt : const char* 
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/06/18 21:22:06
**/
int com_writelog_ex(int events, int version, const char *type, const char *fmt, ...);

/**
 * @brief 根据日志名打印日志信息
 *
 * @param [in/out] name   : const char* 日志名，非空，必须注册过
 * @param [in/out] fmt   : const char* 日志信息
 * @return  int 成功返回0，失败其他
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 11:58:46
**/
int com_writelog(const char *name, const char *fmt, ...);//根据日志名打印日志

/**
 * @brief com_writelog_ex
 *
 * @param [in] events : const char *
 * @param [in] version : int
 * @param [in] type : const char * 
 * @param [in] fmt : const char * 
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/06/18 21:24:06
**/
int com_writelog_ex(const char *name, int version, const char *type, const char *fmt, ...);

/*
 * @brief 打印key，value队
 * @param [in] key, 键名 
 * @param [in] fmt, 格式化内容
 * @param [in] arg..., 内容数据
 * @return 能够再次打印得对象引用
 * @see com_pushlog("key", "value")("key2", "val%d", 2)(COMLOG_NOTICE, "end");
 * 	以(日志等级, 数据) 结束打印
 **/
#define com_pushlog(key, fmt, arg...) com_getcategory()->push(key, fmt, ##arg)

/**
 * @brief 将配置文件得路径和文件名告诉库，库会分析这个配置格式，
 * 	再条件触发下，改变日志等级
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @return  int 成功返回0 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:02:48
**/
int com_reglogstat(const char *path, const char *file);

/**
 * @brief 调用这个函数，将改变日志打印等级
 *
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:03:33
**/
int com_changelogstat();

/**
 * @brief 调用这个函数，将根据设置得logstat信息，改变日志等级
 *
 * @param [in/out] logstat   : com_logstat_t*
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:03:51
**/
int com_changelogstat(com_logstat_t *logstat);

/**
 * @brief 判断当前打开的是哪一个log
 *
 * @return  int 
 * 			USING_COMLOG : 使用comlog
 * 			USING_ULLOG  : 使用ullog
 * 			LOG_NOT_DEFINED : 未显式调用任何openlog/loadlog
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/12/16 01:36:18
**/
int com_logstatus();

/**
 * @brief 动态添加新的appender。用于二次开发。
 * 用户可以继承Appender类，实现一个自己的Appender，并注册相应的get/try函数
 * name表示自定义appender的名称，可以在配置文件中被识别
 *
 * @param [in] name   : const char* 自定义appender的名称
 * @param [in] getAppender   : func_type getAppender函数
 * @param [in] tryAppender   : func_type tryAppender函数
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/09/10 17:24:14
**/
typedef comspace::Appender*(*func_type)(com_device_t &);
int com_registappender(const char * name, func_type getAppender, func_type tryAppender);

/**
 * @brief log enabled
 *
 * @param [in] event
 * @return  int 
 * 1 - enable
 * 0 - disable
 * -1 - error
 * @author wangshaoyan
 * @date 2012/12/13
**/
int com_log_enabled(int level);
int com_log_enabled(const char *selflevel);

//线程数据
//这些数据留给开发人员，不推荐使用
struct com_thread_data_t
{
	comspace::Category *category;
	comspace::Event *event;
};
comspace::Category * com_getcategory();
comspace::Event * com_getevent();
com_thread_data_t * com_getthreaddata();

#endif  //__LOG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */

