#ifndef _SVR_CliD_MainD_BackD_INTERACTION_H_
#define _SVR_CliD_MainD_BackD_INTERACTION_H_



#include <string>
#include <string.h>
#include <assert.h>

using namespace std;

/**
*
* <pre>
* 异步编程的 ClientD,MainD,BackD交互模块
*
* </pre>
**/

class CASyncSvr;

enum
{
    SVR_CliD_MainD = 1,
    SVR_BackD_MainD = 2,
    SVR_NOTIFY_MainD_TIMEOUT = 3,

	ASYNC_RET_FINISH = 4,
	ASYNC_RET_FAIL = 5,
};

enum
{
    SEND_NORMAL = 1,
    SEND_AND_CLOSE = 2,
};

struct TDestInfo {
    unsigned int _ip;
    unsigned short  _port;
};

struct CDataBuf {
    char *data;
    unsigned len;
    unsigned flow;

    unsigned oper;
    unsigned type;
    TDestInfo destinfo;

    CDataBuf(){
        data = NULL;
        len = 0;
        flow = 0;
        type = SVR_CliD_MainD;
        oper = SEND_AND_CLOSE;
        memset(&destinfo,0,sizeof(TDestInfo));
    }
    ~CDataBuf() {
		data = NULL;
		len = 0;
		flow = 0;
		type = SVR_CliD_MainD;
		memset(&destinfo,0,sizeof(TDestInfo));
    }

    bool Empty() {
        return ( (data==NULL)||(len==0) );
    }

    void Free() {
        if ( data!=NULL )
        {
            delete []data;
            data = NULL;
        }
        len = 0;
    }

    void Copy(const char *_in_data,unsigned _in_len,unsigned _flow)
    {
        if ( _in_data )
        {
            Free();

            len = _in_len;
            data = new char[len];
            assert(data!=NULL);

            memcpy(data,_in_data,len);
        }
        flow = _flow;
    }
};



// 同步编程函数
int CliD_Pool(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned ip,unsigned short port);
int Pool_CliD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len);

// 异步编程函数
int CliD_MainD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned ip,unsigned short port);
int MainD_CliD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned _op = SEND_NORMAL);
int MainD_BackD(unsigned flow,char *data,unsigned len,const char *strip,unsigned short port);
int BackD_MainD(char *data,unsigned len,unsigned backip=0,unsigned short backport=0,int obj_type=SVR_BackD_MainD);

// 如果后端也是Http协议，从Header中拿到FlowNo
int GetFlowFromHeader(const string &header,unsigned &flow);
string GetBody(const char *data,const unsigned &len,unsigned &bodylen); // 返回body的长度

#endif

