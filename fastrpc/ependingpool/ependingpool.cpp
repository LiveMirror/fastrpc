/***************************************************************************
 *
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file ependingpool.cpp
 * @version $Revision: 1.12 $
 * @brief
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef KERNEL24
#include "epoll.h"
#else
#include <sys/epoll.h>
#endif


#include <ul_log.h>
#include <ul_net.h>
#include "ependingpool.h"

#define EPPOOLLOG(LEVEL, fmt, arg...) ;

/*
#define EPPOOLLOG(LEVEL, fmt, arg...) do {ul_writelog(LEVEL, \
		"[%s] [%s] : "fmt, __FILE__, __func__, ##arg);} while(0);
*/

ependingpool::ependingpool()
{
	m_sock_len = 0;
	m_free_thr = 0;

	m_listen_fd = -1;

	m_conn_timeo = DEF_CONN_TIMEO;
	m_read_timeo = DEF_READ_TIMEO;
	m_write_timeo = DEF_WRITE_TIMEO;
	m_min_timeo = m_read_timeo;

	m_ep_timeo = DEF_EPOLL_TIMEO;
	m_queue_len = DEF_QUEUE_LEN;
	m_sock_num = DEF_MAX_SOCK;

	m_insert_item_sock_thrsafe = 0;
	m_least_last_update = INT_MAX;

	//锁初始化
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_condition, NULL);
	pthread_mutex_init(&m_insert_sock, NULL);

	queue_create(m_queue_len);

	m_ay_sock = (sock_item_t *)calloc((size_t)m_sock_num, sizeof(sock_item_t));
	if (NULL == m_ay_sock) {
		EPPOOLLOG(UL_LOG_FATAL, "calloc m_ay_sock fail");
	}
	m_ay_events = (struct epoll_event *)calloc((size_t)m_sock_num,
			sizeof(struct epoll_event));      /**< epoll数组       */
	if (NULL == m_ay_events) {
		EPPOOLLOG(UL_LOG_FATAL, "calloc m_ay_events fail");
	}
	m_epfd = epoll_create(m_sock_num);
	for (int i = 0; i < SOCK_EVENT_NUM; ++i) {
		m_callbacklist[i] = NULL;
	}
	m_todo_event_callback = NULL;
	pool_run = 1;
}

/**
 * @brief pending_pool析构函数
 * 这里会把所有未关闭的资源关闭
 *
 * @author baonh
 **/
ependingpool::~ependingpool()
{
	if (m_epfd > 0) {
		close(m_epfd);
	}

	if (m_ay_sock) {
		for (int i = 0; i < m_sock_len; ++i) {
			if (NOT_USED != m_ay_sock[i].sock_status) {
				reset_item(i, false);
			}
		}
		free(m_ay_sock);
	}
	if (m_ay_events) {
		free(m_ay_events);
	}
	queue_destroy();
}


/**
 * @brief 设置监听句柄
 *
 * @param [in] lis_fd   : 需要监听的listen 句柄
 * @return  设置成功与否
 * @retval   -1 设置失败 0 设置成功
 * @author baonh
 **/
int ependingpool::set_listen_fd(int lis_fd)
{
	if (lis_fd <= 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid listen fd %d.", lis_fd);
		return -1;
	}

	m_listen_fd = lis_fd;

	return pool_epoll_add(lis_fd, -1, (/*EPOLLET |*/ EPOLLIN | EPOLLHUP | EPOLLERR));
}



int ependingpool::set_queue_len(int len)
{
	if (m_ay_ready) {
		free(m_ay_ready);
		m_ay_ready = NULL;
	}

	if (len > 2) {
		m_queue_len = len;
	} else {
		m_queue_len = 2;
	}
	return queue_create(m_queue_len);
}

int ependingpool::set_event_callback(int event, event_callback_t* callback)
{
	if (NULL == m_callbacklist) {
		EPPOOLLOG(UL_LOG_FATAL, "m_callbacklist is NULL");
		return -1;
	}
	if (event < 0 || event >= SOCK_EVENT_NUM) {
		EPPOOLLOG(UL_LOG_WARNING, "event must between [0, %d]", SOCK_EVENT_NUM);
		return -1;
	}
	if (m_callbacklist[event] != NULL) {
		EPPOOLLOG(UL_LOG_WARNING, "event %d is no NULL", event);
	}
	m_callbacklist[event] = callback;
	return 0;
}

int ependingpool::set_todo_event_ex(event_callback_ex_t* callback, void *user_arg)
{
	m_todo_event_callback = callback;
	m_todo_event_user_arg = user_arg;
	return 0;
}
int ependingpool::get_queue_len()
{
	int len = m_put - m_get;
	if (len < 0) {
		len += m_queue_len;
	}
	return len;
}

