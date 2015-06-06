// 2008-06-05
// xcore_log.h
// 
// 日志类（多线程安全）


#ifndef _XCORE_LOG_H_
#define _XCORE_LOG_H_

#include "xcore_define.h"
#include "xcore_clock.h"

namespace xcore {

// 日志输出级别
enum XLOG_LEVEL
{
	XLOG_NONE		=	0,	// 不打印日志
	XLOG_CRASH		=   1,  // 严重错误,导致程序不能继续运行
	XLOG_ERR		=   2,  // 程序错误,导致处理失败(如申请内存失败等)
	XLOG_WARNING	=	3,	// 程序警告,可能导致错误产生(如传入不合法参数)
	XLOG_NOTICE		=   4,  // 正常但是值得注意的情况
	XLOG_INFO	    =   5,  // 业务相关的信息(不影响程序流程，如显示用户登陆下线信息)
	XLOG_DEBUG	    =   6,  // 调试信息(不影响程序流程，如打印当前内存池中未使用内存块数目等)
	XLOG_TRACE		=	7,	// 打印程序运行轨迹
	XLOG_LEVEL_LIMIT
};

// 日志输出位置
enum XLOG_OPTIONS
{
	XLOG_OPTION_FILE		=   0x01,   // log on to file, default
	XLOG_OPTION_STDOUT		=   0x02,   // log on the console if errors in sending
	XLOG_OPTION_STDERR		=   0x04,   // log on the stderr stream
	XLOG_OPTION_TCP			=   0x08,	// log on the tcp stream
};

class XLogParam
{
public:
	uint32			m_options;			// 指定输出位置, see XLOG_OPTIONS
	XLOG_LEVEL      m_level;			// 指定输出级别, see XLOG_PRIORITY
	string			m_logdir;			// 日志文件总路径
	string			m_ident;			// 日志分类名称(如多个程序日志输出在同一文件夹下时)
	uint32			m_max_line;         // 每个日志文件最大容纳行数，多出时创建新日志文件
	uint32			m_keep_days;		// 日志文件保存天数
	uint32			m_tcp_port;		    // 输出tcp流时监听端口(0表示禁用)
	
	XLogParam()
		: m_options(XLOG_OPTION_STDOUT)
		, m_level(XLOG_DEBUG)
		, m_logdir("log")
		, m_max_line(0)
		, m_keep_days(30)
		, m_tcp_port(0)
	{
		// empty
	}

	static XLogParam Default;
};

class XLog
{
public:
	// 日志系统初始化
	static bool   initialize(const XLogParam& param = XLogParam::Default);
	static void   uninitialize();

	// 设置和获取日志输出级别
	static void   set_level(uint32 lev/*see XLogLevel*/);
	static uint32 get_level();

	// 设置和获取日志输出指向
	static void   set_options(uint32 opt/*see XLogOption*/);
	static uint32 get_options();

	// 设置和获取日志保存天数
	static void   set_keep_days(uint32 days);
	static uint32 get_keep_days();

	// 设置和获取TCP日志流监听端口(0表示禁用)
	static void   set_tcp_port(uint16 port);
	static uint16 get_tcp_port();

	// 输出
	static void   printf(XLOG_LEVEL lev/*see XLogLevel*/, const char cszFormat[], ...);
};

#define XCRASH(format, ...)    XLog::printf(XLOG_CRASH, (const char*)format, ##__VA_ARGS__)
#define XERROR(format, ...)    XLog::printf(XLOG_ERR, (const char*)format, ##__VA_ARGS__)
#define XWARNING(format, ...)  XLog::printf(XLOG_WARNING, (const char*)format, ##__VA_ARGS__)
#define XNOTICE(format, ...)   XLog::printf(XLOG_NOTICE, (const char*)format, ##__VA_ARGS__)
#define XINFO(format, ...)     XLog::printf(XLOG_INFO, (const char*)format, ##__VA_ARGS__)
#define XDEBUG(format, ...)    XLog::printf(XLOG_DEBUG, (const char*)format, ##__VA_ARGS__)

#ifdef __DEBUG__
class XLogTrace
{
public:
	XLogTrace(const char *msg, int line)
		: m_msg(msg)
		, m_line(line)
	{
		XLog::printf(XLOG_TRACE, "=> Entry %s():%d", m_msg, m_line);
	}

	~XLogTrace()
	{
		XLog::printf(XLOG_TRACE, "<= Leave %s():%d, Elapsed %llu usecs.", m_msg, m_line, m_clock.peek().to_usecs());
	}
private:
	XClock      m_clock;
	const char* m_msg;
	int         m_line;
};
#define XTRACE_FUNCTION() XLogTrace tmplogtrace_(__FUNCTION__, __LINE__)
#define XTRACE(format, ...) XLog::printf(XLOG_TRACE, (const char*)format, ##__VA_ARGS__)
#else //__DEBUG__
#define XTRACE_FUNCTION()
#define XTRACE(format, ...)
#endif //__DEBUG__


#define XLOG_PROCESS_ERROR(Condition) \
	do  \
	{   \
		if (!(Condition))       \
		{                       \
			XERROR("XLOG_PROCESS_ERROR(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
			goto Exit0;         \
		}                       \
	} while (false)

#define XLOG_PROCESS_SUCCESS(Condition) \
	do  \
	{   \
		if (Condition)          \
		{                       \
			XNOTICE("XLOG_PROCESS_SUCCESS(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
			goto Exit1;         \
		}                       \
	} while (false)

#define XLOG_CHECK_ERROR(Condition) \
	do  \
	{   \
		if (!(Condition))       \
		{                       \
			XNOTICE("XLOG_CHECK_ERROR(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
		}                       \
	} while (false)

#define XLOG_PROCESS_ERROR_RET(Condition, Code) \
	do  \
	{   \
		if (!(Condition))       \
		{                       \
			XERROR("XLOG_PROCESS_ERROR_RET_CODE(%s, %d) at %s in %s:%d.", #Condition, Code, __FUNCTION__, __FILE__, __LINE__); \
			nResult = Code;     \
			goto Exit0;         \
		}                       \
	} while (false)

#define XLOG_COM_PROCESS_ERROR(Condition) \
	do  \
	{   \
		if (FAILED(Condition))  \
		{                       \
			XERROR("XLOG_COM_PROCESS_ERROR(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
			goto Exit0;         \
		}                       \
	} while (false)


#define XLOG_COM_PROCESS_SUCCESS(Condition)   \
	do  \
	{   \
		if (SUCCEEDED(Condition))   \
		{                           \
			XNOTICE("XLOG_COM_PROCESS_SUCCESS(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
			goto Exit1;             \
		}                           \
	} while (false)


#define XLOG_COM_PROCESS_ERROR_RET(Condition, Code)     \
	do  \
	{   \
		if (FAILED(Condition))      \
		{                           \
			XERROR("XLOG_COM_PROC_ERROR_RET_CODE(%s, 0X%X) at %s in %s:%d.", #Condition, Code, __FUNCTION__, __FILE__, __LINE__); \
			hrResult = Code;        \
			goto Exit0;             \
		}                           \
	} while (false)

#define XLOG_COM_CHECK_ERROR(Condition) \
	do  \
	{   \
		if (FAILED(Condition))       \
		{                       \
			XNOTICE("XLOG_COM_CHECK_ERROR(%s) at %s in %s:%d.", #Condition, __FUNCTION__, __FILE__, __LINE__); \
		}                       \
	} while (false)

} // namespace xcore

using namespace xcore;

#endif//_XCORE_LOG_H_
