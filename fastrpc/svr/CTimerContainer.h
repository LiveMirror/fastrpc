#ifndef _STORAGE_CONTAINER_TIMER_H_
#define _STORAGE_CONTAINER_TIMER_H_

#include <sys/time.h>
#include <map>
#include "CCallbackObj.h"

using namespace std;


/**
* @author feimat@baidu.com
*
* 
* <pre>
* 编程模型：异步回调，状态机的管理器
* </pre>
**/




class CCallbackObjQueue
{
public:
	CCallbackObjQueue(){};
	virtual ~CCallbackObjQueue(){};
	//
	// 安装timer
	// return 0		成功
	//	      <0	失败
	//
	int Set(const unsigned &flow,xCallbackObj *obj,time_t __mIntervalSecond = 120 /* 单位秒 */);
	
	//
	// 获得timer对应的数据，并且卸载timer
	// return 0		成功
	//        <0    不存在
	//
	int Get(const unsigned &flow,xCallbackObj** obj);

    unsigned Size() {
        return objQueue.size();
    }

	void CheckTimeout();

protected:
	map<unsigned,xCallbackObj*> objQueue;
};




#endif
