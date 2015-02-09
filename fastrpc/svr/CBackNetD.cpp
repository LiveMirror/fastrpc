#include <iostream>
#include <assert.h>
#include <sys/time.h>

#include "CBackNetD.h"
#include "CNetfun.h"
#include "Statistic.h"



using namespace std;

char back_recv_buff[BACK_COMPLETE_MAX_BUFFER];
unsigned back_recv_len = BACK_COMPLETE_MAX_BUFFER;



void DebugSockInfo(const char *info,int sock,int ret,unsigned rws,unsigned dlen)
{
    return;
    string strIp = sock2peer(sock);

    LOG(LOG_ALL,"AAA|%s|sock|%d|%s|errno|%d|%s|ret|%d|rw_size|%u|data_len|%u\n",
        strIp.c_str(),sock,info,errno,strerror(errno),ret,rws,dlen);
}


/*
* <0 写失败，会触发SOCK_CLEAR事件
* =0 数据全部写完毕，并将sock重新放入epoll中进行监听读请求(相当于长连接)
* =1 数据写成功，但没有全部写完，放回epoll等待下次再监听到可写状态

*/

int write_back_svr(ependingpool* ep, int sock, void **arg)
{
    unsigned write_succ_size = 0;
	int ret = -1;

	// 可写，直接从缓存在发送
	CBackSocketReq *psock_req = (CBackSocketReq*)(*arg);

	if (psock_req->sendcache.data_len() == 0 )
	{
        DebugSockInfo("write 0",sock,0,0,0);
		return 0; // 没数据发送，监听recv读。
	}



    unsigned haswrite=0;
    unsigned i = 0;
    while ( i<30 ) {
        ret = noblock_write_buff(sock,psock_req->sendcache,&write_succ_size);
        if ( ret!=0 ){  return -1;    }

        if ( write_succ_size>0 )  psock_req->sendcache.skip(write_succ_size);

        if ( write_succ_size>0 ) haswrite += write_succ_size;
        if ( ret==0 && errno==EAGAIN && write_succ_size==0 ) {
            //LOG(LOG_ALL,"EAGAIN|sock %d|i|%u|%d|%s|has write|%u\n",sock,i,errno,strerror(errno),haswrite);
            i=2000;
            continue;
        }
        //LOG(LOG_ALL,"INFO|sock %d|i|%u|%d|%s|has write|%u\n",sock,i,errno,strerror(errno),haswrite);

        if ( psock_req->sendcache.data_len() == 0 ) return 0;
        i++;
    }







	if ( psock_req->sendcache.data_len() == 0 )
	{
		return 0;// 数据全部写完毕，并将sock重新放入epoll中进行监听读请求(相当于长连接)
	}
	// 没有写完
	return 1;
}
/*
* <0 读失败，会触发SOCK_CLEAR事件
* =0 数据全部读取完毕，并将sock放入就绪队列中
* =1 数据读成功，但没有全部读取完毕，需要再次进行监听，sock会被放回epoll中

*/
int read_back_svr(ependingpool* ep, int sock, void **arg)
{
	//int ret = 0;
	//unsigned read_size = 0;
    //CBackSocketReq *psock_req = (CBackSocketReq*)(*arg);



    //unsigned hasread=0;
    //unsigned i = 0;
    //while ( i<100 ) {
    //    ret = noblock_read_buff(sock,back_recv_buff,&read_size,back_recv_len);
    //    if ( ret!=0 ){  return -1;    }
    //    if ( read_size>0 ) psock_req->recvcache.append(back_recv_buff,(unsigned)read_size);

    //    if ( read_size>0 ) hasread += read_size;
    //    if ( ret==0 && errno==EAGAIN && read_size==0 ) {
    //        //LOG(LOG_ALL,"EAGAIN|sock %d|i|%u|%d|%s|has read|%u\n",sock,i,errno,strerror(errno),hasread);
    //        i=2000000;
    //        continue;
    //    }
    //    //LOG(LOG_ALL,"INFO|sock %d|i|%u|%d|%s|has read|%u\n",sock,i,errno,strerror(errno),hasread);
    //    i++;
    //}





    //int ic = 0;
    //unsigned one_len = 0;
    //do {
    //    ret = psock_req->check_recv_one(back_recv_buff,one_len);
    //    if ( ret>0 ) {
    //        unsigned long ip;
    //        interaction_sock2ip(sock,ip);
    //        BackD_MainD(back_recv_buff,one_len);
    //        //DebugSockInfo("read 2 get one",sock,ret,one_len,psock_req->recvcache.data_len());
    //        if ( psock_req->recvcache.data_len()==0 ) ret = 0;
    //    }
    //    ic++;
    //} while( ret>0 && ic<100 );

	//if ( ret >= 0 ) {
	//	// 这个包还没接收完，需要继续读
	//	return 1;
	//}
	// 其他情况少于0，读失败，触发SOCK_CLEAR事件
	return -1;
}

