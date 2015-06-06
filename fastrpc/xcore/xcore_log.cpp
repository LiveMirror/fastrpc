// 2008-06-05
// xcore_log.cpp
// 
// 日志类（多线程安全）


#include "xcore_log.h"
#include "xcore_critical.h"
#include "xcore_thread.h"
#include "xcore_time.h"
#include "xcore_str_util.h"
#include "xcore_file_util.h"
#include "xcore_socket.h"
#include "xcore_memory.h"
#include <stdarg.h>

#ifdef __WINDOWS__
#define snprintf  _snprintf
#define vsnprintf _vsnprintf
#endif//__WINDOWS__

namespace xcore {

static const char *gs_caszPriorityString[XLOG_LEVEL_LIMIT] =
{
	"NONE ",
	"CRASH",
	"ERROR",
	"WARNI",
	"NOTIC",
	"INFO ",
	"DEBUG",
	"TRACE"
};

XLogParam XLogParam::Default;

////////////////////////////////////////////////////////////////////////////////
// XLog_
////////////////////////////////////////////////////////////////////////////////
// 定义日志类
class XLog_ : public XThread
{
public:
	enum { max_line_each_file = (32 * 1024),
           max_message_length = 1024, 
	       max_malloc_buffer = (32 * 1024)
	     };
public:
	XLog_();

	virtual ~XLog_();

	// 初始化
	bool initialize(const XLogParam& logParam);
	bool uninitialize(void);

	// 设置和获取日志输出级别
	uint32 level() const { return m_param.m_level; }
	void   level(uint32 lev) { if (lev >= XLOG_LEVEL_LIMIT) lev = XLOG_TRACE; m_param.m_level = (XLOG_LEVEL)lev;}
	
	// 设置和获取日志输出指向
	uint32 options() const { return m_param.m_options; }
	void   options(uint32 opt) { m_param.m_options = (opt & 0X0F); }
	
	// 设置和获取日志保存天数
	uint32 keep_days() const { return m_param.m_keep_days; }
	void   keep_days(uint32 days) { if (days > 100) days = 100; m_param.m_keep_days = days; _remove_overdue_file(); }

	// 设置和获取TCP日志流监听端口(0表示禁用)
	uint16 tcp_port() const { return m_param.m_tcp_port; }
	void   tcp_port(uint16 port);

	// 输出
	void printf(XLOG_LEVEL lev, const char cszFormat[], va_list marker);

private:
	bool _init_acceptor();

	void _accept_tcp_stream();

	void _real_write(const char *pszBuffer, uint32 len);

	void _write_file(const char *pszBuffer, uint32 len);

	void _write_tcp_stream(const char *pszBuffer, uint32 len);

	void _change_file(void);

	void _remove_overdue_file();

	time_t _get_time_from_filename(const string& filename);

protected:
	virtual void run(XThread* pThread);

private:
	XLogParam           m_param;
	uint32              m_curr_line;
	string              m_curr_filename;
	FILE*               m_curr_fd;
	uint32				m_last_day;

	XCritical			m_lockUse;
	list<void *>		m_lstUseBufs;
	XEvent				m_evWork;

	XSocket				m_listensock;
	list<SOCKET>		m_lstsockets;

