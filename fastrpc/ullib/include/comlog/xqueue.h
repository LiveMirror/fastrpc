#ifndef _XQUEUE_H
#define _XQUEUE_H

#include <pthread.h>
#include <sys/time.h>
#include "xdebug.h"

namespace comspace
{
template<typename T>
class XQueue
{
private:
	int _capacity;	//总的容量
	int _size;		//队列当前容量
	int _front;		//头指针
	int _rear;		//尾指针
	T *_array;		//队列存储
public:
	XQueue() {
		_array = 0;
		_capacity = _size = _front = _rear = 0;
	}
	~XQueue() {
		destroy();
	}
	int size() {
		return _size;
	}
	/**
	 * @brief 返回队列支持的最大容量
	 *
	 * @return  int 队列支持的最大容量
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:22:29
	**/
	int capacity() {
		return _capacity;
	}
	/**
	 * @brief 判断队列是否为空
	 *
	 * @return  bool 返回true就是空，false非空
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:23:06
	**/
	bool empty() {
		return _size == 0;
	}
	/**
	 * @brief 返回队列是否已满
	 *
	 * @return  bool 返回true就是满，false非满
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:23:26
	**/
	bool full() {
		return _size == _capacity;
	}
	/**
	 * @brief 清空队列的内容，但不回收空间
	 *
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:24:07
	**/
	void clear() {
		_size = 0;
		_front = 0;
		_rear = 1;
	}
	/**
	 * @brief 创建队列
	 *
	 * @param [in qcap   : int 队列支持的最大长度
	 * @return  int 成功返回0，其他失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:24:32
	**/
	int create(int qcap) {
		//下的算法如果qcap小于2将无法正常工作
		if (qcap < 2) {
			return -1;
		}
		_capacity = qcap;
		clear();
		_array = new T[qcap];
		if (_array == 0) {
			return -1;
		}
		return 0;
	}
	/**
	 * @brief 销毁队列
	 *
	 * @return  int 成功返回0，其他失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 12:25:21
	**/
	int destroy() {
		_capacity = 0;
		if(_array != NULL){
			delete [] _array;
			_array = NULL;
		}
		return 0;
	}
	
	/**
	 * @brief 将元素放入队列尾部
	 *
	 * @param [out] val   : T&
	 * @return  int 成功返回0，其他失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/02/06 13:41:09
	**/
	int push_back(const T & val)  {
		if (full()) {
			return -1;
		}
		_array[_rear] = val;
		++ _size;
		if (++ _rear == _capacity) {
			_rear = 0;
		}
		return 0;
	}

	int push_front(const T &val) {
		if (full()) {
			//_com_error("queue is full size[%d], cap[%d]", _size, _capacity);
			return -1;
		}
		_array[_front] = val;
		++ _size;
		if (--_front < 0) {
			_front = _capacity-1;
		}
		return 0;
	}
	
	int pop_back(T &val) {
		if (empty()) {
			return -1;
		}
		if (--_rear < 0) {
			_rear = _capacity-1;
		}
		val = _array[_rear];
		-- _size;
		return 0;
	}
	int pop_front(T &val)  {
		if (empty()) {
			return -1;
		}
		++ _front;
		if (_front == _capacity) {
			_front = 0;
		}
		val = _array[_front];
		-- _size;
		return 0;
	}
	int pop_backs(T *val, int nums) {
		int cnt = 0;
		while (cnt < nums) {
			if (pop_back(val[cnt]) != 0) {
				return cnt;
			}
			++ cnt;
		}
		return cnt;
	}
};


template <typename T>
class XSigQu
{
public:
	XQueue<T> _queue;
	pthread_mutex_t _mutex;
	pthread_cond_t _cond;
	bool _created;
	bool _isWorking;
public:
	XSigQu(){
		_created = false;
	}
	~XSigQu(){
		destroy();
	}
	int create(int cap) {
		if(_created) return -1;
		pthread_mutex_init(&_mutex, NULL);
		pthread_cond_init(&_cond, NULL);
		_created = true;
		_isWorking = true;
		return _queue.create(cap);
	}
	int destroy() {
		if(! _created) return 0;
		pthread_mutex_destroy(&_mutex);
		pthread_cond_destroy(&_cond);
		_created = false;
		return _queue.destroy();
	}
	int push_front(const T &val) {
		pthread_mutex_lock (&_mutex);
		int ret = _queue.push_front(val);
		if (ret == 0) {
			pthread_cond_signal(&_cond);
		}
		pthread_mutex_unlock(&_mutex);
		return ret;
	}
	int push_back(const T &val) {
		pthread_mutex_lock (&_mutex);
		int ret = _queue.push_back(val);
		if (ret == 0) {
			pthread_cond_signal(&_cond);
		}
		pthread_mutex_unlock(&_mutex);
		return ret;
	}
	int pop_front(T &val, int to) {
		pthread_mutex_lock(&_mutex);
		if (to>0 && _queue.empty() && _isWorking) {
			struct timeval now;
			struct timespec timeout;
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + to;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&_cond, &_mutex, &timeout);
		}
		int ret = _queue.pop_front(val);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}

	int pop_back(T &val, int to) {
		pthread_mutex_lock(&_mutex);
		if (to>0 && _queue.empty() && _isWorking) {
			struct timeval now;
			struct timespec timeout;
			gettimeofday(&now, 0);
			timeout.tv_sec = now.tv_sec + to;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&_cond, &_mutex, &timeout);
		}
		int ret = _queue.pop_back(val);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}

	int pop_backs(T *val, int nums, int to)
	{
		pthread_mutex_lock(&_mutex);
		if (to>0 && _queue.empty() && _isWorking) {
			struct timeval now;
			struct timespec timeout;
			gettimeofday(&now, 0);
			timeout.tv_sec = now.tv_sec + to;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&_cond, &_mutex, &timeout);
		}
		int ret = _queue.pop_backs(val, nums);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}
	int size() { return _queue.size(); }
};
}

#endif