int init_back_svr(ependingpool* ep, int sock, void **arg)
{
	CBackSocketReq *sock_req = new CBackSocketReq();
	if (sock_req == NULL)
	{
		return -1;
	}
	//将指针赋给sock捆绑的指针
	*arg = sock_req;
	return 0;
}

int clear_back_svr(ependingpool* ep, int sock, void **arg)
{
	CBackSocketReq *req = (CBackSocketReq*)(*arg);
	if ( req!=NULL ){
		delete req;
		req = NULL;

        string strip = sock2peer(sock);
        LOG(LOG_ALL,"BACK|%s close|sock:%d\n",strip.c_str(),sock);
	}
	return 0;
}


CBackNetSvr::CBackNetSvr(const int &_max,int _sock_num/* =10000 */,int _queue_len/* =10000 */)
{
    max_conn = _max;

    pool_sock_num = _sock_num;
    pool_queue_len = _queue_len;


    pool.set_epoll_timeo(1);
    //设置连接超时时间(秒), 默认为1s
    pool.set_conn_timeo(10);
    //设置读超时时间(秒), 默认为1s
    pool.set_read_timeo(10);
    //设置写超时时间(秒), 默认为1s
    pool.set_write_timeo(10);

    //设置可存储socket的数量
    pool.set_sock_num(pool_sock_num);
    //设置已就绪队列的长度
    pool.set_queue_len(pool_sock_num);
}

void CBackNetSvr::Start()
{
    //初始化与sock捆绑的数据
    pool.set_event_callback(ependingpool::SOCK_INIT, init_back_svr);
    //释放与sock捆绑的数据
    pool.set_event_callback(ependingpool::SOCK_CLEAR, clear_back_svr);
    //使用非堵塞读写方式，模拟异步读写
    pool.set_event_callback(ependingpool::SOCK_READ, read_back_svr);
    pool.set_event_callback(ependingpool::SOCK_WRITE, write_back_svr);
    //数据处理做的事情很少没必要使用fetch进行多线程处理,使用todo单线程处理就可以了
    //pool.set_event_callback(ependingpool::SOCK_TODO, todo_back_svr);
    return;
}

