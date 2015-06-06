/**
 * @file
 * @brief 网络库
 */
//**********************************************************
//          Net Utility 1.0
//
//  Description:
//   Wrap and functional functions about net communication.
//
// Author: Wang Chengqing
//         cwang@baidu.com
//**********************************************************

#ifndef __UL_NETLIB_H__
#define	__UL_NETLIB_H__

//******************************************
//include area 
//*****************************************
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "dep/dep_net.h"
#include "ul_conf.h"

#include <sys/poll.h>

#define IPACCESS_GRANT_LEN                     256
extern char ul_ipaccess_grant[IPACCESS_GRANT_LEN + 1];

/**
 * socket()的包装函数
 *
 * @note 参数和返回值与socket()相同
 */
extern int ul_socket(int family, int type, int protocol);

/**
 * connect()的包装函数
 *
 * @note 参数和返回值与connect()相同
 */
extern int ul_connect(int sockfd, const struct sockaddr *sa, socklen_t addrlen);

/**
 * bind()的包装函数
 *
 * @note 参数和返回值与bind()相同
 */
extern int ul_bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);

/**
 * listen()的包装函数
 *
 * @note 参数和返回值与listen()相同
 */
extern int ul_listen(int sockfd, int backlog);

/*
 * function : read the global ip access grant in config struct.
 *          : only those ips matching the grant string will be
 *          : allowed
 * argu-in  : pd_conf, the config struct
 * argu-out : the global variable ul_ipaccess_grant
 * return   : 0 always
 * notice   : the name-value pair format:
 *          : ip-access-grant : grant-string
 *          : grant-string as 
 *          : *.192.168-160.*;127.*.*.*;...
 *          : if  ip-access-grant does not exist, default string 
 *          :    ("127.0.0.1;192.168.1.*") will be set.
 *          : if this function have never been called, all ip are allowed
 */
extern int ul_read_ipaccess_grant(ul_confdata_t * pd_conf);

/**
 * accept()的包装和增强函数
 * 解决了连接在accept之前被reset的问题（<b>UNPv1:Connection Abort before \c accept Returns</b> ）\n
 * 提供ip访问控制（定义ul_ipaccess_grant）
 *
 * @note 参数和返回值与accept()相同
 */
extern int ul_accept(int sockfd, struct sockaddr *sa, socklen_t * addrlen);

/**
 * getsockname()的包装函数
 *
 * @note 参数和返回值与getsockname()相同
 */
extern int ul_getsockname(int sockfd, struct sockaddr *localaddr, socklen_t * addrlen);

/**
 * getpeername()的包装函数
 *
 * @note 参数和返回值与getpeername()相同
 */
extern int ul_getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t * addrlen);

/**
 * setsockopt()的包装函数
 *
 * @note 参数和返回值与setsockopt()相同
 */
extern int ul_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/**
 * getservbyname_r()的包装函数
 *
 * @note 参数和返回值与getservbyname_r()相同
 */
extern int ul_getservbyname_r(const char *name, const char *proto, struct servent *result_buf,
							  char *buf, size_t buflen, struct servent **result);

/**
 * getservbyport_r()的包装函数
 *
 * @note 参数和返回值与getservbyport_r()相同
 */
extern int ul_getservbyport_r(int port, const char *proto, struct servent *result_buf,
							  char *buf, size_t buflen, struct servent **result);

/**
 * select()的包装函数
 * 被信号中断以后可自动重启
 *
 * @note 参数和返回值与select()相同
 */
extern int ul_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
					 struct timeval *timeout);


/**
 * poll的包装函数
 * 被信号中断以后可自动重启
 * @note 参数和返回值与poll()相同
 */
extern int ul_poll(struct pollfd *fdarray, unsigned int nfds, int timeout);

/**
 * 获得本机的字节序
 *
 * @return 1为Big-endian，0为Little-endian，-1为判断出错
 */
extern int ul_gethostbyteorder();

# define	UL_GET	1
# define	UL_SET	0
/******************************************************
 * function : Operate the receive buff of socket.
 * argu     : s:the socket id;
 *          : optval: the address which store the buffer size.
 *          : mode: operate mode.one is UL_GET and the other is UL_SET.
 * return   : just a simple wrap of getsockopt and setsockopt
 *          : return value is same with them
 ******************************************************/
extern int ul_opsockopt_rcvbuf(int s, void *optval, char mode);

