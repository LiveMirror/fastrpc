/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: dep_net.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_net_dep.h
 * @author feimat(com@baidu.com)
 * @date 2007/12/20 12:07:51
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __UL_NET_DEP_H_
#define  __UL_NET_DEP_H_

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

typedef struct Rline Rline;


/*
 * 这里的不推荐的函数都有更好的版本代替
 */

//****************************************************************
// function : a  time-out version of connect().
// while exit on error,close sockfd
// sockfd,saptr,socklen, as the same as connect
// secs, the ovet time, in second
//****************************************************************
/**
 * 功能同@ref ul_connecto()，如果出错关闭sockfd
 *
 * @param sockfd,saptr,secs 参数与connect()相同
 * @param secs 超时值（s）
 * @return 0成功，-1失败
 */
extern int ul_connecto_sclose(int sockfd, const struct sockaddr *saptr, socklen_t socklen, int secs);


/**
 * 从socket读取指定长度的数据，直到读够，或者socket出错或对方关闭(block mode)
 *
 * @param fd 读取socket
 * @param ptr1 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @return -1出错，否则为实际读取字节数
 *
 * @note socket需要处于阻塞模式
 */
extern ssize_t ul_sread(int fd, void *ptr1, size_t nbytes);



/**
 * 功能同@ref ul_sreado()，超时值单位为毫秒
 *
 * @param fd 读取socket
 * @param ptr 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param mseconds 超时值（ms）
 * @return -1出错，否则为实际读取字节数
 * @deprecated 问题同@ref ul_sreado()
 */
extern ssize_t ul_sreado_ms(int fd, void *ptr, size_t nbytes, int mseconds);



/**
 * 带超时控制的@ref ul_sread()
 *
 * @param fd 读取socket
 * @param ptr1 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param seconds 超时值（s）
 * @return -1出错，否则为实际读取字节数
 *
 * @deprecated 超时控制有问题，可能导致阻塞时间大大超过seconds
 */
extern ssize_t ul_sreado(int fd, void *ptr, size_t nbytes, int seconds);

/**
 * 带超时控制的@ref ul_sread()
 *
 * @param fd 读取socket
 * @param ptr1 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param seconds 超时值（s）
 * @return -1出错，否则为实际读取字节数
 *
 * @note 较精确的超时控制，推荐使用
 */
extern ssize_t ul_sreado_ex(int fd, void *ptr, size_t nbytes, int seconds);


/**
 * 功能同@ref ul_swriteo()
 *
 * @param fd 写入socket
 * @param ptr 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param mseconds 超时值（ms）
 * @return -1出错，否则为实际写入字节数
 *
 * @deprecated 超时控制有问题，可能导致长时间阻塞
 */
extern ssize_t ul_swriteo_ms(int fd, void *ptr, size_t nbytes, int mseconds);


/**
 * 带超时控制的@ref ul_swrite()
 *
 * @param fd 写入socket
 * @param ptr 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param seconds 超时值（s）
 * @return -1出错，否则为实际写入字节数
 *
 * @deprecated 超时控制有问题，可能导致长时间阻塞
 */
extern ssize_t ul_swriteo(int fd, void *ptr, size_t nbytes, int seconds);

/**
 * 带超时控制的@ref ul_swrite()
 *
 * @param fd 写入socket
 * @param ptr 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param seconds 超时值（s）
 * @return -1出错，否则为实际写入字节数
 *
 * @note 较精确的超时控制，推荐使用
 */
extern ssize_t ul_swriteo_ex(int fd, void *ptr, size_t nbytes, int seconds);


/**
 * 带超时控制的@ref ul_read()
 *
 * @param fd 读取socket
 * @param ptr1 数据接收缓冲区
 * @param nbytes 要读取的字节数
 * @param secs 超时值（s）
 * @return -1出错，否则为实际读取字节数
 */
extern ssize_t ul_reado(int fd, void *ptr1, size_t nbytes, int secs);


/**
 * 功能同@ref ul_writeo()，超时值单位为毫秒
 *
 * @param fd 写入socket
 * @param ptr1 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param mseconds 超时值（ms）
 * @return -1出错，否则为实际写入字节数
 */
extern ssize_t ul_writeo_ms(int fd, void *ptr, size_t nbytes, int mseconds);


/**
 * 带超时控制的@ref ul_write()
 *
 * @param fd 写入socket
 * @param ptr1 数据发送缓冲区
 * @param nbytes 要写入的字节数
 * @param seconds 超时值（s）
 * @return -1出错，否则为实际写入字节数
 */
extern ssize_t ul_writeo(int fd, void *ptr, size_t nbytes, int seconds);


/**
 * 检查socket在指定时间内是否可读
 *
 * @param fd socket句柄
 * @param seconds 超时值（s）
 * @return -1出错，0超时，>0可读
 */
extern int ul_sreadable(int fd, int seconds);


/**
 * 检查socket在指定时间内是否可写
 *
 * @param fd socket句柄
 * @param seconds 超时值（s）
 * @return -1出错，0超时，>0可写
 */
extern int ul_swriteable(int fd, int seconds);


//****************************************************************
// function : a  time-out version of connect().
// sockfd,saptr,socklen, as the same as connect
// secs, the ovet time, in second
//****************************************************************
/**
 * 带超时控制的connect()版本
 *
 * @param sockfd,saptr,secs 参数与connect()相同
 * @param secs 超时值（s）
 * @return 0成功，-1失败
 */
extern int ul_connecto(int sockfd, const struct sockaddr *saptr, socklen_t socklen, int secs);


/**
 * 从socket中读取一行字符串，直到缓冲区满，或者socket出错或者关闭
 *
 * @param fd 读取socket
 * @param vptr 数据接收缓冲区
 * @param maxlen 缓冲区大小
 * @return -1出错，否则为实际读取字节数（包括\n）
 */
ssize_t ul_readline(int fd, void *vptr, size_t maxlen);


ssize_t ul_readline_speed(int fd, void *vptr, size_t maxlen, Rline * rlin);
ssize_t ul_readline_speedo(int fd, void *vptr, size_t maxlen, Rline * rlin, int secs);


/**
 * 参数使用更加方便的connect()版本
 *
 * @param host 连接主机地址（IP或域名）
 * @param port 连接端口
 * @return -1失败，否则为连接socket
 */
extern int ul_tcpconnect(char *host, int port);

/**
 * 带超时控制的@ref ul_tcpconnect()
 *
 * @param host 连接主机地址（IP或域名）
 * @param port 连接端口
 * @param secs 超时值（s）
 * @return -1失败，否则为连接socket
 */
extern int ul_tcpconnecto(char *host, int port, int secs);

//*******************************************************
// Get IP address of one host.  Return the h_addr item.
// Return value:    0 :success;
//                  -1:hostname is error.
//                  -2:gethostbyname() call is error.
// Note:This function is not safe in MT-thread process
//******************************************************
extern int ul_gethostipbyname(const char *hostname, struct sockaddr_in *sin);




#endif  //__UL_NET_DEP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