void CBackNetSvr::MainLoop()
{
    timeval rr1;
    timeval rr2;
    int ret = 0;


    while (pool.is_run())
    {
        gettimeofday(&rr1,NULL);
        pool.check_item();
        gettimeofday(&rr2,NULL);STAT_ADD("back_check",0,&rr1,&rr2,1);

        CDataBuf *item = NULL;


        //LOG(LOG_ALL,"BACK|queue %u\n",mPending.size());

        gettimeofday(&rr1,NULL);
        for(unsigned q_i=0; q_i<1280; q_i++)
        {
            item = mPending.dequeue_nowait();
            if ( item )
            {
                /*
                    ip2back_flow[key_ip_port] = back_flow
                */
                unsigned key_ip_port = (item->flow); // MainSvr过来的flow就是ip：port了
				unsigned back_flow = ip2back_flow.GetHandle(key_ip_port); // 不一定存在的

                int offset = -1;
                int sock = -1;

                if ( back_flow>0 ) {
                    GetOffsetFromFlowNo(back_flow,offset,sock);
                }


                if ( item->data == NULL && item->len == 0 ) // 关闭连接
                {
					if (sock>0) {
						pool.reset_item(offset,false);
					}

					ip2back_flow.DelHandel(key_ip_port);
                    item->Free();
                    delete item;
					continue;
                }
				void *arg;
				ret = pool.fetch_handle_arg(offset,&arg);

				// 如果还没连接，那么就连接
				if ( sock<0 || ret!=0 )
				{
					if ( sock>0 ) {
						// 不存在pool，因为超时了，这时候需要关闭
						ip2back_flow.DelHandel(key_ip_port);
					}
					// 连接
					CSocketTCP socket;
					socket.create();
					socket.set_nonblock();
					ret = socket.connect(item->destinfo._ip,item->destinfo._port);
					if ((ret != 0 )&& (ret != -EWOULDBLOCK) && (ret != -EINPROGRESS))
					{
						// 失败响应
						LOG(LOG_ALL,"connect failed %s:%d.\n",ip2str(item->destinfo._ip).c_str(),item->destinfo._port);
                        BackD_MainD(NULL,0,item->destinfo._ip,item->destinfo._port);
                        item->Free();
                        delete item;

					}
					else
					{
                        LOG(LOG_ALL,"BACK|connect %s:%d succ\n",ip2str(item->destinfo._ip).c_str(),item->destinfo._port);

                        int aoff = 0;
						sock = socket.fd();
						socket.detach();

                        CBackSocketReq *req = new CBackSocketReq();
                        assert(req!=NULL);
                        req->sendcache.append(item->data,item->len);

						// 0表示监听读操作，非0 表示监听写操作
						ret = pool.insert_item_sock(sock,aoff,req,-1);
						if ( ret<0 ) {
							//响应
                            LOG(LOG_ALL,"insert_item_sock failed %s:%d.\n",ip2str(item->destinfo._ip).c_str(),item->destinfo._port);
                            BackD_MainD(NULL,0,item->destinfo._ip,item->destinfo._port);
						} else{
                            // 产生flow_dcc
                            unsigned bflow = GenClientFlowNo(aoff,sock);
							ip2back_flow.AddHandle(key_ip_port,bflow);
						}// if ret<0


					}// else if connect ok
				}
				else
				{
					//已经连接
                    CBackSocketReq *psock_req = NULL;
                    psock_req = (CBackSocketReq*)(arg);
                    // 增加到发送cache后面，再发送，没发送完，要增加 EPOLLOUT
                    // 以后优化 if ( psock_req->sendcache.data_len() == 0 ){}

                    if ( psock_req!= NULL )
                    {
                        psock_req->sendcache.append(item->data,item->len);
                        // 不会直接调用回调函数，只是设置为EPOLLOUT，没监听EPOLLIN了
                        // 下次事件到了，才直接调用回调函数写
                        ret = pool.write_reset_item(offset);
                        if ( ret!= 0 )
                        {
                            LOG(LOG_ALL,"Error:pool write_reset_item ret %d,sock %d\n",ret,sock);
                            ip2back_flow.DelHandel(key_ip_port);
                        }
                    }else
                    {
                        close(sock);
                        LOG(LOG_ALL,"Error:offset %d,fetch_handle_arg NULL,sock %d\n",offset,sock);
                    }


				}

                item->Free();
                delete item;
            } // if item

        } // item for

        gettimeofday(&rr2,NULL);STAT_ADD("back_dis",0,&rr1,&rr2,1);
    }// main for
}

void *CBackNetSvr::run(void *instance)
{
    CBackNetSvr *client_svr = (CBackNetSvr*)instance;
    pthread_t tid = pthread_self();
    LOG(LOG_ALL,"Info:IndexD CBackNetSvr detach thread create,pid=%d,tid=%u\n",getpid(),tid);

    client_svr->MainLoop();

    return NULL;
}
