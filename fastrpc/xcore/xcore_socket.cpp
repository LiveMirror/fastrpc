// 2008-12-03
// xcore_socket.cpp
//
// socket简单封装


#include "xcore_socket.h"
#include <errno.h>

namespace xcore {

class IdleSock
{
public:
	IdleSock()
	{
		m_sock.open();
	}

	~IdleSock()
	{
		m_sock.close();
	}

	void attach(SOCKET sock)
	{
		m_sock.attach(sock);
	}

private:
	XSocket m_sock;
};
static IdleSock  g_idleSock;

///////////////////////////////////////////////////////////////////////
// class XSocket
///////////////////////////////////////////////////////////////////////

void XSocket::attach(SOCKET sock)
{
	if (m_sock != sock)
	{
		this->close();
		m_sock = sock;
	}
}

SOCKET XSocket::detach()
{
	SOCKET sock = m_sock;
	m_sock = XCORE_INVALID_SOCKET;
	return sock;
}

bool XSocket::is_open() const
{
	return (m_sock != XCORE_INVALID_SOCKET);
}


///////////////////////////////////////////////////////////////////////
// base interfaces
///////////////////////////////////////////////////////////////////////
bool XSocket::open(int type)
{
	this->close();
	m_sock = ::socket(PF_INET, type, 0);
    this->set_nonblock(true); // add by seinathe 2016.01.06 默认都异步吧
	return (XCORE_INVALID_SOCKET != m_sock);
}

bool XSocket::bind(const XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	ASSERT(local_addr() == XSockAddr::AnyAddr  && "Socket cann't bind two address.");
	return (0 == ::bind(m_sock, addr, sizeof(sockaddr)));
}

int XSocket::connect(const XSockAddr& addr, int timeout_ms)
{
    if (m_sock == XCORE_INVALID_SOCKET) return false;

    int ret = ::connect(m_sock, addr, sizeof(sockaddr));
    bool first = true;
    if (ret < 0)
    {
        do {
            if (!first) {
                if (!can_send(timeout_ms)) {
                    return -2;
                }
                ret = ::connect(m_sock, addr, sizeof(sockaddr));
                if (0 == ret) return true;
            } else {
                first = false;
            }
            if (_is_can_restore())
            {
                continue;
            }
            //else if (_is_would_block())
            else if ((EAGAIN == errno) ||
                    (EWOULDBLOCK == errno) ||
                    (EINPROGRESS == errno))
            {
                continue;
            }
            else if ((EALREADY == errno) ||
                    (EISCONN == errno))
            {
                return 1;
            }
            printf("connect errno:%d\n", errno);
            return -1;
        } while (true);
    }
    return 1;
}

bool XSocket::connect(const XSockAddr& addr)
{
    if (m_sock == XCORE_INVALID_SOCKET) return false;

    int ret = ::connect(m_sock, addr, sizeof(sockaddr));
    bool first = true;
    if (ret < 0)
    {
        do {
            if (!first) {
                if (!can_send(20000)) {
                    return false;
                }
                ret = ::connect(m_sock, addr, sizeof(sockaddr));
                if (0 == ret) return true;
            } else {
                first = false;
            }
            if (_is_can_restore())
            {
                continue;
            }
            //else if (_is_would_block())
            else if ((EAGAIN == errno) ||
                    (EWOULDBLOCK == errno) ||
                    (EINPROGRESS == errno))
            {
                continue;
            }
            else if ((EALREADY == errno) ||
                    (EISCONN == errno))
            {
                return true;
            }
            printf("connect errno:%d\n", errno);
            return false;
        } while (true);
    }
    return true;

	//if (m_sock == XCORE_INVALID_SOCKET) return false;

	//do
	//{
	//	int ret = ::connect(m_sock, addr, sizeof(sockaddr));
	//	if (ret < 0)
	//	{
	//		if (_is_can_restore())
	//		{
	//			continue;
	//		}
	//		else if (_is_already() || _is_would_block())
	//		{
	//			return true;
	//		}
	//		return false;
	//	}
	//	return true;
	//} while (false);

	//return false;
}

bool XSocket::listen(const XSockAddr& addr, int backlog)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	if (backlog < 0) backlog = 0X7FFFFFFF;
	if (!set_reuse_addr(true)) return false;
	if (!this->bind(addr)) return false;
	if (0 != ::listen(m_sock, backlog)) return false;
	return true;
}

