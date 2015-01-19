/**
 * @file
 * @brief DNS客户端函数
 */
#ifndef __UDPDNS_H__
#define __UDPDNS_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/in.h>


#define UDP_DNS_PORT              1280
#define SITE_NAME_LEN             64
#define UDP_MSGLEN                128

// struct 
struct host_struct {
	char domain[SITE_NAME_LEN + 1];
	char ip[INET_ADDRSTRLEN + 1];
};

#include "dep/dep_udpdns.h"

/**
 *  建立UDP服务端套节字(function include "socket and bind")
 *  
 *  @param[in]  udp_port 端口号
 *  @param[out] 无
 *  @return 套结字
 * - >=0 UDP服务端套节字
 * - -1  失败
 */
extern int ul_udplisten(int udp_port);

/**
 *  建立UDP客户端套节字
 *  
 *  @param[in]  host     服务器IP地址
 *  @param[in]  udp_port 服务端端口号
 *  @param[out] servaddr 服务器网络地址
 *  @return 套结字
 * - >=0 UDP客户端套节字
 * - -1  失败
 */
extern int ul_udpconnect(char *host, int udp_port, struct sockaddr_in *servaddr);

/**
 *  发送并接收UDP消息，支持超时
 *  
 *  @param[in]  sockfd     UDP客户端套节字
 *  @param[in]  pservaddr  服务器网络地址
 *  @param[in]  datamsg    待发送的消息
 *  @param[in]  datalen    消息长度
 *  @param[in]  sec        超时时间的秒部分
 *  @param[in]  usec       超时时间的微秒部分
 *  @param[out] datamsg    存放接收到的数据
 *  @return 接收到的数据长度
 * - >=0 接收到的数据长度
 * - -1  失败
 */
extern int send_recv_msgo(int sockfd, struct sockaddr *pservaddr, char *datamsg, int datalen,
						  int sec, int usec);

/**
 *  从DNS服务器获取域名对应的IP，支持超时
 *  
 *  @param[in]  sockfd    UDP客户端套节字
 *  @param[in]  servaddr  服务器网络地址
 *  @param[in]  domain    待解析的域名
 *  @param[in]  sec       超时时间的秒部分
 *  @param[in]  usec      超时时间的微秒部分
 *  @param[out] ip        DNS服务器返回的解析出的IP
 *  @return 返回结果
 * - 0   成功
 * - -1  失败
 */
extern int gethostwithfd_r_o(int sockfd, struct sockaddr *servaddr, char *domain, char *ip, int sec,
							 int usec);

/**
 *  从DNS服务器获取域名对应的IP，支持超时
 *  
 *  @param[in]  dnshost    DNS服务器名
 *  @param[in]  port       服务端口
 *  @param[in]  domain     待解析的域名
 *  @param[in]  sec        超时时间的秒部分
 *  @param[in]  usec       待解析的域名
 *  @param[out] ip         超时时间的微秒部分
 *  @return 返回结果
 * - 0   成功
 * - -1  失败
 */
extern int gethostipbyname_r_o(char *dnshost, int port, char *domain, char *ip, int sec, int usec);

#endif
