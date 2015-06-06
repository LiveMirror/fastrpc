#ifndef _BAIDU_X_SVR_H_
#define _BAIDU_X_SVR_H_

#include <pthread.h>

/*
    用于接受ependingpool的socket读写缓冲区
*/
class CRWCache
{
public:
    CRWCache();
    ~CRWCache();

    char* data();
    unsigned data_len();

    void append(const char* data, size_t data_len);
    void append_nolock(const char* data, size_t data_len);
    void skip(unsigned length);// 调用 c.skip(c.data_len())，相当于释放内存

    size_t fixnew(size_t dlen);
    void SetLock(bool _lock=false);

    void extern_lock(){if (lock) {pthread_mutex_lock(&mutex);}}
    void extern_unlock(){if (lock) {pthread_mutex_unlock(&mutex);}}

private:

    char* _mem;
    size_t _block_size; // 已经申请的空间

    unsigned _data_head; // 头部的位置，就是可用内存的位置
    size_t _data_len;    // 目前使用的内存长度
    pthread_mutex_t mutex; //锁
    bool lock;
};

#endif
