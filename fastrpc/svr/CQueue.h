#ifndef _SVR_CQUEUE_H_
#define _SVR_CQUEUE_H_

#include <deque>
#include <algorithm>
#include <functional>

#include "CMetaThread.h"

using std::deque;
using std::unary_function;



template <typename T>
class CMetaQueue
{
public:
	CMetaQueue(): waiters(0) { }
	~CMetaQueue() { }
	bool empty() { return queue.empty(); }
	void enqueue(T *r);
	T *dequeue();
	T *dequeue_nowait();

    unsigned size() { return queue.size();  }//不需要精度，保证性能，不加锁。

private:
	deque <T *> queue;
	CMetaThread thread;
	int waiters;
	T *dequeue_internal();
};

template <typename T>
void CMetaQueue <T>::enqueue(T *req)
{
    thread.lock();
	queue.push_back(req);
	thread.unlock();

    thread.wakeup();
}

template <typename T>
T *CMetaQueue <T>::dequeue_internal()
{
	T *r = queue.front();
	queue.pop_front();
	return r;
}

template <typename T>
T *CMetaQueue <T>::dequeue()
{
    T *r = NULL;
    //if ( queue.empty() )
    thread.sleep();

    thread.lock();
    r = (queue.empty()) ? NULL : dequeue_internal();
    thread.unlock();

	return r;
}

template <typename T>
T *CMetaQueue <T>::dequeue_nowait()
{
	thread.lock();
	T *r = (queue.empty()) ? NULL : dequeue_internal();
	thread.unlock();
	return r;
}

#endif