//******************************************************
// Operate the send buff of socket.
// para:
//      s:the socket id;
//      optval: the address which store the buffer size.
//      mode:   operate mode.one is GET and the other is UL_SET.
//******************************************************
extern int ul_opsockopt_sndbuf(int s, void *optval, char mode);

#define		UL_CLOSE	0
#define		UL_OPEN	1
//******************************************************
// Operate the switch SO_REUSEADDR. Use this function you could 
// find the state<OPEN/CLOSE> of this switch 
// para:
//      s:      the socket id;
//      optval: the address which store the operation.the relevant value is OPEN/CLOSE
//      mode:   operate mode.one is UL_GET and the other is UL_SET.
//******************************************************
extern int ul_opsockopt_ruseaddr(int s, int *op, char mode);
#if SO_REUSEPORT
extern int ul_opsockopt_ruseport(int s, int *op, char mode);
#endif

//******************************************************************
// Operate the switch SO_KEEPALIVE. Use this function you could 
// find the state<OPEN/CLOSE> of this switch 
// para:
//      s:      the socket id;
//      optval: the address which store the operation.the relevant value is OPEN/CLOSE
//      mode:   operate mode.one is UL_GET and the other is UL_SET.
//******************************************************************
extern int ul_opsockopt_keepalive(int s, int *op, char mode);

//****************************************************************** 
// Operate the option SO_SNDTIMEO. Use this function you could
// find the current value of this option
// para:
//      sockfd:      the socket id;
//      struct timeval : the time struct which store the time.
//      mode:   operate mode.one is UL_GET and the other is UL_SET.
//******************************************************
int ul_opsockopt_sndtimeo(int sockfd, struct timeval *tv, char mode);

//******************************************************
// /Get/Set socket read timeout
// Operate the option SO_RCVTIMEO. Use this function you could
// find the current value of this option
// para:
//      sockfd:      the socket id;
//      struct timeval : the time struct which store the time.
//      mode:   operate mode.one is UL_GET and the other is UL_SET.
//******************************************************
int ul_opsockopt_rcvtimeo(int sockfd, struct timeval *tv, char mode);

//********************************************************************
// Fill data struct <struct sockaddr_in> of IPV4.
//********************************************************************
int ul_fillsa4(struct sockaddr_in *sin, char *ip, int port);

//********************************************************************
//  TcpListen:
//      Start a server on local machine at port number with ANY address
//  queue, the listen sock backlog number
//********************************************************************
/**
 * 在指定端口和所有本地IP上启动监听
 *
 * @param port 端口
 * @param queue 连接队列长度（listen中的backlog参数）
 * @return -1失败，否则为监听socket
 */
extern int ul_tcplisten(int port, int queue);

//****************************************************************
// function : a  time-out version of connect() 
// for millisecond
// while exit on error,close sockfd
// sockfd,saptr,socklen, as the same as connect
// secs, the ovet time, in second
//****************************************************************
extern int ul_connecto_sclose_ms(int sockfd, const struct sockaddr *saptr, socklen_t socklen,
								 int msecs);
//****************************************************************
// function : a  time-out version of connect().
// sockfd,saptr,socklen, as the same as connect
// isecs, the ovet time, in millisecond 
// msec = 0 return immediately
// msec < 0 not return until connected
//****************************************************************
/**
 * 功能同@ref ul_connecto()，超时值单位为毫秒
 *
 * @param sockfd,saptr,secs 参数与connect()相同
 * @param msecs 超时值（ms）
 * @return 0成功，-1失败
 */
extern int ul_connecto_ms_ex(int sockfd, const struct sockaddr *saptr, socklen_t socklen,
							 int msecs);


//********************************************************************
//  ul_tcpconnecto_ms
//      Time out version for millisecond
//      Build up a TCP connection to host at port.  This is used at
//      client part, returns the socket fd on success, or -1 on error
// argu  : host,port, the target host and it's port
//         secs, the over time, in second
//         线程不安全
//********************************************************************
extern int ul_tcpconnecto_ms(char *host, int port, int msecs);

//******************************************************
// Start a unix domain server on a path.
// Return value:
// On success,return a socket descriptor.Otherwise,return -1.
//*******************************************************
extern int ul_tcpdomainlisten(char *path, int queue);

//********************************************************************
// Bulid up a unix domain connection.
// On success,return the socket descriptor.Otherwise,return -1.
//********************************************************************
extern int ul_tcpdomainconnect(char *path);

/**
 * 向socket写入指定长度的数据，直到写完或者socket出错(block mode)
 *
 * @param fd 读取socket
 * @param ptr1 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @return -1出错，否则为实际写入字节数
 *
 * @note socket需要处于阻塞模式
 */
