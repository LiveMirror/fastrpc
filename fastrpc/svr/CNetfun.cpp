#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>


#include "CNetfun.h"


long GetMillisecondTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;  // 返回豪秒
}

long GetMicrosecondTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    unsigned seconds = tv.tv_sec;
    seconds *= 1000 * 1000;
    seconds += tv.tv_usec;

    return seconds;
}

unsigned GenClientFlowNo(int offset,int sock)
{
    unsigned short o = (unsigned short)offset;
    unsigned short s = (unsigned short)sock;


    char buf[4]={0};
    unsigned idx = 0;

    memcpy(buf+idx,&o,sizeof(o));
    idx += sizeof(o);

    memcpy(buf+idx,&s,sizeof(s));

    unsigned flow = 0;
    memcpy((char*)(&flow),buf,sizeof(buf));

    //printf("offset %d,sock %d,gen %u\n,sizeof unsigned %u,buf %u,short %u\n",
    //    offset,sock,flow,sizeof(unsigned),sizeof(buf),sizeof(o));
    return flow;

}
int GetOffsetFromFlowNo(unsigned flow,int &offset,int &sock)
{
    unsigned idx = 0;

    unsigned short o;
    unsigned short s;

    memcpy((char*)(&o),((const char*)(&flow))+idx,sizeof(o));
    idx += sizeof(s);
    memcpy((char*)(&s),((const char*)(&flow))+idx,sizeof(s));

    offset = (int)o;
    sock = (int)s;
    //printf("flow %u,get offset %d,sock %d\n",flow,offset,sock);
    return 0;

}


int do_accept(int sockfd)
{
    int connfd = 0;

again:
    connfd = accept(sockfd,NULL,NULL);
    if (connfd < 0)
    {
        if (errno == ECONNABORTED)
        {
            goto again;
        }
        else
        {
            return -1;
        }
    }
    return connfd;
}

int do_setsocktonoblock(int sd)
{
    int val = 0;
    int flags, flag_t;
    while ((flags = fcntl(sd, F_GETFL, 0)) == -1) {
        if (errno == EINTR)
            continue;

        return -1;
    }
    flag_t = flags;
    if (flag_t & O_NONBLOCK) return 0;

    flags |= O_NONBLOCK;
    while ((val = fcntl(sd, F_SETFL, flags)) == -1) {
        if (errno == EINTR)
            continue;

        return -1;
    }
    return val;
}

int do_tcplisten(const char *strip,int port, int queue)
{
    int listenfd;
    const int on = 1;
    struct sockaddr_in soin;

    if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&soin, sizeof(soin));
    soin.sin_family = AF_INET;
    //soin.sin_addr.s_addr = inet_addr(strip);
    soin.sin_addr.s_addr = htonl(INADDR_ANY);
    soin.sin_port = htons((uint16_t)port);

    if (bind(listenfd, (struct sockaddr *) &soin, sizeof(soin)) < 0) {
        close(listenfd);
        return -1;
    }

    (queue <= 0) ? queue = 5 : queue;
    if (listen(listenfd, queue) < 0) {
        close(listenfd);
        return -1;
    }

    return listenfd;
}



/*
read_size    	返回已经读取的长度
buff_size       buff的缓冲长度
return 0        成功
return -1       失败，需要关闭socket
return -2       关闭
*/
int noblock_read_buff(int sock, char*& buff, unsigned *read_size, unsigned& buff_size)
{
    int ret;
	*read_size = 0;

    while (1) {
        ret = read(sock, buff+(*read_size), buff_size-(*read_size));
        //非堵塞读出现EINTR是正常现象, 再读一次即可
        if (-1 == ret && (EINTR == errno))
            continue;
        break;
    }
    if (ret < 0) {
        if (EAGAIN == errno) {
            //出现EAGAIN表示读的时候，网络底层栈里没数据,返回1等待下次读取
            //ul_writelog(UL_LOG_DEBUG, "read EAGAIN");
            return 0;
        }
        //读数据出错了
        //ul_writelog(UL_LOG_WARNING, "read buff fail [%m]");
        return -1;
    }
    if (0 == ret) {
        //ul_writelog(UL_LOG_DEBUG, "read 0, close it");
        //读到0一般是对端close主动断开
        return -2;
    }

    *read_size += ret;
    if (*read_size + 1024 > buff_size) {
        buff_size += 100*1024;
        buff = (char*)realloc(buff, buff_size);
        printf("realloc %d\n", buff_size);
    }
    return 0;
}

