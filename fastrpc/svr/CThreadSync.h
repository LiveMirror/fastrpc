#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H

#include <string>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>


using namespace std;

enum THREAD_SYNC_ERROR
{
	MUTEX_ERROR_BASE = -1,  
	THREAD_RWLOCK_TIMEOUT = -2,
	THREAD_DEAD_LOCK = -3,
	THREAD_READ_LOCK_EXCEED = -4,
};

class CMutex 
{
public:
    CMutex(int kind);
    CMutex();
    ~CMutex();
    int lock();
    int trylock();
    int unlock();

    pthread_mutex_t* mutex(); 

protected:
    pthread_mutex_t mutex_;
};

template <class LLLOCK>
class Guard
{
public:
    Guard ( LLLOCK &m , bool block = true ): lock_( m )
    {
        result_ = block ? lock_.lock() : lock_.trylock();
    }

    ~Guard (void)
    {
        lock_.unlock();
    }

    bool locked( )
    {
        return (result_ == 0);
    }

private:
    LLLOCK &lock_;

    // Tracks if acquired the lock or failed.
    int result_;
};

typedef Guard< CMutex > MutexGuard;

class CRWLock
{
public:
    CRWLock()
    {
		int ret = 0;
		pthread_rwlockattr_t attr;
		pthread_rwlockattr_init(&attr);
		//pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
		pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_READER_NP);
		ret = pthread_rwlockattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
        ret = pthread_rwlock_init(&wrlock_,&attr);
        assert(ret==0);

        pthread_rwlockattr_destroy(&attr);
    }
    ~CRWLock()
    {
        pthread_rwlock_destroy(&wrlock_);
    }
    int ReadLock()
    {
        return pthread_rwlock_rdlock(&wrlock_);
    }

	int ReadTryLock(int millisecondTimeOut=1000)	// 1000∫¡√Î=1√Î
	{
		int millisecondWaitTime = 0;
		int ret = 0;

		do {
			ret = pthread_rwlock_tryrdlock(&wrlock_);
			if ( ret == EDEADLK ) return THREAD_DEAD_LOCK;
			if ( ret == EAGAIN ) return THREAD_READ_LOCK_EXCEED;

			usleep(100);
			millisecondWaitTime = millisecondWaitTime + 100;
		} 
		while( millisecondWaitTime < millisecondTimeOut );

		return ret;
	}

    int ReadUnLock()
    {
        return pthread_rwlock_unlock(&wrlock_);
    }

    int WriteLock()
    {
        return pthread_rwlock_wrlock(&wrlock_);
    }

    int WriteUnLock()
    {
        return pthread_rwlock_unlock(&wrlock_);
    }

	int TimeReadLock(unsigned microseconds)	// Œ¢√Î(1√Î=1000∫¡√Î==1000*1000Œ¢√Î)
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );

		unsigned seconds = tv.tv_sec;
		seconds *= 1000 * 1000;
		microseconds += seconds; 
		microseconds += tv.tv_usec;

		struct timespec ts;
		ts.tv_sec = microseconds / 1000000;
		ts.tv_nsec = (microseconds % 1000000) * 1000;

		int ret = pthread_rwlock_timedrdlock(&wrlock_,&ts);
		if ( ret==0 ) return 0;
		else if ( ret==ETIMEDOUT ) return THREAD_RWLOCK_TIMEOUT;
		else if ( ret==EDEADLK ) return THREAD_DEAD_LOCK;
		else if ( ret==EAGAIN ) return THREAD_READ_LOCK_EXCEED;
		return ret;
	}

	int TimeWriteLock(unsigned microseconds)
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );

		unsigned seconds = tv.tv_sec;
		seconds *= (1000 * 1000);
		microseconds += seconds; 
		microseconds += tv.tv_usec;

		struct timespec ts;
		ts.tv_sec = microseconds / 1000000;
		ts.tv_nsec = (microseconds % 1000000) * 1000;

		int ret = pthread_rwlock_timedwrlock(&wrlock_,&ts);
		if ( ret==0 ) return 0;
		else if ( ret==ETIMEDOUT ) return THREAD_RWLOCK_TIMEOUT;
		else if ( ret==EDEADLK ) return THREAD_DEAD_LOCK;
		else if ( ret==EAGAIN ) return THREAD_READ_LOCK_EXCEED;
		return ret;
	}

private:
    pthread_rwlock_t wrlock_;
};

class ConditionVariable 
{
private:
	pthread_cond_t _condition;

public:
	ConditionVariable() {
		pthread_cond_init(&_condition, NULL);
	}

	~ConditionVariable() {
		pthread_cond_destroy(&_condition);
	}

	void wait(CMutex &_mutex) {
		pthread_cond_wait(&_condition, _mutex.mutex());
	}

	// »Áπ˚≥¨ ±∑µªÿ true
	bool wait(CMutex &_mutex,unsigned microseconds) {

		struct timeval tv;
		gettimeofday( &tv, 0 );

		unsigned seconds = tv.tv_sec;
		seconds *= 1000 * 1000;
		microseconds += seconds; 
		microseconds += tv.tv_usec;

		struct timespec ts;
		ts.tv_sec = microseconds / 1000000;
		ts.tv_nsec = (microseconds % 1000000) * 1000;

		int result = pthread_cond_timedwait(&_condition,_mutex.mutex(),&ts);
		return (result == ETIMEDOUT);
	}

	void notifyOne() {
		pthread_cond_signal(&_condition);
	}

	void notifyAll() {
		pthread_cond_broadcast(&_condition);
	}
};

#endif

