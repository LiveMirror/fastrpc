// 20010-09-29
// xcore_sock_addr.h
// 
// Sock地址类

//#pragma comment( lib, "ws2_32" )

#include "xcore_sock_addr.h"
#include "xcore_critical.h"
#include "xcore_str_util.h"
#include "xcore_byte_order.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class __XSockGuarder
////////////////////////////////////////////////////////////////////////////////
class __XSockGuarder
{
public:
	__XSockGuarder()
	{
		#ifdef __WINDOWS__
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int nRetCode = WSAStartup(wVersionRequested, &wsaData);
		ASSERT((!nRetCode) && "WSAStartup failed!");
		#endif//__WINDOWS__

		#ifdef __GNUC__
		signal(SIGPIPE, SIG_IGN);
		#endif//__GNUC__
	}

	~__XSockGuarder()
	{
		#ifdef __WINDOWS__
		WSACleanup();
		#endif//__WINDOWS__
	}
};
static __XSockGuarder  __g_sock_guarder;


////////////////////////////////////////////////////////////////////////////////
// class XSockAddr
////////////////////////////////////////////////////////////////////////////////

const XSockAddr XSockAddr::AnyAddr;

const XSockAddr XSockAddr::NoneAddr(INADDR_NONE, 0);

XSockAddr::XSockAddr(void)
{
	reset();
}

XSockAddr::XSockAddr(const XSockAddr& addr)
{
	memcpy(this, &addr, sizeof(addr));
}

XSockAddr::XSockAddr(const sockaddr_in& addr)
{
	reset();
	m_inaddr.sin_addr.s_addr = addr.sin_addr.s_addr;
	m_inaddr.sin_port = addr.sin_port;
}

XSockAddr::XSockAddr(const sockaddr& addr)
{
	reset();
	const sockaddr_in& inaddr = (const sockaddr_in&)addr;
	m_inaddr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
	m_inaddr.sin_port = inaddr.sin_port;
}

XSockAddr::XSockAddr(const string& addr)
{
	reset();

	string host = addr;
	string port;
	size_t pos = addr.find(':');
	if (pos != string::npos)
	{
		host = addr.substr(0, addr.find(':'));
		port = addr.substr(addr.find(':') + 1);
	}
	
	set_host(host);
	set_port(XStrUtil::try_to_uint_def(port, 0));
}

XSockAddr::XSockAddr(const string& host, uint16 port)
{
	reset();
	set_host(host);
	set_port(port);
}

XSockAddr::XSockAddr(uint32 ip, uint16 port)
{
	reset();
	set_ipaddr(ip);
	set_port(port);
}

XSockAddr::~XSockAddr(void)
{
	reset();
}

XSockAddr& XSockAddr::operator = (const XSockAddr& addr)
{
	if (this != &addr)
	{
		memcpy(&m_inaddr, &addr.m_inaddr, sizeof(m_inaddr));
	}
	return *this;
}

XSockAddr& XSockAddr::operator = (const sockaddr_in& addr)
{
	m_inaddr.sin_addr.s_addr = addr.sin_addr.s_addr;
	m_inaddr.sin_port = addr.sin_port;
	return *this;
}

XSockAddr& XSockAddr::operator = (const sockaddr& addr)
{
	const sockaddr_in& inaddr = (const sockaddr_in&)addr;
	m_inaddr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
	m_inaddr.sin_port = inaddr.sin_port;
	return *this;
}

XSockAddr::operator const sockaddr_in *() const
{
	return &m_inaddr;
}

XSockAddr::operator const sockaddr *() const
{
	return (const sockaddr *)&m_inaddr;
}

XSockAddr::operator sockaddr_in() const
{
	return m_inaddr;
}

XSockAddr::operator sockaddr() const
{
	return *(sockaddr *)&m_inaddr;
}

void XSockAddr::set_port(uint16 port)
{
	m_inaddr.sin_port = XByteOrder::local2net(port);
	//m_inaddr.sin_port = htons(port);
}

void XSockAddr::set_ipaddr(uint32 ip)
{
	m_inaddr.sin_addr.s_addr = XByteOrder::local2net(ip);
	//m_inaddr.sin_addr.s_addr = htonl(ip);
}

bool XSockAddr::set_ipaddr(const string& ip)
{
	vector<string> vItems;
	if (4 != XStrUtil::split(ip, vItems, "\r\n\t ."))
	{
		return false;
	}

	uint32 tmp = 0;
	uint32 num = 0;
	for (int i = 0; i < 4; i++)
	{
		if (!XStrUtil::to_uint(vItems[i], tmp) || tmp >= 256)
		{
			return false;
		}
		num <<= 8;
		num |= tmp;
	}
	set_ipaddr(num);
	return true;
}