/*
buff				写的内容
buff_size			准备写入的大小
write_succ_size     返回真正写入的大小
return 0			成功
return -1			失败
return -2           关闭
*/
int noblock_write_buff(int sock, CRWCache& sendcache, unsigned *write_succ_size)
{
    int ret;

	*write_succ_size = 0;

    while (1) {
        char* buff = sendcache.data();
        unsigned buff_size = sendcache.data_len();
        ret = write(sock, buff+(*write_succ_size), buff_size-(*write_succ_size));
        if (-1 == ret && (EINTR == errno))
            continue;
        break;
    }
    if (ret < 0) {
        if ( EAGAIN == errno || errno == EINPROGRESS ) {
            return 0;
        }
        //写数据出错了
        //ul_writelog(UL_LOG_WARNING, "write buff fail [%m]");
        return -1;
    }
    if (0 == ret) { // 0一般是对端close主动断开
        return -2;
    }
    *write_succ_size += ret;

    return 0;
}

string ip2str(unsigned long addr)
{
    char buf[128]={0};
    const char* p = inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return p ? p : string();
}

int str2ip(const string &addr,unsigned long &addr_4byte)
{
    addr_4byte=0;

    struct in_addr sinaddr;
    int ret = inet_pton(AF_INET, addr.c_str(), &sinaddr);
    if ( ret>0 )
    {
        addr_4byte = sinaddr.s_addr;
        return 0;
    }
    return -1;
}

char *time2str(time_t t,char *szDateTime)
{
    if(!szDateTime) return NULL;

    struct tm stTm ;
    localtime_r((time_t*)&t, &stTm);
    sprintf(szDateTime, "%02d-%02d %02d:%02d:%02d",
        stTm.tm_mon+1, stTm.tm_mday,
        stTm.tm_hour, stTm.tm_min, stTm.tm_sec );

    return szDateTime;
}

int interaction_str2ip(const string &addr,unsigned long &addr_4byte)
{
    addr_4byte=0;

    struct in_addr sinaddr;
    int ret = inet_pton(AF_INET, addr.c_str(), &sinaddr);
    if ( ret>0 )
    {
        addr_4byte = sinaddr.s_addr;
        return 0;
    }
    return -1;
}

int interaction_sock2ip(const int sock,unsigned long &addr_4byte)
{
    addr_4byte = 0;

    struct sockaddr_in addr;
    unsigned len = sizeof(struct sockaddr_in);

    int ret = ::getsockname(sock, (struct sockaddr *)(&addr), &len);
    if (ret < 0)    return errno ? -errno : ret;

    addr_4byte = addr.sin_addr.s_addr;
    return 0;
}

string sock2peer(const int &sock)
{
    char peerip[32]={0};
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(struct sockaddr_in);

    int ret = getpeername(sock,(struct sockaddr *)&peeraddr,&len);
    if ( ret<0 )
    {
        string strErr(strerror(errno));
        return strErr;
    }
    inet_ntop(AF_INET,&peeraddr.sin_addr, peerip, sizeof(peerip));

    string strPeer = peerip;
    return strPeer;
}
int sock2peer(const int &sock,unsigned long &addr_4byte)
{
    char peerip[32]={0};
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(struct sockaddr_in);

    int ret = getpeername(sock,(struct sockaddr *)&peeraddr,&len);
    if ( ret<0 )
    {
        addr_4byte = 0;
        return -1;
    }

    addr_4byte = peeraddr.sin_addr.s_addr;
    return 0;
}

