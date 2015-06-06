#include<map>
#include<sstream>
#include <sys/stat.h>
#include <strings.h>


#include "CMainD.h"
#include "CNetfun.h"
#include "Statistic.h"



using namespace std;




CMainAsyncSvr::CMainAsyncSvr()
{
}



void *CMainAsyncSvr::run(void *instance)
{
    pthread_t tid = pthread_self();
    LOG(LOG_ALL,"Info:CMainAsyncSvr thread create,pid=%d,tid=%u\n",getpid(),tid);

    CMainAsyncSvr *obj = (CMainAsyncSvr*)instance;
    obj->MainLoop();
    return NULL;
}

void CMainAsyncSvr::MainLoop()
{
	for(;;)
	{
        //////////////////////////////////////////////////////////////////////////
        timeval rr1;
        timeval rr2;
        gettimeofday(&rr1,NULL);

        int ret = 0;
        CDataBuf *item = NULL;

        item = mPending.dequeue_nowait();


        if ( item )
        {
            xCallbackObj *obj = NULL;

            if ( item->type == SVR_CliD_MainD )
            {
                unsigned ccd_flow = item->flow;


                obj = CreateAsyncObj(item);


                if ( obj )
                {
                    mObjQueue.Set(obj->flow,obj);
                    obj->ccd_flow = ccd_flow;
                }
            }
            else if ( item->type == SVR_BackD_MainD )
            {

                //LOG(LOG_ALL,"SVR_BackD_MainD mcd flow=%u\n",item->flow);


                unsigned mcd_flow = item->flow;
				if ( mcd_flow==0 )
				{
					LOG(LOG_ALL,"Error:Network error,IP=%s:%ld Queue size=%u\n",ip2str(item->destinfo._ip).c_str(),item->destinfo._port,mObjQueue.Size());
				}
				else
				{
					mObjQueue.Get(mcd_flow,&obj);
					if ( obj==NULL )
					{
						LOG(LOG_ALL,"Error:flow=%u has delete,Queue size=%u",mcd_flow,mObjQueue.Size());
                        item->Free();
                        delete item;
						continue;
					}
					ret = obj->HandleEvent(1,item);
					if ( ret==ASYNC_RET_FINISH || ret==ASYNC_RET_FAIL )
					{
                        LOG(LOG_ALL,"Info:Process item %s,Delete obj,flow=%u,cost %dmms\n",
                            (ret==ASYNC_RET_FINISH)?"succ":"fail",mcd_flow,obj->CostInfo());
						delete obj;
						obj = NULL;
					}
					else
					{
						mObjQueue.Set(mcd_flow,obj);
					}
				}
            }
        }

        gettimeofday(&rr2,NULL);STAT_ADD("DispatchMain1",0,&rr1,&rr2,1);
		mObjQueue.CheckTimeout();

        if ( item ) {
            item->Free();
            delete item;
        }else{
            usleep(1);
        }


        //////////////////////////////////////////////////////////////////////////
	} // for
	return;
}