bool XSockAddr::set_host(const string& host)
{
	if (host.empty())
	{
		// ip set "0.0.0.0"
		m_inaddr.sin_addr.s_addr = INADDR_ANY;
		return true;
	}
	if (set_ipaddr(host)) return true;

	XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全

	struct hostent *pHost = gethostbyname(host.c_str());
	if (pHost && pHost->h_addr)
	{
		m_inaddr.sin_addr = *(in_addr *)pHost->h_addr;
		return true;
	}
	else
	{
		m_inaddr.sin_addr.s_addr = INADDR_NONE;
		return false;
	}
}

uint16 XSockAddr::get_port() const
{
	return XByteOrder::net2local((uint16)m_inaddr.sin_port);
	//return ntohs(m_inaddr.sin_port);
}

uint32 XSockAddr::get_ipaddr() const
{
	return XByteOrder::net2local((uint32)m_inaddr.sin_addr.s_addr);
	//return ntohl(m_inaddr.sin_addr.s_addr);
}

string XSockAddr::get_hostname() const
{
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
	
	if (this->m_inaddr.sin_addr.s_addr == INADDR_ANY)
	{
		return local_net_name();
	}

	struct hostent *pHost = gethostbyaddr((char *)&m_inaddr.sin_addr, 4, PF_INET);
	if (pHost && pHost->h_name)
	{
		return pHost->h_name;
	}
	else
	{
		return get_hostaddr();
	}
}

string XSockAddr::get_hostaddr() const
{
	char buf[32];
	uint32 ip_ = get_ipaddr();
	sprintf(buf, "%u.%u.%u.%u", (ip_ >> 24) & 0XFF, (ip_ >> 16) & 0XFF, (ip_ >> 8) & 0XFF, ip_ & 0XFF);
	return buf;
}

string XSockAddr::to_str() const
{
	char buf[32];
	uint32 ip_ = get_ipaddr();
	uint16 port_ = get_port();
	sprintf(buf, "%u.%u.%u.%u:%u", (ip_ >> 24) & 0XFF, (ip_ >> 16) & 0XFF, (ip_ >> 8) & 0XFF, ip_ & 0XFF, port_);
	return buf;
}

void XSockAddr::reset()
{
	memset(&m_inaddr, 0, sizeof(m_inaddr));
	m_inaddr.sin_family = PF_INET;
	return;
}

bool XSockAddr::is_any() const
{
	return (m_inaddr.sin_addr.s_addr == INADDR_ANY);
}

bool XSockAddr::is_none() const
{
	return (m_inaddr.sin_addr.s_addr == INADDR_NONE);
}

bool XSockAddr::is_loopback() const
{
	 return ((get_ipaddr() & 0XFF000001) == 0X7F000001);
}

bool XSockAddr::is_multicast() const
{
	uint32 ip = get_ipaddr();
	return (ip >= 0xE0000000) &&  // 224.0.0.0
		   (ip <= 0xEFFFFFFF); // 239.255.255.255
}

string XSockAddr::local_net_name()
{
	static string name_;

	if (name_.empty())
	{
		char buf[1024] = {};
		struct hostent *pHost = NULL;
		XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
		if (SOCKET_ERROR != gethostname(buf, 1023))
			pHost = gethostbyname(buf);
		else
			pHost = gethostbyname("");
		
		if (pHost && pHost->h_name)
		{
			name_ = pHost->h_name;
		}
	}
	return name_;
}

XSockAddr XSockAddr::local_mainaddr()
{
	static XSockAddr addr_;
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
	if (!addr_.is_any()) return addr_;

	#ifdef __WINDOWS__
	char buf[1024] = {};
	struct hostent *pHost = NULL;
	if (SOCKET_ERROR != gethostname(buf, 1023))
		pHost = gethostbyname(buf);
	else
		pHost = gethostbyname("");
	if (pHost && pHost->h_addr)
	{
		addr_.m_inaddr.sin_addr = *(in_addr *)pHost->h_addr;
	}
	return addr_;
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int             fd;
	struct ifreq    buf[16];
	struct ifconf   ifc;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0) return addr_;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
	{
		close(fd);
		return addr_;
	}

	int interface = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < interface; i++)
	{
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&buf[i]) == -1) continue;
		if (buf[i].ifr_flags & IFF_LOOPBACK) continue;
		if (!(buf[i].ifr_flags & IFF_UP)) continue;
		if (ioctl(fd, SIOCGIFADDR, (char *)&buf[i]) == 0)
		{
			addr_.m_inaddr.sin_addr = ((struct sockaddr_in *)(&buf[i].ifr_addr))->sin_addr;
			break;
		}
	}
	close(fd);
	return addr_;
	#endif//__GNUC__
}