int ependingpool::set_sock_num(int num)
{
	if (num <= 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid socket number %d. sock_num keep unchanged, %d.",
				num, m_sock_num);
		return -1;
	}

	if (m_ay_sock) {
		free(m_ay_sock);
		m_ay_sock = NULL;
	}

	m_sock_num = num;
	m_ay_sock = (sock_item_t *)calloc((size_t)m_sock_num, sizeof(sock_item_t));
	if (!m_ay_sock) {
		EPPOOLLOG(UL_LOG_WARNING, "m_ay_sock calloc memory failed. [%d]: %m.", errno);
		return -1;
	}

	//epoll初始化
	if (m_epfd > 0) {
		while (close(m_epfd) < 0 && errno == EINTR) {};
		m_epfd = -1;
	}

	if (m_ay_events) {
		free(m_ay_events);
		m_ay_events = NULL;
	}

	m_ay_events = (struct epoll_event *)calloc((size_t)m_sock_num, sizeof(struct epoll_event));
	if (NULL == m_ay_events) {
		EPPOOLLOG(UL_LOG_WARNING, "m_ay_events calloc memory failed. [%d]: %m.", errno);
		return -1;
	}

	m_epfd = epoll_create(m_sock_num);
	if (m_epfd < 0) {
		EPPOOLLOG(UL_LOG_WARNING, "create epoll descriptor failed. [%d]: %m.", errno);
		return -1;
	}


	return 0;

}

int ependingpool::set_conn_timeo(int timeo)
{
	if (timeo <= 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid timeout %d timeout. timeout keep unchanged, %d.",
				timeo, m_conn_timeo);
		return -1;
	}

	m_conn_timeo = timeo;
	if (m_conn_timeo < m_read_timeo && m_conn_timeo < m_write_timeo) {
		m_min_timeo = m_conn_timeo;
	}
	return 0;
}

int ependingpool::set_read_timeo(int timeo)
{
	if (timeo <= 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid timeout %d read timeout. timeout keep unchanged, %d.",
				timeo, m_read_timeo);
		return -1;
	}

	m_read_timeo = timeo;
	if (m_read_timeo < m_conn_timeo && m_read_timeo < m_write_timeo) {
		m_min_timeo = m_read_timeo;
	}

	return 0;
}

int ependingpool::set_write_timeo(int timeo)
{
	if (timeo <= 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid timeout %d write timeout. timeout keep unchanged, %d.",
				timeo, m_write_timeo);
		return -1;
	}

	m_write_timeo = timeo;
	if (m_write_timeo < m_read_timeo && m_write_timeo < m_conn_timeo) {
		m_min_timeo = m_write_timeo;
	}

	return 0;
}

int ependingpool::get_conn_timeo()
{
	return m_conn_timeo;
}

int ependingpool::get_read_timeo()
{
	return m_read_timeo;
}


int ependingpool::get_write_timeo()
{
	return m_write_timeo;
}


int ependingpool::set_epoll_timeo(int timeo)
{
	if (timeo < 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid timeout %d milliseconds. time_out keep unchanged, %d.",
				timeo, m_ep_timeo);
		return -1;
	}

	m_ep_timeo = timeo;
	return 0;
}

int ependingpool::get_epoll_timeo()
{
	return m_ep_timeo;
}


int ependingpool::get_free_thread()
{
	return m_free_thr;
}

int ependingpool::get_offset()
{

	int ret;

	if (NULL == m_ay_sock) {
		return -1;
	}

	for (int i = 0; i < m_sock_len; ++i) {
		if (NOT_USED == m_ay_sock[i].sock_status) {
			return i;
		}
	}
	if (m_sock_len >= m_sock_num) {
		return -1;
	}
	ret = m_sock_len++;
	return ret;
}

int ependingpool::insert_item(int sock)
{
	if (sock < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "insert socket fail, invalid socket %d.", sock);
		return -1;
	}

	int current_offset = 0;

	int safe = m_insert_item_sock_thrsafe;
	if (safe) {
		pthread_mutex_lock(&m_insert_sock);
	}

	current_offset = get_offset();		  /**< 获取sock池中空闲位置       */
	//获取失败
	if (-1 == current_offset) {

		EPPOOLLOG(UL_LOG_WARNING, "insert socket[%d] fail, socket array[max_size:%d] overflow.",
				sock, m_sock_num);
		if (safe) {
			pthread_mutex_unlock(&m_insert_sock);
		}

		return -1;
	}

	//设置sock状态
	m_ay_sock[current_offset].last_active = time(NULL);
	m_ay_sock[current_offset].sock = sock;
	m_ay_sock[current_offset].sock_status = READY;
	EPPOOLLOG(UL_LOG_DEBUG, "socket %d inserted into sock array[offset:%d] [sock_len:%d].", sock,
			current_offset, m_sock_len);

	if (safe) {
		pthread_mutex_unlock(&m_insert_sock);
	}


	return current_offset;
}

int ependingpool::restart_run()
{
	pool_run = 1;
	return 0;
}

int ependingpool::stop()
{
	if (1 == pool_run) {
		pool_run = 0;
	}

	pthread_cond_broadcast(&m_condition);
	return 0;
}

int ependingpool::is_run()
{
	return pool_run;
}

/*
 * 获取已经准备好的sock
 */
