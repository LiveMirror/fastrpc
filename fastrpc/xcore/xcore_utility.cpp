// 2011-08-27
// xcore_utility.cpp
// 
// 实用工具

#include "xcore_utility.h"
#include "xcore_critical.h"
#include "xcore_str_util.h"
#include "xcore_file_util.h"

#ifdef __WINDOWS__
#include <conio.h>
#endif//__WINDOWS__

#ifdef __GNUC__
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/utsname.h>
#include <termios.h>
//#include <stropts.h>
#endif//__GNUC__

namespace xcore {
	
#ifdef __WINDOWS__

uint32 errnum()
{
	return ::GetLastError();
}

string strerror()
{
	char buf[512] = {};
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 511, NULL);
	return buf;
}

int kbhit()
{
	return _kbhit();
}

string computer_name()
{
	static string name_;

	if (name_.empty())
	{
		char buf[MAX_COMPUTERNAME_LENGTH + 1] = {};
		DWORD lenth = MAX_COMPUTERNAME_LENGTH;
		XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
		if (::GetComputerName(buf, &lenth))
		{
			name_ = buf;
		}
	}
	return name_;
}

string user_name()
{
	static string name_;

	if (name_.empty())
	{
		char buf[257] = {};
		DWORD lenth = 256;
		XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
		if (::GetUserName(buf, &lenth))
		{
			name_ = buf;
		}
	}
	return name_;
}

string os_version()
{
	return "Windows";
}

bool meminfo(uint32& total_KB, uint32& use_KB, uint32& free_KB)
{
	MEMORYSTATUS stat; 
	GlobalMemoryStatus(&stat); 
	total_KB = stat.dwTotalPhys / 1024;
	free_KB = stat.dwAvailPhys / 1024;
	use_KB = total_KB > free_KB ? total_KB - free_KB : 0;
	return true;
}

bool cpuinfo(vector<string>& infos)
{
	infos.clear();
	infos.push_back("unknown");
	return true;
}

uint32 processnum()
{
	return 0;
}

#endif//__WINDOWS__

#ifdef __GNUC__

uint32 errnum()
{
	return (uint32)errno;
}

string strerror()
{
	return ::strerror(errno);
}

int kbhit()
{
	static const int STDIN = 0;
	static bool initialized = false;
	if (!initialized)
	{
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}
	int bytesWaiting = 0;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

int kbhit2()
{
	int            nResult         = false;
	int            nRetCode        = false;
	int            nGetOldAttrFlag = false;
	int            nStdinFD        = -1;
	int            nTtyFlag        = false;      
	struct termios OldValue;
	struct termios NewValue;
	timeval        Timeout         = {0, 0};
	fd_set         FDSet;

	nStdinFD = fileno(stdin);
	if (nStdinFD < 0)
		goto Exit0;

	// stdin maybe redirect to socket
	nTtyFlag = isatty(nStdinFD);
	if (nTtyFlag)
	{
		nRetCode = tcgetattr(nStdinFD, &OldValue);
		if (nRetCode)
		{
			// not a tty 
			goto Exit0;
		}
		nGetOldAttrFlag = true;

		NewValue = OldValue;

		/* Disable canonical mode, and set buffer size to 1 byte */
		NewValue.c_lflag     &= (~ICANON);
		NewValue.c_cc[VTIME]  = 0;
		NewValue.c_cc[VMIN]   = 1;

		tcsetattr(nStdinFD, TCSANOW, &NewValue);
	}

	// select or poll
	FD_ZERO(&FDSet);
	FD_SET(nStdinFD, &FDSet);

	nRetCode = select(nStdinFD + 1, &FDSet, NULL, NULL, &Timeout);
	if (nRetCode != 1)
		goto Exit0;

	nResult = true;
Exit0:
	if (nTtyFlag && nGetOldAttrFlag)
	{
		tcsetattr(nStdinFD, TCSANOW, &OldValue);
		nGetOldAttrFlag = false;
	}
	return nResult;
}

string computer_name()
{
	return "unknown";
}

string user_name()
{
	return "unknown";
}

string os_version()
{
	struct utsname un;
	int ret = uname(&un);
	if (ret != 0)
	{
		return "Linux";
	}
	
	string result;
	result += un.sysname;
	result += " ";
	result += un.nodename;
	result += " ";
	result += un.release;
	result += " ";
	result += un.machine;
	return result;
}

bool meminfo(uint32& total_KB, uint32& use_KB, uint32& free_KB)
{
	char buf[10240] = {};
	FILE * fd = fopen("/proc/meminfo", "rt");
	if (fd == NULL) return false;
	int ret = fread(buf, 1, 10239, fd);
	fclose(fd);
	if (ret <= 0) return false;

	string src = buf;
	XStrUtil::to_lower(src);

	vector<string> lines;
	XStrUtil::split(src, lines, "\r\n");
	for (int i = 0; i < (int)lines.size(); i++)
	{
		vector<string> vItems;
		if (XStrUtil::split(lines[i], vItems, " \t:") < 2) continue;
		if (vItems[0] == "memtotal")
		{
			total_KB = XStrUtil::try_to_uint_def(vItems[1], 0);
		}
		else if (vItems[0] == "memfree")
		{
			free_KB = XStrUtil::try_to_uint_def(vItems[1], 0);
		}
		if (total_KB != 0 && free_KB != 0)
		{
			if (total_KB < free_KB) return false;
			use_KB = total_KB - free_KB;
			return true;
		}
	}
	return false;
}

bool cpuinfo(vector<string>& infos)
{
	infos.clear();
	char buf[1024];
	FILE* fd = fopen("/proc/cpuinfo", "rt");
	if (fd == NULL) return false;
	while (fgets(buf, 1023, fd))
	{
		vector<string> vItems;
		if (XStrUtil::split(buf, vItems, ":\r\n") < 2) continue;
		XStrUtil::chop(vItems[0]);
		if (vItems[0] == "model name")
		{
			XStrUtil::chop(vItems[1]);
			infos.push_back(vItems[1]);
		}
	}
	fclose(fd);
	return true;
}

uint32 processnum()
{
	string name;
	uint32 uValue = 0;
	uint32 result = 0;
	XFileFinder finder;
	if (!finder.open("/proc/", XFileFinder::TYPE_DIR | XFileFinder::ATTR_ALL)) return 0;
	while (XFileFinder::TYPE_NONE != finder.next(name))
	{
		if (XStrUtil::to_uint(name, uValue)) result++;
	}
	return result;
}


#endif//__GNUC__

}//namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_utility()
{
	printf("computer name:%s, user name:%s\n", xcore::computer_name().c_str(), xcore::user_name().c_str());
	printf("errnum:%d, strerror:%s\n", xcore::errnum(), xcore::strerror().c_str());
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