bool XSocket::accept(XSocket &sock, XSockAddr* remote_addr)
{
	ASSERT(this != &sock);
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	SOCKET s = XCORE_INVALID_SOCKET;
	sock.attach(XCORE_INVALID_SOCKET);

	do
	{
		struct sockaddr addr = {};
		socklen_t sockLen = sizeof(sockaddr);
		s = ::accept(m_sock, (sockaddr *)&addr, &sockLen);
		if (remote_addr) *remote_addr = addr;
		if (s == XCORE_INVALID_SOCKET)
		{
			if (_is_can_restore())
			{
				continue;
			}
			else if (_is_would_block())
			{
				break;
			}
			else if (_is_emfile())
			{
				g_idleSock.attach(XCORE_INVALID_SOCKET);
				g_idleSock.attach(::accept(m_sock, NULL, NULL));
				g_idleSock.attach(XCORE_INVALID_SOCKET);
				g_idleSock.attach(::socket(AF_INET, SOCK_STREAM, 0));
				break;
			}
			return false;
		}
	} while (false);

	sock.attach(s);
	return true;
}

bool XSocket::abort()
{
	return close(0);
}

XSockAddr XSocket::local_addr() const
{
	if (m_sock == XCORE_INVALID_SOCKET) return XSockAddr::AnyAddr;

	struct sockaddr saddr = {};
	socklen_t namelen = (socklen_t)sizeof(sockaddr);
	if (0 != getsockname(m_sock, &saddr, &namelen)) return XSockAddr::AnyAddr;
	return saddr;
}

XSockAddr XSocket::remote_addr() const
{
	if (m_sock == XCORE_INVALID_SOCKET) return XSockAddr::AnyAddr;

	struct sockaddr saddr = {};
	socklen_t namelen = (socklen_t)sizeof(sockaddr);
	if (0 != getpeername(m_sock, &saddr, &namelen)) return XSockAddr::AnyAddr;
	return saddr;
}

///////////////////////////////////////////////////////////////////////
// option interfaces
///////////////////////////////////////////////////////////////////////
bool XSocket::set_reuse_addr(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_keep_alive(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_tcp_nodelay(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_linger(uint16 delay_sec, bool bl)
{
	struct linger lingerStruct = {(uint16)bl, delay_sec};
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
	return (0 == ret);
}

bool XSocket::set_nonblock(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	#ifdef __WINDOWS__
	unsigned long ulOption = (unsigned long)bl;
	int ret = ioctlsocket(m_sock, FIONBIO, (unsigned long *)&ulOption);
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int ret = fcntl(m_sock, F_GETFL, 0);
	if (ret == -1) return false;
	if (bl)
		ret = fcntl(m_sock, F_SETFL, ret | O_NONBLOCK);
	else
		ret = fcntl(m_sock, F_SETFL, ret & (~O_NONBLOCK));
	#endif//__GNUC__

	return (0 == ret);
}

bool XSocket::set_send_bufsize(uint32 nSize)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int ret = setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSize, sizeof(int));
	return (0 == ret);
}

bool XSocket::get_send_bufsize(uint32& nSize) const
{
	nSize = 0;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	socklen_t optlen = sizeof(int);
	int ret = getsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&nSize, &optlen);
	return (0 == ret);
}

bool XSocket::set_recv_bufsize(uint32 nSize)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int ret = setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (const char*)&nSize, sizeof(int));
	return (0 == ret);
}

