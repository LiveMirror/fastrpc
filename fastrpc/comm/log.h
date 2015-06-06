#ifndef __C_LOG_H
#define __C_LOG_H

/*
功能: 
       1.  Log类

Created by Song, 2003-01
Change list:

*/


#include <stdio.h>

class CLog
{
public:
	CLog(const char *log_base_path,
		int max_log_size,
		int max_log_num = 10,
		int * log_level_ptr = NULL);
	
	virtual ~CLog();

public:
	int open();
	int close();
	const char * get_error() const { return _error_text; }

public:
	enum CLogLevel
	{
		NO_LOG = 1,  // 所有的log都不写
		ERROR_LOG = 2,  // 只写错误log
		NORMAL_LOG = 3,  // 写普通log和错误log
		DEBUG_LOG = 4  // 写普通log、错误log和调试log
	};

	// 取得log的级别
	inline int get_log_level() const 
	{return _log_level_ptr != NULL ? (*_log_level_ptr):DEBUG_LOG;}

	inline void set_log_level_ptr(int * log_level_ptr)
	{_log_level_ptr = log_level_ptr;}

	static char * get_log_level_str(int log_level, char * str);
	static bool log_level_is_valid(int log_level);

public:
	// 多进程时显示进程编号
	void set_process_no(int proc_no) { _proc_no = proc_no; }
	// 多进程时显示进程编号
	int get_process_no() { return _proc_no; }

	// 错误log，前面带 ERROR:
	int error_log(const char *fmt, ...);

	// 写普通log
	int write_log(const char *fmt, ...);

	// 调试log，前面带DEBUG:
	int debug_log(const char *fmt, ...);

	// 以16进制形式打印buf
	int print_hex(const void *buf, size_t len);

	// 不建议使用
	int puts(const char * str);

	// 每天一个的log
	int day_log(const char * day_log_path, const char *fmt, ...);

	// 和day_log类似，但前面不写时间
	int day_log_raw(const char * day_log_path, const char *fmt, ...);

private:
	
	int shilft_files();
	
protected:
	static const int LOG_SHIFT_COUNT  = 32;

	FILE * _fp;

	char _log_base_path[256];
	int _max_log_size;
	int _max_log_num;

	// log的级别指针，通常指向共享内存
	// 当这个指针为NULL时，取默认NORMAL_LOG
	volatile int * _log_level_ptr;

	char _log_file_name[256];
	
	int _write_count;
	int _proc_no;
	
	char _log_buf[16384];
	char _error_text[256];
};

extern class CLog * g_pLog;

#ifdef DEBUG_LOG_ARGS
    #define trpc_write_log printf
    #define trpc_error_log printf
    #define trpc_debug_log printf
#else
    #define trpc_write_log g_pLog->write_log
    #define trpc_error_log g_pLog->error_log
    #define trpc_debug_log g_pLog->debug_log
#endif

#define trpc_day_log g_pLog->day_log

#define trpc_set_log_level_ptr g_pLog->set_log_level_ptr

#endif