extern ssize_t ul_swrite(int fd, void *ptr1, size_t nbytes);

/**
 * recv()的包装函数
 *
 * @note 参数和返回值与recv()相同
 */
ssize_t ul_recv(int sockfd, void *buf, size_t len, int flags);

/**
 * send()的包装函数
 *
 * @note 参数和返回值与send()相同
 */
ssize_t ul_send(int sockfd, const void *buf, size_t len, int flags);

/**
 * 功能同@ref ul_sreadable()，超时值单位为毫秒
 *
 * @param fd socket句柄
 * @param mseconds 超时值（ms）
 * @return -1出错，0超时，>0可读
 */
extern int ul_sreadable_ms(int fd, int mseconds);

/**
 * 功能同@ref ul_swriteable()，超时值单位为毫秒
 *
 * @param fd socket句柄
 * @param mseconds 超时值（ms）
 * @return -1出错，0超时，>0可写
 */
extern int ul_swriteable_ms(int fd, int mseconds);

/**
 * 功能同@ref ul_sreadable_tv
 *
 * @param fd socket句柄
 * @param timeval 超时时间
 * @return -1出错，0超时，>0可写
 */
extern int ul_sreadable_tv(int fd, struct timeval *tv);

/**
 * 功能同@ref ul_sreado_ex()，超时值单位为毫秒
 *
 * @param fd 读取socket
 * @param ptr 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param mseconds 超时值（ms）
 * @return -1出错，否则为实际读取字节数
 */
extern ssize_t ul_sreado_ms_ex(int fd, void *ptr, size_t nbytes, int mseconds);
extern ssize_t ul_reado_tv(int fd, void *ptr1, size_t nbytes, struct timeval *tv);


/**
 * 功能同@ref ul_reado()，超时值单位为毫秒
 *
 * @param fd 读取socket
 * @param ptr1 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param msecs 超时值（ms）
 * @return -1出错，否则为实际读取字节数
 */
extern ssize_t ul_reado_ms(int fd, void *ptr1, size_t nbytes, int msecs);

/**
 * 功能同@ref ul_swrite_ex()，超时值单位为毫秒
 *
 * @param fd 写入socket
 * @param ptr 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param mseconds 超时值（ms）
 * @return -1出错，否则为实际写入字节数
 */
extern ssize_t ul_swriteo_ms_ex(int fd, void *ptr, size_t nbytes, int mseconds);



/**
 * @brief 读超时，实现与ul_sreado_ms_ex不同, CPU的消耗更少
 *
 * @param [in] socket   : 读的socket局柄
 * @param [out] ptr   : 需要读的buff
 * @param [in] nbytes   : 需要读的数据长度
 * @param [in] msecs   : 超时时间
 * @return  -1 出错,可以由errno判断出错结果 
**/
extern ssize_t ul_sreado_ms_ex2(int socket, void *ptr, size_t nbytes, int msecs);

/**
 * @brief 写超时，实现与ul_writeo_ms_ex， CPU的消耗更少
 *
 * @param [in] socket   : 写的socket句柄
 * @param [in] ptr   : 需要写的buff
 * @param [in] nbytes   : 需要写的长度
 * @param [in] msecs   : 超时时间
 * @return  -1 出错,可以由errno判断出错结果
**/
extern ssize_t ul_swriteo_ms_ex2(int socket, const void *ptr, size_t nbytes, int msecs);

/**
 * @brief 多段buff读数据
 *
 * struct iovec 结构 在系统头文件 sys/uio.h中定义
 *
 * struct iovec {
 *     __ptr_t iov_base;
 *     size_t iov_len;
 * };
 *
 * @param [in] socket   : 需要读据的socket
 * @param [out] ve   : 接受的多个buff
 * @param [in] count   : buff的个数
 * @param [in] msecs   : 超时时间
 * @note  count的大小不能超过宏  IOV_MAX
 * @return  -1 出错， 可以由errno判断出的结果 
**/
extern ssize_t ul_sreadv_ms(int socket, const struct iovec *ve, int count, int msecs);

/**
 * @brief 写入多段buff的数据
 *
 * struct iovec 结构 在系统头文件 sys/uio.h中定义
 *
 * struct iovec {
 *     __ptr_t iov_base;
 *     size_t iov_len;
 * };
 *
 * @param [in] socket   : 需要写数据的socket
 * @param [out] ve   : 需要写数据的多个buff
 * @param [in] count   : buff的个数
 * @param [in] msecs   : 超时时间
 * @note  count的大小不能超过宏  IOV_MAX
 * @return  -1 出错， 可以由errno判断出的结果 
**/