bool XSocket::get_recv_bufsize(uint32& nSize) const
{
	nSize = 0;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	socklen_t optlen = sizeof(int);
	int ret = getsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, &optlen);
	return (0 == ret);
}

///////////////////////////////////////////////////////////////////////
// date interfaces
///////////////////////////////////////////////////////////////////////
int XSocket::send(const void* buf, int len)
{
	if ((m_sock == XCORE_INVALID_SOCKET)) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::send(m_sock, (const char *)buf, len, 0);
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::recv(void* buf, int len)
{
	if ((m_sock == XCORE_INVALID_SOCKET)) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::recv(m_sock, (char *)buf, len, 0);
		if (ret == 0) return -1; // closed by remote host
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::sendto(const void* buf, int len, const XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::sendto(m_sock, (const char *)buf, len, 0, addr, sizeof(sockaddr));
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::recvfrom(void* buf, int len, XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;
	addr.reset();

	do
	{
		struct sockaddr saddr;
		socklen_t fromlen = (socklen_t)sizeof(sockaddr);
		int ret = ::recvfrom(m_sock, (char *)buf, len, 0, &saddr, &fromlen);
		if (ret == 0) return -1; // shutdown by remote host
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		addr = saddr;
		return ret;
	} while (false);

	return 0;

}

int XSocket::send_n(const void* buf, int len, int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || len <= 0) return 0;

	int sendsize = 0;
	do
	{
		if (!can_send(timeout_ms)) {
            //printf("send time out\n");
            return sendsize;
        }
		int ret = this->send((const char *)buf + sendsize, len - sendsize);
		if (ret < 0) return -1;
		sendsize += ret;
	} while(sendsize < len);

	ASSERT(sendsize == len);
	return sendsize;
}

/*find str form begin*/
char* find_str_begin(const char* src,unsigned srclen,const char* matchstr,unsigned matchstrlen) {
    if(src == NULL || matchstr == NULL || srclen <= 0 || matchstrlen <= 0) {
        return NULL;
    }
    unsigned k,j;
    char* p_str = (char*)src;
    for(unsigned i = 0; i < srclen; i++) {
        j = i;
        k = 0;
        while(j < srclen && (tolower(p_str[j]) == tolower(matchstr[k]))) {
            j++;
            k++;
            if( k == matchstrlen) {
                return (p_str + j - matchstrlen);
            }
        }
    }
    return NULL;
}

/*
   0       还需要继续接收
   -1      失败
   */
int is_http_complete(const char* p_data, unsigned int data_len) {
    if(p_data == NULL || data_len < 4 || data_len<= 0) {
        return 0;
    }
    /*first match http head end*/
    int head_len = 0;
    char *p_head_end = find_str_begin(p_data,data_len,"\r\n\r\n",4);
    if(p_head_end == NULL) {
        p_head_end = find_str_begin(p_data,data_len,"\n\n",2);
        if ( p_head_end == NULL ) return 0;
        else head_len = p_head_end + 2 - p_data;
    } else {
        head_len = p_head_end + 4 - p_data;
    }
    //get Content-Length:
    char* pcontent_length = find_str_begin(p_data,data_len,"Content-Length:",15);
    if(pcontent_length != NULL) {
        int content_length = atoi(pcontent_length + 15);
        if( (int)(data_len) < content_length + head_len) {
            return 0;
        } else {
            return content_length + head_len;
        }
    } else {
        // try find chunkend
        char* chunk_end = find_str_begin(p_data,data_len,"\r\n0\r\n\r\n",7);
        if (chunk_end) {
            return (chunk_end + 7 - p_data);
        }
        // 那就是只有头部了
        return head_len;
    }
    return -1;
}


int XSocket::recv_one_http(char*& buf, int& len,
                       char*& last_left, int& left_len,
                       char*& http_head, int timeout_ms) {
    if (m_sock == XCORE_INVALID_SOCKET) return -1;
    if ((buf == NULL) || len <= 0) return 0;
    int recvsize = 0;
    if (left_len > 0) {
        do{// http包检测 server端检测函数更高效 后续优化
            int ret = is_http_complete(last_left, left_len);
            if (ret > 0) {
                http_head = last_left;
                last_left = last_left + ret;
                left_len = left_len - ret;
                return ret;
            }
        } while(false);
        if (left_len > 0) {
            memmove(buf, last_left, left_len);
            recvsize = left_len;
        }
    }
    int cont_len = -1;
    char *head_end = NULL;
    char *content_len_end = NULL;
    do {
        // 从网络拿数据
        if (!can_recv(timeout_ms)) {
            //printf("recv time out\n");
            continue;
        }
        int ret = this->recv((char *)buf + recvsize, len - recvsize);
        if (ret < 0) {
            http_head = NULL;
            left_len = 0;
            return -1;
        }
        recvsize += ret;
        do{// http包检测 server端检测函数更高效 后续优化
            int ret = is_http_complete(buf, recvsize);
            if (ret > 0) {
                http_head = buf;
                last_left = buf + ret;
                left_len = recvsize - ret;
                return ret;
            }
        } while(false);
        if (recvsize + 1024 > len) {
            len += 10*1024;
            buf = (char*)realloc(buf, len);
        }
    } while(recvsize < len);
    return -1;
}

int XSocket::recv_one_http(std::string& one_http, int timeout_ms) {
    int buf_size = 1024*100;
    char *buf = (char*)malloc(buf_size);
    char *last_left = buf;
    int left_len = 0;
    char* http_head = NULL;
    int one_len = recv_one_http(buf, buf_size, last_left, left_len, http_head, timeout_ms);
    if (one_len > 0) {
        one_http.assign(buf, one_len);
    }
    free(buf);
    return one_len;
}

bool XSocket::mutex_close() {
	bool ret = false;
	if (!isstop) {
		mutex_.lock();
		close();
		mutex_.unlock();
		ret = true;
	}
	return ret;
}

bool XSocket::reconnect(const XSockAddr& addr) {
	bool ret = false;
	if (!isstop) {
		mutex_.lock();
		close();
		//int tryc = 0;
		int waittime = 100;
		while (!ret) {
			open(SOCK_STREAM);
			set_nonblock(true);
			ret = connect(addr);
			if (ret) {
				break;
			} else {
				close();
			}
			xcore::sleep(waittime);
			waittime *= 2;
			if (waittime > 4000) {
                break;
			}
		}
		mutex_.unlock();
	}
	return ret;
}


int XSocket::recv_n(void* buf, int len, int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || len <= 0) return 0;

	int recvsize = 0;
	do
	{
		if (!can_recv(timeout_ms)) return recvsize;
		int ret = this->recv((char *)buf + recvsize, len - recvsize);
		if (ret < 0)
		{
			if (recvsize > 0)
				return recvsize;
			else
				return -1;
		}
		recvsize += ret;
	} while(recvsize < len);

	ASSERT(recvsize == len);
	return recvsize;
}

///////////////////////////////////////////////////////////////////////
// private interfaces
///////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__
bool XSocket::can_recv(int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return true;

	struct timeval tv = {};
	timeval *pcTimeout = NULL;
	if (timeout_ms >= 0)
	{
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		pcTimeout = &tv;
	}

	do
	{
		fd_set rdset, exceptset;
		FD_ZERO(&rdset);
		FD_ZERO(&exceptset);
		FD_SET(m_sock, &rdset);
		FD_SET(m_sock, &exceptset);

		// If timeout is NULL (no timeout), select can block indefinitely.
		// In windows, pcTimeout not altered; In linux, pcTimeout may update.
		int ret = select((int)m_sock + 1, &rdset, NULL, &exceptset, pcTimeout);
		if (ret == 0) return false;
		if (FD_ISSET(m_sock, &rdset) || FD_ISSET(m_sock, &exceptset)) return true;
		if (_is_can_restore()) continue;
		//fprintf(stderr, "socket select return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket select exception.");
	} while (false);

	return false;
}

bool XSocket::can_send(int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	struct timeval tv = {};
	timeval *pcTimeout = NULL;
	if (timeout_ms >= 0)
	{
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		pcTimeout = &tv;
	}

	do
	{
		fd_set wrset, exceptset;
		FD_ZERO(&wrset);
		FD_ZERO(&exceptset);
		FD_SET(m_sock, &wrset);
		FD_SET(m_sock, &exceptset);

		// If timeout is NULL (no timeout), select can block indefinitely.
		// In windows, pcTimeout not altered; In linux, pcTimeout may update.
		int ret = select((int)m_sock + 1, NULL, &wrset, &exceptset, pcTimeout);
		if (ret == 0) return false;
		if (FD_ISSET(m_sock, &wrset)) return true;
		if (FD_ISSET(m_sock, &exceptset)) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket select return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket select exception.");
	} while (false);

	return false;
}

bool XSocket::send_v(const iovec* iov, int cnt)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;
	if (iov == NULL || cnt == 0) return true;
	ASSERT(cnt <= 20);

	WSABUF bufs[20];
	ULONG total = 0;
	for (int i = 0; i < cnt; i++)
	{
		ASSERT(iov->iov_base);
		bufs[i].buf = (CHAR *)iov->iov_base;
		bufs[i].len = (ULONG)iov->iov_len;
		total += bufs[i].len;
	}

	DWORD snds = 0;
	if (0 != WSASend(m_sock, bufs, cnt, &snds, 0, NULL, NULL)) return false;
	if (snds != total) return false;
	return true;
}