	bool                m_initok;
};

static XLog_ g_LogInstance;

XLog_::XLog_()
	: m_curr_line(0)
	, m_curr_fd(NULL)
	, m_last_day(0)
	, m_initok(false)
{
	// empty
}

XLog_::~XLog_()
{
	// empty
}

// 初始化
bool XLog_::initialize(const XLogParam& param)
{
	if (m_initok)
	{
		fprintf(stderr, "ERR: Log system re-init!\n");
		return true;
	}

	m_param = param;
	if (m_param.m_options == 0) m_param.m_options = XLOG_OPTION_STDOUT;
	if (m_param.m_level >= XLOG_LEVEL_LIMIT) m_param.m_level = XLOG_TRACE;
	if (m_param.m_max_line == 0 || m_param.m_max_line > max_line_each_file)
	{
		m_param.m_max_line = max_line_each_file;
	}
	else if (m_param.m_max_line < 1024)
	{
		m_param.m_max_line = 1024;
	}
	m_curr_line = m_param.m_max_line;  // 使第一次写日志时就创建文件夹
	if (m_param.m_keep_days > 100) m_param.m_keep_days = 100;
	if (m_param.m_options & XLOG_OPTION_FILE)
	{
		XStrUtil::chop(m_param.m_logdir, " \r\n\t");
		if (m_param.m_logdir.empty()) m_param.m_logdir = ".";
		char ch = m_param.m_logdir.at(0);
		XStrUtil::chop_tail(m_param.m_logdir, "\\/");
		if (m_param.m_logdir.empty() && ch == '/') m_param.m_logdir = "/";
		if (m_param.m_logdir.empty() && ch == '\\') m_param.m_logdir = "\\\\";
		XStrUtil::chop(m_param.m_ident, " _\r\n\t\\/");
	}
	if ((m_param.m_options & XLOG_OPTION_TCP) && (m_param.m_tcp_port != 0) && !_init_acceptor())
	{
		fprintf(stderr, "ERR: Log system init tcp listen(port:%d) failed!\n", m_param.m_tcp_port);
		return false;
	}

	if (!start())
	{
		m_listensock.close();
		fprintf(stderr, "ERR: Log system thread start failed!\n");
		return false;
	}
	
	m_initok = true;
	XNOTICE("Log system startup OK!");
	return true;
}

bool XLog_::uninitialize()
{
	if (!m_initok) return true;

	// wait until write all message to file
	volatile uint32 count = 1;
	while (count != 0)
	{
		m_lockUse.lock();
		count = (uint32)m_lstUseBufs.size();
		m_lockUse.unlock();
		if (count == 0) break;
		m_evWork.set();
		xcore::sleep(100);
	}

	// stop thread
	this->stop();
	m_evWork.set();
	this->join();

	// close file
	if (m_curr_fd)
	{
		fclose(m_curr_fd);
		m_curr_fd= NULL;
	}

	// close socket stream
	m_listensock.close();
	while (m_lstsockets.size() > 0)
	{
		XSocket sock(m_lstsockets.front());
		m_lstsockets.pop_front();
		sock.abort();
	}

	m_initok = false;
	return true;
}

void XLog_::tcp_port(uint16 port)
{
	if (port == 0)
	{
		m_param.m_options &= ~XLOG_OPTION_TCP;
		m_param.m_tcp_port = 0;
		m_listensock.close();
		while (m_lstsockets.size() > 0)
		{
			XSocket sock(m_lstsockets.front());
			m_lstsockets.pop_front();
			sock.abort();
		}
		XNOTICE("Log system TCP port close.");
		return;
	}
	else
	{
		if (m_param.m_tcp_port == port) return;
		m_param.m_tcp_port = port;
		m_param.m_options &= ~XLOG_OPTION_TCP;
		while (m_lstsockets.size() > 0)
		{
			XSocket sock(m_lstsockets.front());
			m_lstsockets.pop_front();
			sock.abort();
		}
		if (!_init_acceptor())
		{
			m_param.m_tcp_port = 0;
			XERROR("Log system TCP at port(%d) init acceptor failed(errno:%d).", port, errno);
			return;
		}
		m_param.m_options |= XLOG_OPTION_TCP;
		XNOTICE("Log system TCP port(%d) open.", port);
		return;
	}
	return;
}

// 输出
void XLog_::printf(XLOG_LEVEL lev, const char cszFormat[], va_list marker)
{
	int nRetCode = -1;
	XTime now;
	uint32 nStrLen = 0;
	uint32 nBufferLen  = 0;
	char *pszBuffer = NULL;
	char *pvData = NULL;

	if (cszFormat == NULL) return;
	if (lev == XLOG_NONE) return;
	if (lev > m_param.m_level) return;

	pvData = (char *)xcore::memory_alloc(max_message_length);
	pszBuffer = (char *)pvData + sizeof(uint32);
	nBufferLen = (uint32)(max_message_length - sizeof(uint32));
	nRetCode = snprintf(pszBuffer, nBufferLen - 1,
						"%04d%02d%02d-%02d%02d%02d<%s:%u>: ",
						now.local_year(),
						now.local_mon(),
						now.local_mday(),
						now.local_hour(),
						now.local_min(),
						now.local_sec(),
						gs_caszPriorityString[lev],
						xcore::thread_id()
						);
	if (nRetCode <= 0)
	{
		xcore::memory_free(pvData);
		return;
	}
	nStrLen = nRetCode;

	nRetCode = vsnprintf(
		pszBuffer + nStrLen, 
		nBufferLen - nStrLen - 1,
		cszFormat, marker
		);
	if (nRetCode < 0 || nRetCode >= (int)(nBufferLen - nStrLen - 1))
		nStrLen = nBufferLen - 1;	// windows下在字符串超过长度时,会返回-1. linux下返回值可能大于缓冲区大小,但是缓冲区不会被溢出.
	else
		nStrLen += nRetCode;

	while (pszBuffer[nStrLen - 1] == '\r' || pszBuffer[nStrLen - 1] == '\n')
	{
		nStrLen--;
	}
	if (nStrLen >= nBufferLen - 2)	// if full
	{
		nStrLen = nBufferLen - 3;
	}
	pszBuffer[nStrLen] = '\r';
	pszBuffer[nStrLen + 1] = '\n';
	pszBuffer[nStrLen + 2] = '\0';
	nStrLen += 2;

	#ifdef __DEBUG__
	if (m_param.m_options & XLOG_OPTION_STDOUT)
	{
		fputs(pszBuffer, stdout);
		#ifdef __WINDOWS__
		::OutputDebugString(pszBuffer);
		#endif//__WINDOWS__
	}
	if (m_param.m_options & XLOG_OPTION_STDERR)
	{
		fputs(pszBuffer, stderr);
		#ifdef __WINDOWS__
		if (!(m_param.m_options & XLOG_OPTION_STDOUT)) ::OutputDebugString(pszBuffer);
		#endif//__WINDOWS__
	}
	if ((m_param.m_options & ~(XLOG_OPTION_STDOUT | XLOG_OPTION_STDERR)) == 0)
	{
		xcore::memory_free(pvData);
		return;
	}
	#endif//__DEBUG__

	// 使用额外的线程写
	XLockGuard<XCritical> lock(m_lockUse);
	*((uint32 *)pvData) = (uint32)nStrLen;
	m_lstUseBufs.push_back(pvData);
	m_evWork.set();	// 置事件

	return;
}

void XLog_::run(XThread* pThread)
{
	void *pvData = NULL;
	char *pszBuffer = NULL;
	uint32 len = 0;
	if (pThread == NULL) return;

	while (!pThread->wait_quit(0))
	{
		pvData = NULL;
		m_lockUse.lock();
		if (m_lstUseBufs.size() > 0)
		{
			pvData = m_lstUseBufs.front();
			m_lstUseBufs.pop_front();
		}
		m_lockUse.unlock();
		if (pvData == NULL)
		{
			// 空闲时接受TCP连接
			if (m_param.m_options & XLOG_OPTION_TCP)
			{
				_accept_tcp_stream();
			}

			m_evWork.trywait(500);
			continue;
		}

		pszBuffer = (char *)pvData + sizeof(uint32);
		len = *(uint32 *)pvData;

		_real_write(pszBuffer, len);

		xcore::memory_free(pvData);
	}

	return;
}

bool XLog_::_init_acceptor()
{
	if (m_param.m_tcp_port == 0) return false;

	m_listensock.close();
	if (!m_listensock.open(SOCK_STREAM))
	{
		XERROR("Log system TCP port(%d) open failed(errno:%d).", m_param.m_tcp_port, errno);
		return false;
	}
	m_listensock.set_reuse_addr();
	if (!m_listensock.listen(XSockAddr("", m_param.m_tcp_port)))
	{
		m_listensock.close();
		XERROR("Log system TCP port(%d) Listen failed(errno:%d).", m_param.m_tcp_port, errno);
		return false;
	}

	m_listensock.set_nonblock();
	return true;
}

void XLog_::_accept_tcp_stream()
{
	int ret = 0;
	XSocket sock;
	char buf[128];
	list<SOCKET>::iterator it = m_lstsockets.begin();
	while (it != m_lstsockets.end())
	{
		sock.attach(*it);
		while ((ret = sock.recv(buf, 128)) > 0);
		if (ret == -1)
		{
			sock.close();
			it = m_lstsockets.erase(it);
			continue;
		}
		sock.detach();
		++it;
	}
	
	if (!m_listensock.accept(sock))
	{
		_init_acceptor();
		return;
	}
	if (sock.get_handle() == XCORE_INVALID_SOCKET)
	{
		return;
	}
	sock.set_nonblock();

	XNOTICE("Log system tcp accept a socket(%s).", sock.remote_addr().to_str().c_str());

	if (m_lstsockets.size() >= 5) // 最多允许五个连接
	{
		sock.send("Server busy!!!\r\n", 16);
		XNOTICE("Log system tcp connection count > 5, new socket disconnect.");
		sock.close();
		return;
	}

	m_lstsockets.push_back(sock.detach());
	return;
}

void XLog_::_real_write(const char *pszBuffer, uint32 len)
{
	#ifndef __DEBUG__
	if (m_param.m_options & XLOG_OPTION_STDOUT)
	{
		fputs(pszBuffer, stdout);
	}
	if (m_param.m_options & XLOG_OPTION_STDERR)
	{
		fputs(pszBuffer, stderr);
	}
	#endif//__DEBUG__

	if (m_param.m_options & XLOG_OPTION_FILE)
	{
		_write_file(pszBuffer, len);
	}
	if (m_param.m_options & XLOG_OPTION_TCP)
	{
		_write_tcp_stream(pszBuffer, len);
	}
	return;
}

void XLog_::_write_file(const char *pszBuffer, uint32 len)
{
	XTime now;
	if (m_curr_line >= max_line_each_file || 
		m_curr_fd == NULL ||
		now.to_date() != m_last_day)
	{
		_change_file();
		m_curr_line = 0;
		m_last_day = now.to_date();
	}
	
	int ret = (int)fwrite(pszBuffer, 1, (int)len, m_curr_fd);
	if (ret != (int)len)
	{
		fprintf(stderr, "Log system write file(%s) failed, errno:%d(%s)!\n", m_curr_filename.c_str(), errno, strerror(errno));
		return;
	}
	m_curr_line++;
	return;
}

void XLog_::_write_tcp_stream(const char *pszBuffer, uint32 len)
{
	list<SOCKET>::iterator it, it_end;
	it = m_lstsockets.begin();
	it_end = m_lstsockets.end();
	while (it != it_end)
	{
		XSocket sock(*it);
		if (-1 == sock.send(pszBuffer, len))
		{
			it = m_lstsockets.erase(it);

			XNOTICE("Log system tcp socket disconnect(%s).", sock.remote_addr().to_str().c_str());
			sock.close();
		}
		else
		{
			sock.detach();
			++it;
		}
	}
	return;
}

void XLog_::_change_file()
{
	XTime now;
	char path[1024] = {};

	_remove_overdue_file();

	XFileUtil::make_dir_p(m_param.m_logdir);
	if (m_curr_fd)
	{
		fclose(m_curr_fd);
		m_curr_fd = NULL;
	}

	if (m_param.m_ident.empty())
	{
		snprintf(path, 1023, "%04d%02d%02d_%02d%02d%02d.log", 
			now.local_year(),
			now.local_mon(),
			now.local_mday(),
			now.local_hour(),
			now.local_min(),
			now.local_sec());
	}
	else
	{
		snprintf(path, 1023, "%s_%04d%02d%02d_%02d%02d%02d.log",
			m_param.m_ident.c_str(),
			now.local_year(),
			now.local_mon(),
			now.local_mday(),
			now.local_hour(),
			now.local_min(),
			now.local_sec());
	}
	m_curr_filename = path;
	snprintf(path, 1023, "%s%c%s", m_param.m_logdir.c_str(), XFileUtil::PATH_SEPARATOR, m_curr_filename.c_str());
	m_curr_fd = fopen(path, "at+");
	if (m_curr_fd == NULL)
	{
		fprintf(stderr, "Log system open file(%s) failed, errno:%d(%s)!\n", path, errno, strerror(errno));
		return;
	}
	
	setvbuf(m_curr_fd, NULL, _IONBF, 0);
	return;
}

void XLog_::_remove_overdue_file()
{
	time_t last = time(NULL) - 3600 * 24 * m_param.m_keep_days;
	XFileFinder finder;
	string strName;
	string strDirName;
	string strFileName;
	int type;

	if (!finder.open(m_param.m_logdir)) return;
	while ((type = finder.next(strName)) != XFileFinder::TYPE_NONE)
	{
		if (strName == m_curr_filename) continue;
		time_t t = _get_time_from_filename(strName);
		if (t >= last) continue;
		string path = m_param.m_logdir + XFileUtil::PATH_SEPARATOR+ strName;
		XFileUtil::remove_file(path);
	}
	finder.close();
	return;
}

time_t XLog_::_get_time_from_filename(const string& filename)
{
	string datetime;
	vector<string> vItems;
	XStrUtil::split(filename, vItems, "_.");
	if (vItems.size() == 3)
	{
		datetime = vItems[0] + vItems[1];
		if(vItems[2] != "log") return time(NULL);
		if (!m_param.m_ident.empty()) return time(NULL);
	}
	else if (vItems.size() == 4)
	{
		datetime = vItems[1] + vItems[2];
		if (vItems[3] != "log") return time(NULL);
		if (m_param.m_ident != vItems[0]) return time(NULL);
	}
	else
	{
		return time(NULL);
	}
	XTime t = XTime::try_parse(datetime);
	if (t.has_error()) return time(NULL);
	return t.sec();
}

//////////////////////////////////////////////////////////////////////

bool XLog::initialize(const XLogParam& param)
{
	return g_LogInstance.initialize(param);
}

void XLog::uninitialize()
{
	g_LogInstance.uninitialize();
}

void XLog::set_level(uint32 lev)
{
	return g_LogInstance.level(lev);
}

uint32 XLog::get_level()
{
	return g_LogInstance.level();
}

void XLog::set_options(uint32 opt)
{
	return g_LogInstance.options(opt);
}

uint32 XLog::get_options()
{
	return g_LogInstance.options();
}

void XLog::set_keep_days(uint32 days)
{
	return g_LogInstance.keep_days(days);
}

uint32 XLog::get_keep_days()
{
	return g_LogInstance.keep_days();
}

void XLog::set_tcp_port(uint16 port)
{
	return g_LogInstance.tcp_port(port);
}

uint16 XLog::get_tcp_port()
{
	return g_LogInstance.tcp_port();
}

void XLog::printf(XLOG_LEVEL lev, const char cszFormat[], ...)
{
	va_list marker;
	va_start(marker, cszFormat);
	g_LogInstance.printf(lev, cszFormat, marker);
	va_end(marker);
	return;
}

} // namespace xcore



////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_log()
{
	XDEBUG("not init log system now.");

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
