#include <stdlib.h>
#include <assert.h>

#include "CThreadSync.h"


CMutex::CMutex(int kind)
{
	pthread_mutexattr_t  attr;
	pthread_mutexattr_init(&attr);
	//attr.__mutexkind = kind;

	pthread_mutex_init(&mutex_, &attr);
}

CMutex::CMutex()
{
	pthread_mutexattr_t  attr;
	pthread_mutexattr_init(&attr);
	//attr.__mutexkind = PTHREAD_MUTEX_TIMED_NP;

	pthread_mutex_init(&mutex_, &attr);
}

CMutex::~CMutex()
{
	pthread_mutex_destroy(&mutex_);
}

int CMutex::lock()
{
	if (pthread_mutex_lock(&mutex_) != 0)
	{
		return MUTEX_ERROR_BASE;        
	}
	return 0;
}

int CMutex::trylock()
{
	if (pthread_mutex_trylock(&mutex_) != 0)
	{
		return MUTEX_ERROR_BASE;        
	}
	return 0;
}

int CMutex::unlock()
{
	if (pthread_mutex_unlock(&mutex_) != 0)
	{
		return MUTEX_ERROR_BASE;        
	}

	return 0;
}

pthread_mutex_t* CMutex::mutex()
{
	return &mutex_;
}