bool XSocket::shutdown()
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		return (-1 != ::shutdown(m_sock, SD_SEND));
	}
	return true;
}

bool XSocket::close(int delay)
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		SOCKET sock = m_sock;
		m_sock = XCORE_INVALID_SOCKET;

		if (delay >= 0)
		{
			struct linger linger_ = { 1, delay };
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
		}

		return (-1 != ::closesocket(sock));
	}
	return true;
}

bool XSocket::_is_can_restore()
{
	return (WSAEINTR == WSAGetLastError());
}

bool XSocket::_is_already()
{
	return ((WSAEALREADY == WSAGetLastError()) ||
		    (WSAEINPROGRESS == WSAGetLastError()) ||
			(WSAEISCONN == WSAGetLastError()));
}

bool XSocket::_is_would_block()
{
	return ((WSAEWOULDBLOCK == WSAGetLastError()) ||
		    (WSA_IO_PENDING == WSAGetLastError()));
}

bool XSocket::_is_emfile()
{
	return (WSAEMFILE == WSAGetLastError());
}
#endif//__WINDOWS__

#ifdef __GNUC__
bool XSocket::can_recv(int timeout_ms)
{
	if (timeout_ms < 0) timeout_ms = -1;
	if (m_sock == XCORE_INVALID_SOCKET) return true;

	do
	{
		struct pollfd  event;
		event.fd = m_sock;
		event.events = POLLIN;  // 不考虑带外数据
		int ret = poll(&event, 1, timeout_ms);
		if (ret > 0)
		{
			if (event.revents & POLLIN)
			{
				int err = 0;
				socklen_t len = (socklen_t)sizeof(err);
				if (getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) return false;
				return (err == 0);
			}
			else
			{
				ASSERT(false);
				return false;
			}
		}
		if (ret == 0) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket poll return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket poll exception.");
	} while (false);

	return false;
}