/**
 * @brief 从已就绪队列中获取已经就绪的sock
 *
 * @param [in/out] offset   : int*
 * @param [in/out] sock   : int * 实际使用的句柄
 * @return  int
 * @retval
 * @see
 * @note
 * @author baonh
 **/
int ependingpool::fetch_item(int *handle, int *sock)
{

	int fetch_handle = -1;
	int fetch_sock;
	int ret = -1;
	if (NULL == m_ay_sock || NULL == handle || NULL == sock) {
		return -1;
	}
	pthread_mutex_lock(&m_mutex);

	++m_free_thr;
	//队列为空的时候
	while (queue_empty() && pool_run) {
		struct timeval now;
		struct timespec timeout;
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + 5;
		timeout.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&m_condition, &m_mutex, &timeout);
	}
	--m_free_thr;
	if (!pool_run) {
		EPPOOLLOG(UL_LOG_DEBUG, "pool is stop");
		goto failure;
	}
	//获取句柄
	if (queue_pop(&fetch_handle) < 0) {
		EPPOOLLOG(UL_LOG_WARNING, "get invalid handle [%d] from queue.", fetch_handle);
		goto failure;
	}

	fetch_sock = m_ay_sock[fetch_handle].sock;

	if (fetch_sock < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "get invalid socket %d from queue.", fetch_sock);
		goto failure;
	}

	if (m_ay_sock[fetch_handle].sock_status != BUSY) {
		EPPOOLLOG(UL_LOG_WARNING, "get error handle %d socket %d from queue, status is %d",
				fetch_handle, fetch_sock, m_ay_sock[fetch_handle].sock_status);
		goto failure;
	}


	if (m_free_thr > 0 && !queue_empty()) {
		pthread_cond_signal(&m_condition);
	}

	*handle = fetch_handle;
	*sock = fetch_sock;
	if (0 == pool_run) {
		goto failure;
	}

	pthread_mutex_unlock(&m_mutex);

	EPPOOLLOG(UL_LOG_DEBUG, "handle[%d] socket [%d] geted, head=%d, tail=%d, free_thr [%d].",
			fetch_handle, fetch_sock, m_get, m_put, m_free_thr);
	ret = 0;
	if (m_callbacklist[SOCK_FETCH] != NULL) {
		ret = m_callbacklist[SOCK_FETCH](this, fetch_sock, (void**)&(m_ay_sock[fetch_handle].arg));
		switch (ret) {
			case 0:
				break;

			case 1:
				reset_item(fetch_handle, false);
				break;

			case 2:
				reset_item(fetch_handle, true);
				break;

			case 3:
				clear_item(fetch_handle);
				break;

			default:
                EPPOOLLOG(UL_LOG_WARNING, "SOCK_FETCH callback return unknown ret[%d]", ret);
				reset_item(fetch_handle, false);
				break;
		}
	}

	return ret;

failure:
	pthread_mutex_unlock(&m_mutex);
	*handle = -1;
	*sock = -1;
	return ret;
}

int ependingpool::fetch_item(int *handle, int *sock, long long *staytime)
{
	int ret = 0;
	if (NULL == staytime) {
		return -1;
	}
	ret = fetch_item(handle, sock);
	if (ret != -1 && *handle >= 0) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		*staytime = (long long)(tv.tv_sec-m_ay_sock[*handle].queue_time.tv_sec)*1000000LL
			+ (tv.tv_usec-m_ay_sock[*handle].queue_time.tv_usec);
	}
	return ret;
}

int ependingpool::close_ready_socket()
{

	EPPOOLLOG(UL_LOG_DEBUG, "close ready socket m_sock_len:%d", m_sock_len);
	int ready = 0;
	for (int i = 0; i < m_sock_len; ++i) {
		if (READY == m_ay_sock[i].sock_status) {
			++ready;
			reset_item(i, false);
		}
	}
	return ready;
}

int ependingpool::waitforstop(int timeout)
{
	struct timeval tv_old;
	struct timeval tv;
	EPPOOLLOG(UL_LOG_DEBUG, "queue_len is %d", get_queue_len());
	stop();
	int handle = -1;
	pthread_mutex_lock(&m_mutex);
	while (!queue_empty()) {
		queue_pop(&handle);
		reset_item(handle, false);
	}
	pthread_mutex_unlock(&m_mutex);
	EPPOOLLOG(UL_LOG_DEBUG, "queue all close, queue is empty");
	gettimeofday(&tv_old, NULL);
	while (1) {
		gettimeofday(&tv, NULL);
		if ((tv.tv_sec - tv_old.tv_sec)*1000L + (tv.tv_usec - tv_old.tv_usec)/1000L > timeout) {
			break;
		}
		check_item();
		if (0 == m_read_socket_num && 0 == m_write_socket_num) {
			break;
		}
	}
	int close_sock = close_ready_socket();
	EPPOOLLOG(UL_LOG_DEBUG, "close socket num is %d, queue_len is %d", close_sock, get_queue_len());
	return 0;
}

