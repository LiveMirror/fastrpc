// mysock.h
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#define BLOCK_SIZE 1024 
#define MYSOCK_TIMEOUT_ERR 0
#define MYSOCK_CLOSED_ERR  -1

int listen_socket(const char* bind_ip, unsigned long port);
int safe_recv(int sd, char* pBuf, int size, int timeout);
int safe_send(int sd, const char* pBuf, int size, int timeout);
int connect_to(const char* ip, unsigned int port);
int safe_close(int conn);
int _my_inet_ntoa(struct in_addr in, char* ip_addr);