bool XSocket::can_send(int timeout_ms)
{
	if (timeout_ms < 0) timeout_ms = -1;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	do
	{
		struct pollfd  event;
		event.fd = m_sock;
		event.events = POLLOUT;
		int ret = poll(&event, 1, timeout_ms);
		if (ret > 0)
		{
			if (event.revents & POLLOUT)
			{
				int err = 0;
				socklen_t len = (socklen_t)sizeof(err);
				if (getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) return false;
				return (err == 0);
			}
			else
			{
				return false;
			}
		}
		if (ret == 0) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket poll return %d, errno:%d\n", ret, errno);
		ASSERT(!"socket poll exception.");
	} while (false);

	return false;
}

bool XSocket::send_v(const iovec* iov, int cnt)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;
	if (iov == NULL || cnt == 0) return true;

	size_t total = 0;
	for (int i = 0; i < cnt; i++)
	{
		ASSERT(iov->iov_base);
		total += iov->iov_len;
	}

	int ret = ::writev(m_sock, iov, cnt);
	if (ret < 0) return false;
	if (ret != (int)total) return false;
	return true;
}

bool XSocket::shutdown()
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		return (-1 != ::shutdown(m_sock, SHUT_WR));
	}
	return true;
}

bool XSocket::close(int delay)
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		SOCKET sock = m_sock;
		m_sock = XCORE_INVALID_SOCKET;

		if (delay >= 0)
		{
			struct linger linger_ = { 1, delay };
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
		}

		return (-1 != ::close(sock));
	}
	return true;
}

