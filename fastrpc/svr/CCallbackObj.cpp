#include <sstream>

#include "CThreadSync.h"
#include "CCallbackObj.h"
#include <stdio.h>


unsigned _flow = 0;
CMutex flow_mutex;

unsigned FlowNo() {
	MutexGuard g(flow_mutex);

	_flow = ((_flow == 0 || _flow == 0xffffffffUL ) ? 1 : _flow+1);
	return _flow;
}

unsigned WhatTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;  // ∑µªÿ∫¿√Î
}

int xCallbackObj::CostInfo(string &info)
{
    if ( stepTimeV.empty() ) {
        info = "cost|0|\n";
        return 0;
    }
    char szInfo[256]={0};
    unsigned totalc = 0;

    totalc = stepTimeV[stepTimeV.size()-1].nowTime - stepTimeV[0].nowTime;

    int pos = sprintf(szInfo,"cost|%u|step",totalc);
    for(unsigned i=0; i<stepTimeV.size()-1; i++)
    {
        pos = sprintf(szInfo+pos,"|%u",stepTimeV[i+1].nowTime - stepTimeV[i].nowTime);
    }
    info = szInfo;
    return totalc;
}

int xCallbackObj::CostInfo()
{
    if ( stepTimeV.empty() ) {
        return 0;
    }
    return stepTimeV[stepTimeV.size()-1].nowTime - stepTimeV[0].nowTime;
}
