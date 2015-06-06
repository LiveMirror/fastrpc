// 2008-12-03
// xcore_socket.h
//
// socket¼òµ¥·â×°


#ifndef _XCORE_SOCKET_H_
#define _XCORE_SOCKET_H_

#include "xcore_define.h"
#include "xcore_noncopyable.h"
#include "xcore_sock_addr.h"
#include "xcore_thread.h"
#include "xcore_mutex.h"

namespace xcore {

#ifdef __WINDOWS__
#pragma comment(lib, "ws2_32.lib")

struct iovec
{
	void*  iov_base;  // Base address
	size_t iov_len;   // Length
};
#endif//__WINDOWS__

////////////////////////////////////////////////////////////////////////////////
// class XSocket
////////////////////////////////////////////////////////////////////////////////
class XSocket : XNoncopyable
{
public:
	explicit XSocket(SOCKET sock = XCORE_INVALID_SOCKET) : m_sock(sock) {
		isstop = false;
    }
	~XSocket() {
    }

	SOCKET get_handle() const { return m_sock; }
	bool   is_open() const;
	void   attach(SOCKET sock);
	SOCKET detach();

	bool open(int type = SOCK_STREAM);
	bool bind(const XSockAddr& addr);
	bool connect(const XSockAddr& addr);
	bool listen(const XSockAddr& addr, int backlog = -1);
	bool accept(XSocket &sock, XSockAddr* remote_addr = NULL);
	bool shutdown();
	bool close(int delay = -1);
	bool mutex_close();
    bool reconnect(const XSockAddr& addr);
	bool abort();
	void StopAutoConnect(bool stop) {
		isstop = stop;
	}

	XSockAddr local_addr() const;
	XSockAddr remote_addr() const;

public:
	bool set_reuse_addr(bool bl = true);
	bool set_keep_alive(bool bl = true);
	bool set_nonblock(bool bl = true);
	bool set_tcp_nodelay(bool bl = true);
	bool set_linger(uint16 delay_sec, bool bl = true);
	bool set_send_bufsize(uint32 nSize = 8192);
	bool set_recv_bufsize(uint32 nSize = 8192);
	bool get_send_bufsize(uint32& nSize) const;
	bool get_recv_bufsize(uint32& nSize) const;

public:
	bool can_recv(int timeout_ms = -1);
	bool can_send(int timeout_ms = -1);

	// return >=0: send/recv number of bytes, -1: error or peer host closed
	int send(const void* buf, int len);
	int recv(void* buf, int len);
	int sendto(const void* buf, int len, const XSockAddr& addr);
	int recvfrom(void* buf, int len, XSockAddr& addr);
	int send_n(const void* buf, int len, int timeout_ms = -1);
	int recv_n(void* buf, int len, int timeout_ms = -1);
    int recv_one_http(char*& buf, int& len,
                               char*& last_left, int& left_len,
                               char*& http_head, int timeout_ms);

	bool send_v(const iovec* iov, int cnt);

	volatile SOCKET m_sock;

private:
	bool _is_can_restore();
	bool _is_already();
	bool _is_would_block();
	bool _is_emfile();

private:
	XMutex mutex_;
	bool isstop;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_SOCKET_H_