int ependingpool::clear_item(int handle)
{
	int sock = -1;

	if (handle < 0 || handle >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d.", handle);
		return -1;
	}

	if (NULL == m_ay_sock) {
		EPPOOLLOG(UL_LOG_FATAL, "reset_item invalid m_ay_sock.");
		return -1;
	}

	if (NOT_USED == m_ay_sock[handle].sock_status) {
		EPPOOLLOG(UL_LOG_WARNING, "no found socket in handle %d.", handle);
		return 0;
	}

	sock = m_ay_sock[handle].sock;

	if (sock < 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d sock %d.", handle, sock);
		return -1;
	}
	//不判断epoll del成功与否,这种错误没有影响
	pool_epoll_offset_del(handle);


	if (m_callbacklist[SOCK_CLEAR] != NULL) {
		m_callbacklist[SOCK_CLEAR](this, m_ay_sock[handle].sock, (void**)(&m_ay_sock[handle].arg));
		m_ay_sock[handle].arg = NULL;
	}

	m_ay_sock[handle].sock = -1;
	m_ay_sock[handle].sock_status = NOT_USED;
	return 0;

}

int ependingpool::reset_item(int handle, bool keep_alive)
{
	int ret = 0;
	int sock = -1;

	if (handle < 0 || handle >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d.", handle);
		return -1;
	}

	if (NULL == m_ay_sock) {
		EPPOOLLOG(UL_LOG_FATAL, "reset_item invalid m_ay_sock.");
		return -1;
	}

	if (NOT_USED == m_ay_sock[handle].sock_status) {
		EPPOOLLOG(UL_LOG_WARNING, "no found socket in handle %d.", handle);
		return 0;
	}

	sock = m_ay_sock[handle].sock;

	if (sock < 0) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d sock %d.", handle, sock);
		return -1;
	}


	if (!(keep_alive && pool_run)) {	//不保持连接
		// 关闭句柄
		EPPOOLLOG(UL_LOG_DEBUG, "close sock[%d]", sock);

		if (m_callbacklist[SOCK_CLEAR] != NULL) {
			m_callbacklist[SOCK_CLEAR](this, m_ay_sock[handle].sock, (void**)(&m_ay_sock[handle].arg));
			m_ay_sock[handle].arg = NULL;
		}
		pool_epoll_del(sock, handle);

		while ((ret = close(sock)) < 0 && errno == EINTR) {};

		if (ret < 0) {
			EPPOOLLOG(UL_LOG_WARNING, "close socket handle[%d], %d failed, [%d]: %m.",
					handle, sock, errno);
			return -1;
		}

		m_ay_sock[handle].sock = -1;
		m_ay_sock[handle].sock_status = NOT_USED;

	} else {		// keep alive
		m_ay_sock[handle].last_active = time(NULL);
		m_ay_sock[handle].sock_status = READY;
		ret = pool_epoll_offset_mod(handle, /*EPOLLET |*/ EPOLLIN | EPOLLHUP | EPOLLERR);
	}

	EPPOOLLOG(UL_LOG_DEBUG, "socket %d handle %d, reseted, %s",
			sock, handle, (keep_alive) ? "alive" : "closed");

	return ret;
}

int ependingpool::fetch_handle_arg(int handle, void **arg)
{
	if (handle < 0 || handle >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d.", handle);
		return -1;
	}

	if (NULL == m_ay_sock) {
		EPPOOLLOG(UL_LOG_FATAL, "reset_item invalid m_ay_sock.");
		return -1;
	}

	*arg = m_ay_sock[handle].arg;
	return 0;
}


int ependingpool::write_reset_item(int handle)
{
	if (handle < 0 || handle >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid handle %d.", handle);
		return -1;
	}

	if (NULL == m_ay_sock) {
		EPPOOLLOG(UL_LOG_FATAL, "reset_item invalid m_ay_sock.");
		return -1;
	}

	if (NOT_USED == m_ay_sock[handle].sock_status) {
		EPPOOLLOG(UL_LOG_WARNING, "no found socket in handle %d.", handle);
		return -1;
	}

	EPPOOLLOG(UL_LOG_DEBUG, "write rest handle[%d]", handle);
	m_ay_sock[handle].last_active = time(NULL);
	m_ay_sock[handle].sock_status = WRITE_BUSY;

	return pool_epoll_offset_mod(handle, /*EPOLLLT |*/ EPOLLOUT | EPOLLHUP | EPOLLERR | EPOLLIN );
}