extern ssize_t ul_swritev_ms(int socket, const struct iovec *ve, int count, int msecs); 

//****************************************************************
// Read a line data from socket speed. For safety in mutithreads,you should defined
// data struct Rline.In this function ,will read MAXLINE data from socket once.
//****************************************************************
#define	MAXLINE	1240
typedef struct Rline{
	int read_fd;
	char *read_ptr;
	size_t read_maxlen;
	int rl_cnt;
	char *rl_bufptr;
	char rl_buf[MAXLINE];
} Rline;

/** 从socket读取一行字符串，直至缓冲区满，或者socket出错、超时或者关闭
 *
 * @param fd 读取数据的源socket句柄
 * @param p  数据接收缓冲区
 * @param len 数据缓冲区大小
 * @param Rline 读取缓冲区
 * @param msecs 超时时间 单位毫秒
 * @return -1 网络错误或者超时， 否则为实际读取字节数
 */ 
extern ssize_t ul_readline_speedo_ms_ex(int fd, void *p, size_t len, Rline *rlin, int msecs);

/**
 * close()的包装函数
 *
 * @note 参数和返回值与close()相同
 */
extern int ul_sclose(int fd);

//*******************************************************
// Close one socket and force FIN created and sent whenever this socket was
// been open by other process.
// argument howto include:
//              SHUT_RD<0>//SHUT_WR<1>//SHUT_RDWR<2>
//*******************************************************
/**
 * shutdown()的包装函数
 *
 * @note 参数和返回值与shutdown()相同
 */
extern int ul_shutdown(int sockid, int howto);

//**************************************
//  Get datat struct hostent of one host
//  mutithread safe and for LINUX version
//  note: you should define the size of buf was less then 8192
//      error code was returned by point h_err.
//  the error code is in err:
//  ERANGE:  buf size too small
//  HOST_NOT_FOUND: Authoritive Answer Host not found
//  TRY_AGAIN:   Non-Authoritive Host not found, or SERVERFAIL
//  NO_RECOVERY: Non recoverable errors, FORMERR, REFUSED, NOTIMP
//  NO_DATA:     Valid name, no data record of requested type
//  NO_ADDRESS:  no address, look for MX record
//  MutliThread:    safe
//**************************************
/**
 * gethostbyname_r()的包装函数
 *
 * @note 参数与gethostbyname_r()相同
 * @return 0成功，-1错误
 */
extern int ul_gethostbyname_r(const char *hostname, struct hostent *result_buf, char *buf,
							  int buflen, struct hostent **result, int *h_err);

//*******************************************************
//  Get IP address of one host. Return the h_addr item.
//  Return value:   0 :success;
//                  -1:hostname is error.
//                  -2:gethostbyname_r() call is error.
//                  You can look the errno number by Global Variable errno.
//  Out Mode:       #define BD_ASCII    0
//                  #define BD_NET      1
//  MutliThread:    safe
//******************************************************
# define	BD_ASCII	0
# define	BD_NET		1
extern int ul_gethostipbyname_r(const char *hostname, struct sockaddr_in *sin, int mode,
								char *asc_str);

# define IP_GROUP_NUM      10
typedef struct {
	int addrtype;
	u_int ip[IP_GROUP_NUM];
} sip_group;

//*******************************************************
//  Get IP address group of one host. Return the h_addr item.
//  MutliThread:        safe
//*****************************************************************************

extern int ul_gethostallipbyname_r(const char *hostname, struct sockaddr_in *sin, int mode,
								   char *asc_str, sip_group * sip);


//********************************************************
// Set a socket descriptor's flag about non_block
// Return value:
//      0:success;      -1:failed.
//********************************************************
int ul_setsocktonoblock(int sd);

//********************************************************
// Set a socket descriptor's flag about block
// Return value:
//      0:success;      -1:failed.
//********************************************************
int ul_setsocktoblock(int sd);

//**************************************************************
// Check the size of data in receive queue.
// Return value:
// on success,return the data size.Otherwise return -1
// 得到接收队列缓冲区的数据字节数
//**************************************************************
ssize_t ul_getrecvqueuesize(int sd);

//****************************************************************
// Get the port by server name
// Return value: on success,return the port. Otherwise,return -1.
// 根据服务名得到相应的端口号。
//****************************************************************
int ul_getportbyserv(const char *name, const char *proto, int *port);

#endif