bool XSocket::_is_can_restore()
{
	return (EINTR == errno);
}

bool XSocket::_is_already()
{
	return ((EALREADY == errno) ||
		    (EINPROGRESS == errno) ||
			(EISCONN == errno));
}

bool XSocket::_is_would_block()
{
	return ((EAGAIN == errno) ||
		    (EWOULDBLOCK == errno));
}

bool XSocket::_is_emfile()
{
	return (EMFILE == errno);
}
#endif//__GNUC__

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"
#include "xcore_utility.h"

namespace xcore
{

bool xcore_test_socket()
{
	XSocket sock_, sock2_;
	sock_.open(SOCK_DGRAM);
	printf("bind address: %s\n", sock_.local_addr().to_str().c_str());
	//VERIFY(sock_.bind(XSockAddr("", 2222)));
	//printf("bind address: %s\n", sock_.local_addr().to_str().c_str());
	//VERIFY(sock_.bind(XSockAddr("", 2223)));
	//printf("bind address: %s\n", sock_.local_addr().to_str().c_str());
	//VERIFY(sock_.listen(XSockAddr("", 2222)));
	//VERIFY(sock_.accept(sock2_));
	sock_.close();
	sock2_.close();

	XSocket sock1;
	sock1.open(SOCK_STREAM);
	VERIFY(sock1.listen(XSockAddr("", 0)));

	//XSocket sock2 = sock1.get_handle();
	XSocket sock3(sock1.get_handle());
	printf("local:%s, remote:%s\n", sock3.local_addr().to_str().c_str(), sock3.remote_addr().to_str().c_str());

	sock3.detach();
	sock1.close();

	XSocket sock4;
	sock4.open();
	if (!sock4.connect(XSockAddr("127.0.0.1", 3344)))
	{
		sock4.close();
		printf("connect to port 3344 failed.\n");
		return true;
	}
	printf("connect to port 3344 ok.\n");
	sock4.close();

	XSocket sock;
	sock.open();
	sock.set_nonblock();
	//bool bRet = sock.connect(XSockAddr("202.118.66.221", 54445));
	bool bRet = sock.connect(XSockAddr("127.0.0.1", 54445));
	if (!bRet || !sock.can_send(1000))
	{
		printf("connect to remote port failed, ret:%d errno:%d\n", (int)bRet, xcore::errnum());
		sock.close();
		return true;
	}
	printf("connect to remote port OK.\n");
	sock.close();
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
