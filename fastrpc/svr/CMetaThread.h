#ifndef _SVR_CTHREAD2_H_
#define _SVR_CTHREAD2_H_

#include <cassert>
#include <pthread.h>
#include <semaphore.h>



/**
* @author feimat@feimat.com
*
* 
* <pre>
* 线程启动，停止辅组类
* </pre>
**/

class CMetaThread 
{
public:
	typedef void *(*thread_start_t)(void *);

	CMetaThread(): thread(NO_THREAD)
	{
		pthread_mutex_init(&mutex, NULL);
		sem_init(&_sem, 0 ,0);
	}

	~CMetaThread()
	{
		pthread_mutex_destroy(&mutex);

        if (thread != NO_THREAD) {
            int status = pthread_cancel(thread);
            assert(status == 0);
        }

		sem_destroy(&_sem);
	}
	void lock()
	{
		int status = pthread_mutex_lock(&mutex);
		assert(status == 0);
	}
	void unlock()
	{
		int status = pthread_mutex_unlock(&mutex);
		assert(status == 0);
       	}
	void wakeup()
	{
		int status = sem_post(&_sem);
		assert(status == 0);
	}
	void sleep()
	{
		int status = sem_wait(&_sem);
		assert(status == 0);
	}
	void start(thread_start_t func, void *arg)
	{
		int status;
		status = pthread_create(&thread, NULL, func, arg);
		assert(status == 0);
	}
	void stop()
	{
		int status = pthread_cancel(thread);
		assert(status == 0);
	}
	void exit(int status)
	{
		pthread_exit((void *) &status);
	}
	void join()
	{
		int status = pthread_join(thread, NULL);
		assert(status == 0);
	}
	bool isEqual(pthread_t other)
	{
		return pthread_equal(thread, other);
	}

private:

	pthread_mutex_t mutex;
	sem_t _sem;
	pthread_t thread;

	static const pthread_t NO_THREAD = -1u;
};



#endif