int ependingpool::set_insert_item_sock_thrsafe(int safe)
{
	m_insert_item_sock_thrsafe = safe;
	return 0;
}
int ependingpool::insert_item_sock(int sock,int &async_offset,void *arg, int flags)
{
	int offset = -1;
	int events = 0;
	if (0 == pool_run) {
		return -1;
	}
	offset = insert_item(sock);
    async_offset = offset;

	if (offset < 0) {
		if (m_callbacklist[SOCK_INSERTFAIL] != NULL) {
			m_callbacklist[SOCK_INSERTFAIL](this, sock, NULL);
		}
		return -1;
	}

	if (arg != NULL) {
		m_ay_sock[offset].arg = arg;
	} else {
		if (m_callbacklist[SOCK_INIT] != NULL) {
			if (m_callbacklist[SOCK_INIT](this, sock, (void**)(&(m_ay_sock[offset].arg))) < 0) {
                EPPOOLLOG(UL_LOG_WARNING, "SOCK_INIT callback return < 0");
				clear_item(offset);
				return -1;
			}
		}
	}
	if (0 == flags) {
		events = /*EPOLLLT |*/ EPOLLIN | EPOLLHUP | EPOLLERR;
	} else {
		events = /*EPOLLET |*/ EPOLLOUT | EPOLLHUP | EPOLLERR;
		//events = EPOLLOUT | EPOLLHUP | EPOLLERR;
	}
	if (pool_epoll_offset_add(offset, events) < 0) {
		if (arg != NULL) {
			m_ay_sock[offset].arg = NULL;
		}
        EPPOOLLOG(UL_LOG_WARNING, "epoll_del offset[%d] error", offset);
		clear_item(offset);
		return -1;
	}
	return 0;
}
//to detech server socket. liuyulian; 2014-04-09
void ependingpool::do_epollhup(int offset)
{
	if (m_callbacklist[SOCK_HUP] != NULL){
        m_callbacklist[SOCK_HUP](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
	}



}
void ependingpool::do_epollerr(int offset){
	if (m_callbacklist[SOCK_ERROR] != NULL){
		m_callbacklist[SOCK_ERROR](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
	}

}
void ependingpool::do_read_event(int offset)
{
	int ret = 0;
	if (offset < 0 || offset >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid offset %d.", offset);
		return;
	}
	++m_read_socket_num;
	if (m_callbacklist[SOCK_READ] != NULL) {
		EPPOOLLOG(UL_LOG_DEBUG, "run read callback..., arg [%p]",
				m_ay_sock[offset].arg);
		if (READ_BUSY != m_ay_sock[offset].sock_status) {
			m_ay_sock[offset].last_active = time(NULL);
			m_ay_sock[offset].sock_status = READ_BUSY;
		}
		ret = m_callbacklist[SOCK_READ](this, m_ay_sock[offset].sock,
				(void**)(&m_ay_sock[offset].arg));
	} else {
		ret = 0;
	}
	switch (ret) {
		case 0:
			ret = queue_in(offset);
			if (0 == ret) {
				break;
			}
            EPPOOLLOG(UL_LOG_WARNING, "queue_in offset[%d] error[%d] sock[%d]", offset, ret, m_ay_sock[offset].sock);
			if (m_callbacklist[SOCK_QUEUEFAIL] != NULL) {
				ret = m_callbacklist[SOCK_QUEUEFAIL](this, m_ay_sock[offset].sock,
						(void**)(&m_ay_sock[offset].arg));

			}
			switch (ret) {
				case 0:
					if (m_callbacklist[SOCK_CLOSE] != NULL){
						m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
					}
					reset_item(offset, false);
					break;
				case 1:
					clear_item(offset);
					break;
				default:
                    EPPOOLLOG(UL_LOG_WARNING, "SOCK_QUEUEFAIL callback return ret[%d], sock[%d]", ret, m_ay_sock[offset].sock);
					if (m_callbacklist[SOCK_CLOSE] != NULL){
						m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
					}
					reset_item(offset, false);
					break;
			}
			break;

		case 1:
			EPPOOLLOG(UL_LOG_DEBUG, "read no end ret[%d], sock reset to EPOLLIN",ret);
			pool_epoll_offset_mod(offset, EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLOUT);
			break;

		case 2:
			if (m_callbacklist[SOCK_TODO] != NULL) {
				EPPOOLLOG(UL_LOG_DEBUG, "read call success, not put into epoll, call"
						"SOCK_TODO");

				m_ay_sock[offset].sock_status = BUSY;
				ret = m_callbacklist[SOCK_TODO](this, m_ay_sock[offset].sock,
						(void**)(&m_ay_sock[offset].arg));
			} else {
				ret = 0;
			}
			if (0 == ret) {
				if (NULL != m_todo_event_callback) {
					ependingpool_task_t todo_arg;
					m_ay_sock[offset].sock_status = BUSY;
					todo_arg.sock = m_ay_sock[offset].sock;
					todo_arg.offset = offset;
					todo_arg.arg = m_ay_sock[offset].arg;
					todo_arg.user_arg = m_todo_event_user_arg;
					ret = m_todo_event_callback(this, &todo_arg);
				}
			}

			switch (ret) {
				case 0:
					//write_reset_item(offset);
					break;

				case 1:
					//reset_item(offset, true);
					break;

				case 2:
					clear_item(offset);
					break;

				case 3:
					break;

				default:
					EPPOOLLOG(UL_LOG_WARNING, "call back SOCK_TODO run result is err[%d]",ret);
					if (m_callbacklist[SOCK_CLOSE] != NULL){
						m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
					}
					reset_item(offset, false);
					break;
			}
			break;

		case 3:
			EPPOOLLOG(UL_LOG_DEBUG, "read end call clear item");
			clear_item(offset);
			break;

		default:
			EPPOOLLOG(UL_LOG_WARNING, "read call error [%d]", ret);
			if (m_callbacklist[SOCK_CLOSE] != NULL){
				m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
			}
			reset_item(offset, false);
			break;

	}

}

void ependingpool::do_write_event(int offset)
{
	int ret = 0;
	if (offset < 0 || offset >= m_sock_len) {
		EPPOOLLOG(UL_LOG_WARNING, "invalid offset %d.", offset);
		return;
	}
	++m_write_socket_num;
	if (m_callbacklist[SOCK_WRITE] != NULL) {
		EPPOOLLOG(UL_LOG_DEBUG, "run write callback..., arg [%p]",
				m_ay_sock[offset].arg);

		if (WRITE_BUSY != m_ay_sock[offset].sock_status) {
			m_ay_sock[offset].last_active = time(NULL);
			m_ay_sock[offset].sock_status = WRITE_BUSY;
		}

		ret = m_callbacklist[SOCK_WRITE](this, m_ay_sock[offset].sock,
				(void**)(&m_ay_sock[offset].arg));
	} else {
		ret = 0;
	}

	switch (ret) {
		case 0:
			EPPOOLLOG(UL_LOG_DEBUG, "write end sock, close sock[%d]",
					m_ay_sock[offset].sock);
			reset_item(offset, true);
			break;
		case 1:
			EPPOOLLOG(UL_LOG_DEBUG, "write no end ret, continue");//write_reset_item(offset);
			break;
		case 2:
			EPPOOLLOG(UL_LOG_DEBUG, "write end sock, no close sock[%d]",
					m_ay_sock[offset].sock);
	    	if (m_callbacklist[SOCK_CLOSE] != NULL){
				m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
			}
			reset_item(offset, false);
			break;

		case 3:
			EPPOOLLOG(UL_LOG_DEBUG, "write end sock, clear item");
			clear_item(offset);
			break;

		default:
			EPPOOLLOG(UL_LOG_WARNING, "write call error [%d]", ret);
			reset_item(offset, false);
			if (m_callbacklist[SOCK_CLOSE] != NULL){
					m_callbacklist[SOCK_CLOSE](this, m_ay_sock[offset].sock,(void**)(&m_ay_sock[offset].arg));
			}
			break;
	}

}

int ependingpool::check_item()
{
	//先检查超时事件
	check_time_out();
	//等待epoll事件;
	int num = pool_epoll_wait(m_ep_timeo);
	if (num <= 0) { //没有触发事件
		return num;
	}
    int async_offset = -1; // 修改by feimat@feimat.com 2013-02-01
	int offset = -1;
	m_read_socket_num = 0;
	m_write_socket_num = 0;
	for (int i = 0; i < num; i++) {
		//如果是监听端口则accept出来
		if (m_ay_events[i].data.fd == -1 && m_listen_fd > 0) {
			int work_sock = -1;
			while ((work_sock = accept_sock()) > 0) {
				if (insert_item_sock(work_sock,async_offset) < 0) {
					while (close(work_sock) < 0 && errno == EINTR) {};
					EPPOOLLOG(UL_LOG_WARNING, "insert_item_sock work_sock [%d] fail", work_sock);
				}
			}

		} else if (m_ay_events[i].data.fd >= 0) {
			offset = m_ay_events[i].data.fd;
			//检查发生的事件
			EPPOOLLOG(UL_LOG_DEBUG, "events handle is [%d]", offset);

			if (m_ay_events[i].events & EPOLLHUP) { //断开
				do_epollhup(offset);  //add by liuyulian;2014-04-09. detech server socket
				EPPOOLLOG(UL_LOG_WARNING, "socket %d closed by peer.", m_ay_sock[offset].sock);
				reset_item(offset, false);
			} else if (m_ay_events[i].events & EPOLLERR) {	// 出错
				do_epollerr(offset);
				EPPOOLLOG(UL_LOG_WARNING, "socket %d error.", m_ay_sock[offset].sock);
				reset_item(offset, false);
			} else if ((m_ay_events[i].events & EPOLLIN)&&pool_run) {	// 可读
				do_read_event(offset);
			} else if (m_ay_events[i].events & EPOLLOUT) {
				do_write_event(offset);
			} else {
				printf("offset %d is close\n");
				//EPPOOLLOG(UL_LOG_WARNING, "offset %d is close", offset);
				reset_item(offset, false);
			}
		}
	}
	return 0;
}

int ependingpool::accept_sock()
{
	int work_sock = -1;
	//用户定义的回调函数
	if (m_callbacklist[SOCK_ACCEPT] != NULL) {
		work_sock = m_callbacklist[SOCK_ACCEPT](this, m_listen_fd, NULL);
		if (work_sock < 0) {
			EPPOOLLOG(UL_LOG_WARNING, "accept sock callback fail");
			return -1;
		}
		return work_sock;
	}

	// accept connection
	work_sock = ul_accept(m_listen_fd, NULL, NULL);
	if (work_sock < 0) {// accept failed.
		EPPOOLLOG(UL_LOG_WARNING, "accept sock fail");
		return -1;
	}
	EPPOOLLOG(UL_LOG_DEBUG, "accept [%d] success", work_sock);

	return work_sock;
}

int ependingpool::queue_in(int offset)
{
	if (!pool_run) {
		EPPOOLLOG(UL_LOG_WARNING, "pool is stop");
		return -1;
	}
	int sock = m_ay_sock[offset].sock;
	if (m_ay_sock[offset].sock_status != READY &&
			m_ay_sock[offset].sock_status != READ_BUSY) {
		EPPOOLLOG(UL_LOG_WARNING, "status of socket %d offset %d is not ready!", sock, offset);
		return -1;
	}

	pthread_mutex_lock(&m_mutex);
	//sock放入就绪队列
	int ret = queue_push(offset);

	if (ret < 0) { //放入失败
		EPPOOLLOG(UL_LOG_WARNING, "queue overflow socket %d, queue len %d",
				sock, m_queue_len);
		pthread_mutex_unlock(&m_mutex);
		return -1;

	} else {
		// 设置状态为 BUSY
		m_ay_sock[offset].sock_status = BUSY;
		// 设置最后活动时间
		gettimeofday(&(m_ay_sock[offset].queue_time), NULL);
		m_ay_sock[offset].last_active = m_ay_sock[offset].queue_time.tv_sec;
		if (m_free_thr > 0) {
			pthread_cond_signal(&m_condition);
		}
	}

	pthread_mutex_unlock(&m_mutex);
	EPPOOLLOG(UL_LOG_DEBUG, "socket %d offset %d added into ready queue, head=%d, tail=%d",
			sock, offset, m_get, m_put);

	return 0;
}
int ependingpool::queue_create(int queue_size)
{
	m_get = 0;
	m_put = 0;
	m_ay_ready = (int *)calloc((size_t)queue_size, sizeof(int));
	if (NULL == m_ay_ready) {
		EPPOOLLOG(UL_LOG_WARNING, "calloc queue fail");
		return -1;
	}
	return 0;

}
int ependingpool::queue_push(int val)
{
	if (NULL == m_ay_ready) {
		return -1;
	}


	if (queue_full()) {
		return -1;
	}

	m_ay_ready[m_put] = val;
	if (++m_put >= m_queue_len) {
		m_put = 0;
	}
	return 0;
}
int ependingpool::queue_pop(int *val)
{
	if (NULL == m_ay_ready) {
		return -1;
	}
	if (queue_empty()) {
		return -1;
	}
	*val = m_ay_ready[m_get];

	if (++m_get >= m_queue_len) {
		m_get = 0;
	}
	return 0;
}

int ependingpool::queue_empty()
{
	return (m_put == m_get);
}
int ependingpool::queue_full()
{
	int pos = m_put + 1;
	if (pos >= m_queue_len) {
		pos -= m_queue_len;
	}
	if (pos == m_get) {	// overflow
		return 1;
	}
	return 0;
}

int ependingpool::queue_destroy()
{
	if (m_ay_ready != NULL) {
		free(m_ay_ready);
		m_ay_ready = NULL;
	}
	return 0;
}

int ependingpool::check_time_out()
{
	int current_time = time(NULL);
	if (m_least_last_update != INT_MAX &&
		current_time - m_least_last_update <= m_min_timeo) {
		return 0;
	}
	if (NULL == m_ay_sock || m_sock_len < 0) {
		return -1;
	}
	int ready_num = 0;
	int busy_num = 0;
	int read_num = 0;
	int write_num = 0;
	int last_active_offset = -1;
	m_least_last_update = INT_MAX;

	for (int i = 0; i < m_sock_len; ++i) {
		switch (m_ay_sock[i].sock_status) {
			case NOT_USED:
				break;

			case READY:
				if (current_time >= m_ay_sock[i].last_active + m_conn_timeo) {
					EPPOOLLOG(UL_LOG_WARNING, "handle %d time out last_active[%d], m_conn_timeo[%d], current_time[%d]",
							i, m_ay_sock[i].last_active, m_conn_timeo, current_time);
					if (m_callbacklist[SOCK_LISTENTIMEOUT] != NULL) {
						m_callbacklist[SOCK_LISTENTIMEOUT](this, m_ay_sock[i].sock,
								(void**)(&m_ay_sock[i].arg));
					}

					reset_item(i, false);
					continue;
				}
				ready_num ++;
				if (m_least_last_update > m_ay_sock[i].last_active) {
					m_least_last_update = m_ay_sock[i].last_active;
				}
				last_active_offset = i;
				break;
			case READ_BUSY:
				if (current_time >= m_ay_sock[i].last_active + m_read_timeo) {
					EPPOOLLOG(UL_LOG_WARNING, "handle %d read time out last_active[%d], m_conn_timeo[%d], current_time[%d]",
							i, m_ay_sock[i].last_active, m_conn_timeo, current_time);
					if (m_callbacklist[SOCK_READTIMEOUT] != NULL) {
						m_callbacklist[SOCK_READTIMEOUT](this, m_ay_sock[i].sock,
								(void**)(&m_ay_sock[i].arg));
					}
					reset_item(i, false);
					continue;
				}

				read_num ++;
				if (m_least_last_update > m_ay_sock[i].last_active) {
					m_least_last_update = m_ay_sock[i].last_active;
				}
				last_active_offset = i;
				break;
			case WRITE_BUSY:
				if (current_time >= m_ay_sock[i].last_active + m_write_timeo) {
					EPPOOLLOG(UL_LOG_WARNING, "handle %d write time out last_active[%d], m_conn_timeo[%d], current_time[%d]",
							i, m_ay_sock[i].last_active, m_conn_timeo, current_time);
					if (m_callbacklist[SOCK_WRITETIMEOUT] != NULL) {
						m_callbacklist[SOCK_WRITETIMEOUT](this, m_ay_sock[i].sock,
								(void**)(&m_ay_sock[i].arg));
					}
					reset_item(i, false);
					continue;
				}

				write_num ++;
				if (m_least_last_update > m_ay_sock[i].last_active) {
					m_least_last_update = m_ay_sock[i].last_active;
				}
				last_active_offset = i;
				break;
			case BUSY:
				busy_num++;
				m_ay_sock[i].last_active = current_time;
				last_active_offset = i;
				break;

			default:
				EPPOOLLOG(UL_LOG_WARNING, "unknow status %d",
						m_ay_sock[i].sock_status);
				break;

		}

	}
	//为保证多线程insert的安全, m_sock_len不减少
	if (!m_insert_item_sock_thrsafe) {
		m_sock_len = last_active_offset + 1;
	}

	if (ready_num == 0 && (busy_num > 0 || read_num > 0 || write_num > 0)) {
		EPPOOLLOG(UL_LOG_DEBUG, "%d socket READY, %d BUSY, %d READ %d WRITE %d Total",
				ready_num, busy_num, read_num, write_num, m_sock_len);
	}
	return 0;
}

int ependingpool::pool_epoll_wait(int timeout)
{
	int  nfds;

	if (m_epfd <= 0) {
		EPPOOLLOG(UL_LOG_FATAL, "invalid epoll fd %d.", m_epfd);
		return -1;
	}

	while (1) {
		nfds = epoll_wait(m_epfd, m_ay_events, m_sock_num, timeout);
		if (nfds < 0 && errno == EINTR) {
			continue;
		}
		if (nfds < 0) {
			EPPOOLLOG(UL_LOG_WARNING, "epoll_wait failed. [%d]: %m", errno);
		}
		break;
	}
	return nfds;
}

int ependingpool::pool_epoll_offset_add(int offset, int events)
{
	if (offset < 0) {
		return -1;
	}
	int ret = pool_epoll_add(m_ay_sock[offset].sock, offset, events);

	return ret;

}

int ependingpool::pool_epoll_offset_del(int offset)
{
	if (offset < 0) {
		return -1;
	}
	int ret = pool_epoll_del(m_ay_sock[offset].sock, offset);

	return ret;
}

int ependingpool::pool_epoll_add(int sock, int fd, int events)
{
	struct epoll_event ev;

	if (m_epfd < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "invalid epoll fd %d.", m_epfd);
		return -1;
	}
	ev.data.u64 = 0UL;
	ev.data.fd = fd;
	ev.events = events;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, sock, &ev) < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "epoll_ctl add socket %d failed. [%d]: %m.", sock, errno);
		return -1;
	}


	EPPOOLLOG(UL_LOG_DEBUG, "socket %d fd %d add into epoll", sock, fd);

	return 0;
}