bool XSockAddr::local_addrs(vector<XSockAddr>& addrs)
{
	static vector<XSockAddr> addrs_;
	addrs.clear();
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // 保证多线程安全
	if (addrs_.size() > 0)
	{
		addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
		return true;
	}

	#ifdef __WINDOWS__
	struct hostent *pHost = gethostbyname("");
	if (pHost == NULL) return false;
	for (int i = 0; i < 16; i++)
	{
		char* inaddr = pHost->h_addr_list[i];
		if (inaddr == NULL) break;

		XSockAddr addr;
		addr.m_inaddr.sin_addr = *(in_addr *)inaddr;
		if (addr.is_any()) continue;
		addrs_.push_back(addr);
	}

	addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
	return true;
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int             fd;
	struct ifreq    buf[16];
	struct ifconf   ifc;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0) return false;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
	{
		close(fd);
		return false;
	}

	int interface = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < interface; i++)
	{
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&buf[i]) == -1) continue;
		//if (buf[i].ifr_flags & IFF_LOOPBACK) continue;
		if (!(buf[i].ifr_flags & IFF_UP)) continue;
		if (ioctl(fd, SIOCGIFADDR, (char *)&buf[i]) == 0)
		{
			XSockAddr addr;
			addr.m_inaddr.sin_addr = ((struct sockaddr_in *)(&buf[i].ifr_addr))->sin_addr;
			if (addr.is_any()) continue;
			addrs_.push_back(addr);
		}
	}
	
	addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
	close(fd);
	return true;
	#endif//__GNUC__
}

bool operator < (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() < addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() < addr2.get_port())))
		return true;
	else
		return false;
}

bool operator <= (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() < addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() <= addr2.get_port())))
		return true;
	else
		return false;
}

bool operator >  (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() > addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() > addr2.get_port())))
		return true;
	else
		return false;
}

bool operator >= (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() > addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() >= addr2.get_port())))
		return true;
	else
		return false;
}

bool operator == (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() == addr2.get_ipaddr()) && 
		(addr1.get_port() == addr2.get_port()))
		return true;
	else
		return false;
}

bool operator != (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() != addr2.get_ipaddr()) ||
		(addr1.get_port() != addr2.get_port()))
		return true;
	else
		return false;
}


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

bool xcore_test_sock_addr()
{
	printf("start call xcore_test_sock_addr()...\n");
	printf("AnyAddr is: %s\n", XSockAddr::AnyAddr.to_str().c_str());
	printf("NoneAddr is: %s\n", XSockAddr::NoneAddr.to_str().c_str());
	XSockAddr addr1;
	printf("addr1 is: %s\n", addr1.to_str().c_str());
	XSockAddr addr2("202.118.66.6");
	printf("addr2 is: %s\n", addr2.to_str().c_str());
	XSockAddr addr3("202.118.66.6:80");
	printf("addr3 is: %s\n", addr3.to_str().c_str());
	XSockAddr addr4("www.baidu.com.cn");
	printf("addr4 is: %s\n", addr4.to_str().c_str());
	XSockAddr addr5("www.baidu.com.cn:80");
	printf("addr5 is: %s\n", addr5.to_str().c_str());

	sockaddr_in sockaddrin = addr3;
	sockaddr sockaddr = addr5;
	XSockAddr addr6 = sockaddrin;
	printf("addr6 is: %s\n", addr6.to_str().c_str());
	XSockAddr addr7 = sockaddr;
	printf("addr7 is: %s\n", addr7.to_str().c_str());
	XSockAddr addr8 = addr4;
	printf("addr8 is: %s\n", addr8.to_str().c_str());
	XSockAddr addr9;
	addr9.set_host("127.0.0.1");
	addr9.set_port(1234);
	printf("addr9 is: %s\n", addr9.to_str().c_str());

	XSockAddr addr10 = XSockAddr::local_mainaddr();
	printf("local main addr is: %s, hostname:%s\n", addr10.get_hostaddr().c_str(), addr10.get_hostname().c_str());
	printf("local net name: %s\n", XSockAddr::local_net_name().c_str());

	vector<XSockAddr> addrs;
	XSockAddr::local_addrs(addrs);
	for (int i = 0; i < (int)addrs.size(); i++)
	{
		XSockAddr& addr = addrs[i];
		printf("local addr: %s\n", addr.get_hostaddr().c_str());
	}

	printf("addr1 hostaddr:%s, hostname:%s\n", addr1.get_hostaddr().c_str(), addr1.get_hostname().c_str());
	printf("addr3 hostaddr:%s, hostname:%s\n", addr3.get_hostaddr().c_str(), addr3.get_hostname().c_str());
	printf("addr7 hostaddr:%s, hostname:%s\n", addr7.get_hostaddr().c_str(), addr7.get_hostname().c_str());

	ASSERT(addr7 == addr5);
	ASSERT(addr8 != addr7);
	ASSERT(addr7 >= addr1);
	ASSERT(addr1.is_any());

	return true;
}

#endif//_XCORE_NEED_TEST

}//namespace xcore
