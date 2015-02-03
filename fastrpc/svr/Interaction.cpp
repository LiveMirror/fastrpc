 #include <arpa/inet.h>


#include "Interaction.h"
#include "CNetfun.h"
#include "CSvr.h"

using namespace std;

int CliD_Pool(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned ip,unsigned short port)
{
    int ret = p_svr->_pool->PushDataBuf(p_svr,clid_flow,data,len,ip,port);
    if ( ret!=0 ) LOG(LOG_ALL,"Error:flow=%u exist,replace it\n",clid_flow);
    return 0;
}

int CliD_MainD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned ip,unsigned short port)
{
    CDataBuf *dataObj = new CDataBuf();
    dataObj->Copy(data,len,clid_flow);
    dataObj->destinfo._ip = ip;
    dataObj->destinfo._port = port;
    dataObj->type = SVR_CliD_MainD;

    p_svr->_main_d->mPending.enqueue(dataObj);
    return 0;
}

string GetBody(const char *data,const unsigned &len,unsigned &bodylen)
{
    char *body = (char*)strstr(data,"\r\n\r\n");
    if ( body==NULL ) return "";

    unsigned headerlen = (body+4-data);
    if ( headerlen>1024 ) {
        // 头部有问题
        return "";
    }
    bodylen = len - headerlen;

    return string(body+4,bodylen);
}

int GetFlowFromHeader(const string &header,unsigned &flow)
{
    string::size_type pos;
    string::size_type posend;

    string sflow;

    pos = header.find("FlowNo:");
    if ( pos==string::npos ) return -1;
    else {
        posend = header.find("\r\n",pos);
        if ( posend==string::npos ) return -1;
        else
        {
            sflow = header.substr(pos+8,posend-pos-8);//1是空格
            flow = atoi(sflow.c_str());
        }
    }

    //LOG(LOG_ALL,"Header\n%s\nGET FlowNo=%u[%s]\n",header.c_str(),flow,sflow.c_str());
    return 0;
}

int BackD_MainD(char *data,unsigned len,unsigned backip,unsigned short backport,int obj_type)
{
    return 0;
}

int MainD_CliD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len,unsigned _op /* = SEND_NORMAL */)
{
    //CDataBuf *dataObj = new CDataBuf();

    //dataObj->Copy(data,len,clid_flow);
    //dataObj->oper = _op;

    //p_svr->_client_d->mPending.enqueue(dataObj);

    CClientNetSvr &cli = (  *((CClientNetSvr*)p_svr->_client_d) );
    // 必须根据flow_no拿到offset
    unsigned cd_flow = clid_flow;
    int offset = cli.sp->cliF2O->GetOffset(cd_flow);

    if ( offset==-1 )  // socket已经关闭或者超时了
    {
        return -1;
    }

    if ( data  && len > 0)
    {
        void *arg;
        int ret = cli.pool.fetch_handle_arg(offset,&arg);
        if ( ret != 0 )
        {
            LOG(LOG_ALL,"Error:pool fetch_handle_arg ret %d,cdflow %u,offset %d\n",ret,cd_flow,offset);
        }
        else
        {
            CClientSocketReq *psock_req = (CClientSocketReq*)(arg);
            if ( psock_req!=NULL )
            {
                psock_req->sendcache.extern_lock();
                psock_req->sendcache.append_nolock(data,len);
                ret = cli.pool.write_reset_item(offset);
                if ( ret!= 0 )
                {
                    printf("write reset item fail\n");
                    LOG(LOG_ALL,"Error:pool write_reset_item ret %d,cd_flow %u,offset %d\n",ret,cd_flow,offset);
                }
                if ( _op == SEND_AND_CLOSE ) psock_req->keep_live = false;
                else psock_req->keep_live = true;
                psock_req->sendcache.extern_unlock();
            }
            else
            {
                LOG(LOG_ALL,"Error:cd_flow %u,offset %d fetch_handle_arg NULL\n",cd_flow,offset);
            }
        }
    }
    else {
        cli.pool.reset_item(offset,false);
    }
    return 0;
}

int Pool_CliD(CASyncSvr* p_svr,unsigned clid_flow,char *data,unsigned len)
{
    return MainD_CliD(p_svr,clid_flow,data,len);
}

int MainD_BackD(unsigned flow,char *data,unsigned len,const char *strip,unsigned short port)
{

    return 0;
}





