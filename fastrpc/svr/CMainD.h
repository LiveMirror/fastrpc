#ifndef _SEARCH_MAIN_SVR_H_
#define _SEARCH_MAIN_SVR_H_

#include <fstream>
#include <string>

using namespace std;

/**
* @author feimat@baidu.com
*
* 
* <pre>
* 状态机处理主模块
*
* </pre>
**/


#include "Define.h"





class CMainAsyncSvr
{
public:
    static void *run(void *instance);


	CMainAsyncSvr();
    virtual ~CMainAsyncSvr(){}

	void MainLoop();
    virtual xCallbackObj *CreateAsyncObj(CDataBuf *item); // 业务实现

    CCallbackObjQueue mObjQueue;
	CMetaQueue<CDataBuf> mPending;

};


#endif 
