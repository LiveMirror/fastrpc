#ifndef _SVR_CNET_FUN_CLIENT_H_
#define _SVR_CNET_FUN_CLIENT_H_


#include <vector>
#include <errno.h>
#include <string>
#include <sys/time.h>
#include "CRWCache.h"


using namespace std;


/**
* @author feimat@baidu.com
*
*
* <pre>
* 网络读写函数
* </pre>
**/




const unsigned CLIENT_COMPLETE_MAX_BUFFER = (1*1024*1024); // 接受缓冲区初始化大小
const unsigned BACK_COMPLETE_MAX_BUFFER = (1);   // 接收后台最大的包



int do_accept(int sockfd);
int do_setsocktonoblock(int sd);
int do_tcplisten(const char *strip,int port, int queue);



string sock2peer(const int &sock);
int sock2peer(const int &sock,unsigned long &addr_4byte);
string ip2str(unsigned long addr);
int str2ip(const string &addr,unsigned long &addr_4byte);
int interaction_str2ip(const string &addr,unsigned long &addr_4byte);
int interaction_sock2ip(const int sock,unsigned long &addr_4byte);
char *time2str(time_t t,char *szDateTime);


unsigned GenClientFlowNo(int offset,int sock);
int GetOffsetFromFlowNo(unsigned flow,int &offset,int &sock);

long GetMillisecondTime();
long GetMicrosecondTime();

/*
read_size    	返回已经读取的长度
buff_size       buff的缓冲长度
return 0        成功
return -1       失败，需要关闭socket
return -2       关闭
*/
int noblock_read_buff(int sock, char*& buff, unsigned *read_size, unsigned& buff_size);

/*
buff				写的内容
buff_size			准备写入的大小
write_succ_size     返回真正写入的大小
return 0			成功
return -1			失败
return -2       关闭
*/
int noblock_write_buff(int sock, CRWCache& sendcache, unsigned *write_succ_size);







#endif

