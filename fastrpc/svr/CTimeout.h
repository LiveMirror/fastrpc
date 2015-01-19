#ifndef _SVR_TIMEOUT_H_
#define _SVR_TIMEOUT_H_


#include <sys/types.h>
#include <sys/time.h>



/**
* @author feimat@baidu.com
*
* 
* <pre>
* 编程模型：Timeout类
* </pre>
**/

class CTimeout {
public:
    CTimeout() : mIntervalMs(0) { 
        mLastCall = CTimeout::NowMs();
    }
    virtual ~CTimeout() { }
    
    void SetTimeoutInterval(const int &intervalMs) {
        mIntervalMs = intervalMs;
    }

    int GetTimeElapsed() {
        return (NowMs() - mLastCall);
    }

    static long NowMs() {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (timeNow.tv_sec) * 1000 + timeNow.tv_usec / 1000;
    }

    void TimerExpired(const long &nowMs) {
        if ( mIntervalMs <= 0 || nowMs >= mLastCall + mIntervalMs) {
            Timeout();
            mLastCall = nowMs;
        }
    }
    
    virtual void Timeout() = 0;
protected:
    int	mIntervalMs;
private:
    long mLastCall;
};



#endif
