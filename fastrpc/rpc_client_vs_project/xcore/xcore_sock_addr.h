// 20010-09-29
// xcore_sock_addr.h
// 
// SockµØÖ·Àà


#ifndef _XCORE_SOCK_ADDR_H_
#define _XCORE_SOCK_ADDR_H_

#include "xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XSockAddr
////////////////////////////////////////////////////////////////////////////////
class XSockAddr
{
public:
	static const XSockAddr AnyAddr;
	static const XSockAddr NoneAddr;

	XSockAddr(void);
	XSockAddr(const XSockAddr& addr); 
	XSockAddr(const sockaddr_in& addr);
	XSockAddr(const sockaddr& addr);
	XSockAddr(const string& addr); // addr such as "192.168.4.12:80" or "www.sina.com.cn:80"
	XSockAddr(const string& host, uint16 port); // host such as "192.168.4.12" or "www.sina.com.cn"
	XSockAddr(uint32 ip, uint16 port); // ip: 0x12131415 => 18.19.20.21
	~XSockAddr(void);
	
	XSockAddr& operator = (const XSockAddr& addr);
	XSockAddr& operator = (const sockaddr_in& addr);
	XSockAddr& operator = (const sockaddr& addr);
	operator const sockaddr_in *() const;
	operator const sockaddr *() const;
	operator sockaddr_in() const;
	operator sockaddr() const;

	void   set_port(uint16 port);
	void   set_ipaddr(uint32 ip); // ip: 0x12131415 => 18.19.20.21
	bool   set_ipaddr(const string& ip); // ip such as "192.168.5.12"
	bool   set_host(const string& host); // host such as "192.168.4.12" or "www.sina.com.cn"
	uint16 get_port() const;
	uint32 get_ipaddr() const;
	string get_hostname() const;
	string get_hostaddr() const;
	string to_str() const;
	void   reset();

	bool is_any() const;
	bool is_none() const;
	bool is_loopback() const;
	bool is_multicast() const;

	static string    local_net_name();
	static XSockAddr local_mainaddr();
	static bool local_addrs(vector<XSockAddr>& addrs);

	friend bool operator <  (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator <= (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator >  (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator >= (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator == (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator != (const XSockAddr& addr1, const XSockAddr& addr2);

private:
	sockaddr_in  m_inaddr;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_SOCK_ADDR_H_
