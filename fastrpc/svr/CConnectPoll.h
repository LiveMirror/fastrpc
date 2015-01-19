#ifndef _SVR_CNET_CONNECT_POOL_H_
#define _SVR_CNET_CONNECT_POOL_H_

#include <map>
#include <vector>

using namespace std;

/**
* @author feimat@baidu.com
*
*
* <pre>
* ip 和 handle 的对应关系
* </pre>
**/



#include "CNetfun.h"



typedef unsigned long long ull;

/*
    在ClientD中保存[flow]和[offset]的对应关系，从而可以操作ependingpool
*/

class CFlow2Offset
{
public:
    CFlow2Offset(){
        _flow=0;
        pthread_mutex_init(&mutex, NULL);
    }
    ~CFlow2Offset(){
        pthread_mutex_destroy(&mutex);
    }

    int Add(unsigned flowno,int offset) {
        pthread_mutex_lock(&mutex);
        _f2o[flowno]=offset;
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    int Del(unsigned flowno) {
        pthread_mutex_lock(&mutex);
        map<unsigned,int>::iterator it = _f2o.find(flowno);
        if ( it!=_f2o.end() )
        {
            _f2o.erase(it);
        }
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    int GetOffset(unsigned flowno) {
        pthread_mutex_lock(&mutex);
        map<unsigned,int>::iterator it = _f2o.find(flowno);
        int ret = -1;
        if ( it != _f2o.end() )
        {
            ret = (it->second);
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    int GetSize() {
        //pthread_mutex_lock(&mutex);
        //int ret = 0;
        //ret = _f2o.size();
        //pthread_mutex_unlock(&mutex);
        return _f2o.size();
    }

    int GetNextSock(int& random) {
        pthread_mutex_lock(&mutex);
        map<unsigned,int>::iterator it = _f2o.upper_bound(random);
        if (it != _f2o.end()) {
            random = it->first;
        }
        else {
            it = _f2o.upper_bound(0);
            random = it->first;
        }
        int ret = random;
        pthread_mutex_unlock(&mutex);
        return ret;
    }

public:
    unsigned _flow;
    map<unsigned,int> _f2o;
    pthread_mutex_t mutex;
};

/*
    在BackD中保存 [ip:port] 对应 ependingpool handle的对应关系,handle里面能拿到sock和offset，操作ependingpool
*/
class CIp2Handle
{
public:
	CIp2Handle(){}
	~CIp2Handle(){}

	unsigned Key(const string &str_ip,const int &port)
	{
		unsigned long ip = 0;
		interaction_str2ip(str_ip,ip);

		unsigned mip = (ip>>16) + (port<<16);

		return mip;
	}
	int GetHandle(const unsigned &key_ip_port)
	{
		map<unsigned,int>::iterator it = map_ip_handle.find(key_ip_port);
		if ( it==map_ip_handle.end() )
		{
			return -1;
		}
		return (it->second);
	}


	int DelHandel(const unsigned &key_ip_port)
	{
		map<unsigned,int>::iterator it = map_ip_handle.find(key_ip_port);
		if ( it!=map_ip_handle.end() )
		{
            // 保存一致性，我们辅助关了
            close((it->second));
			map_ip_handle.erase(it);
		}

		return 0;

	}

	int AddHandle(const unsigned &key_ip_port,const int &sock)
	{
		map_ip_handle[key_ip_port] = sock;
		return 0;
	}

private:
	map<unsigned,int> map_ip_handle;
};





#endif