int ependingpool::pool_epoll_del(int sock, int fd)
{
	struct epoll_event ev;

	if (m_epfd < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "invalid epoll fd %d.", m_epfd);
		return -1;
	}
	ev.data.u64 = 0UL;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;

	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, sock, &ev) < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "epoll_ctl del socket %d failed. [%d]: %m.", sock, errno);
		return -1;
	}
	EPPOOLLOG(UL_LOG_DEBUG, "socket %d fd %d delete form epoll", sock, fd);
	return 0;
}

int ependingpool::pool_epoll_offset_mod(int offset, int event)
{
	if (offset < 0) {
		return -1;
	}
	int ret = pool_epoll_mod(m_ay_sock[offset].sock, offset, event);

	return ret;
}

int ependingpool::pool_epoll_mod(int sock, int fd, int event)
{
	struct epoll_event ev;

	if (m_epfd < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "invalid epoll fd %d.", m_epfd);
		return -1;
	}
	ev.data.u64 = 0ULL;
	ev.data.fd = fd;
	ev.events = event;

	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev) < 0) {
		EPPOOLLOG(UL_LOG_FATAL, "epoll_mod mod socket %d failed. [%d]: %m.", sock, errno);
		return -1;
	}


	EPPOOLLOG(UL_LOG_DEBUG, "socket %d fd %d mod in epoll", sock, fd);

	return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
