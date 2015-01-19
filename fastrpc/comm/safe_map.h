/**
************************************
* Copyright (c) 2013 baidu
* All rights reserved.
************************************
*
* @file conn_pool.h
*
* @version 0.0.1
* @author hemingzhe
* @date 2013.6.6
*/

#ifndef _SAFE_MAP_H_
#define _SAFE_MAP_H_

#include <pthread.h>
#include <map>

template<typename T1,typename T2>
class SafeMap
{
public:
	SafeMap() {
        pthread_mutex_init(&mutex, NULL);
    }
	~SafeMap() {
        pthread_mutex_destroy(&mutex);
    }

    void insert(T1 key, T2 value);

    T2 find(T1 key, T2 default_value);

private:
    pthread_mutex_t mutex;
    std::map<T1, T2> map_obj;
};

template <typename T1,typename T2>
void SafeMap<T1, T2>::insert(T1 key, T2 value) {
    pthread_mutex_lock(&mutex);
    map_obj.insert(std::map<T1, T2>::value_type(key, value));
    pthread_mutex_unlock(&mutex);
}

template <typename T1,typename T2>
T2 SafeMap<T1, T2>::find(T1 key, T2 default_value) {
    pthread_mutex_lock(&mutex);
    map<T1, T2>::iterator iter;
    iter = map_obj.find(key);
    if (iter != map_obj.end()) {
        return iter->second;
    }
    else {
        return default_value;
    }
    pthread_mutex_unlock(&mutex);
}

#endif
