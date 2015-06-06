#include "CTimerContainer.h"


int CCallbackObjQueue::Set(const unsigned &flow,xCallbackObj *obj,time_t __mIntervalSecond /* = 120  */)
{
	time_t cur_time = time(NULL);

    obj->access_time = cur_time;
    
    if ( obj->inter_timeout==0 ) obj->inter_timeout = __mIntervalSecond;

	objQueue[flow] = obj;

	return 0;
}

int CCallbackObjQueue::Get(const unsigned &flow,xCallbackObj** obj)
{
	map<unsigned,xCallbackObj*>::iterator it = objQueue.find(flow);
	if ( it == objQueue.end() )
	{
		*obj = NULL;
		return -1;
	}
	*obj = it->second;
	objQueue.erase(it);
	return 0;
}

void CCallbackObjQueue::CheckTimeout()
{
    time_t sub_time;
	time_t cur_time = time(NULL);
	map<unsigned,xCallbackObj*>::iterator it = objQueue.begin();

	while ( it != objQueue.end() )
	{
		xCallbackObj *obj  = it->second;
        sub_time = cur_time - obj->start_time;

        if ( sub_time > (obj->inter_timeout*2) ) // 大于2倍就肯定超时了
        {
           //printf("Info:flow=%u,curtime=%ld,start time=%ld,inter timeout=%ld\n",obj->flow,cur_time,obj->start_time,obj->inter_timeout*2);
           obj->TimeOut(HANDLE_CODE_TIME_OUT,NULL);
           delete obj;
           obj = NULL;

           objQueue.erase(it++);
           continue;
        }

		if ( sub_time>obj->inter_timeout && obj->inter_timeout_able )
		{
            //printf("Info:2 flow=%u,curtime=%ld,start time=%ld,inter timeout=%ld\n",obj->flow,cur_time,obj->start_time,obj->inter_timeout);
            obj->TimeOut(HANDLE_CODE_FORCE_FINISH,NULL);
            obj->inter_timeout_able = false;
            
            // add 2013-02-07
            delete obj;
            obj = NULL;

            objQueue.erase(it++);
            continue;

		}
		
		it++;
	}
}

