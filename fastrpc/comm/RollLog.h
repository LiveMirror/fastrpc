#ifndef _ROLL_LOCK_H_
#define _ROLL_LOCK_H_

#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>

typedef enum _RLKEY {
    RLWARN = 0,
    RLINFO = 1,
    RLERR  = 2
} RLKEY;

// x 日志路径；y 并发句柄数
#define RLogInit(k,x,y)                  \
    RollL::RLogInitFun(k,x,y);           \

#define RLog(k, x)                        \
    if (k >= 1 && k < RollL::pool_size) {                         \
        std::stringstream rl_ss;              \
        rl_ss << x;                           \
        RollL::AsynRollLog* p_pool = RollL::pool_ary[k-1]; \
        if (!p_pool)                   \
            {printf("RollLogPointer null"); exit(1);} \
        p_pool->Log(rl_ss.str());   \
    }                                     \



namespace RollL {
#define rool_log_buffer_len 1024*20

// 一条日志超过20k不合理
class RollLog {
public:
    RollLog(bool islock = false) {
        lock = islock;
        file_is_open = false;
        nHandle = -1;
        dir = "./nodefine_log";
        szLine = new char[rool_log_buffer_len];
        szMsg = new char[rool_log_buffer_len];
        if (lock) pthread_mutex_init(&_jobmux,NULL);
    }
    ~RollLog() {
        delete []szLine;
        delete []szMsg;
        if (lock) pthread_mutex_destroy(&_jobmux);
    }
    void SetLogDir(std::string pdir);
    //int Log(char *pszFmt,...);
    int Log(std::string data);
private:
    bool file_is_open;
    int nHandle;
    std::string last_filepath;
    std::string dir;
    char *szLine;
    char *szMsg;
    pthread_mutex_t _jobmux;
    bool lock;
};

void DateAdd(int day, char *all_data, char *day_data,
             char *all_format, char *day_format, int all_size, int day_size);

class RollLogPool {
public:
    RollLogPool(int _pool_size = 4, std::string w_dir="./roll.log") {
        pool_size = _pool_size;
        pthread_mutex_init(&_jobmux,NULL);
        if((sem_init(&_jobsem,0,0))<0){
            printf("RollLogPool init fail");
            exit(1);
        }
        pthread_mutex_lock(&_jobmux);
        for (int i = 0; i < pool_size; ++i) {
            RollLog* roll_log = new RollLog();
            roll_log->SetLogDir(w_dir);
            roll_log_list.push_back(roll_log);
            sem_post(&_jobsem);
        }
        pthread_mutex_unlock(&_jobmux);

    }
    ~RollLogPool() {
        pthread_mutex_lock(&_jobmux);
        for (int i = 0; i < pool_size; ++i) {
            RollLog* roll_log = roll_log_list[i];
            delete roll_log;
        }
        roll_log_list.clear();
        pthread_mutex_unlock(&_jobmux);

        pthread_mutex_destroy(&_jobmux);
        sem_destroy(&_jobsem);
    }
    RollLog* GetLog() {
        sem_wait(&_jobsem);
        pthread_mutex_lock(&_jobmux);
        RollLog* c_log = NULL;
        while(!roll_log_list.empty()) {
            c_log = roll_log_list.front();
            roll_log_list.pop_front();
        }
        pthread_mutex_unlock(&_jobmux);
        return c_log;
    }
    bool Release(RollLog* c_log) {
        if (!c_log)
            return false;
        pthread_mutex_lock(&_jobmux);
        roll_log_list.push_back(c_log);
        pthread_mutex_unlock(&_jobmux);
        sem_post(&_jobsem);
        return true;
    }
    int pool_size;
    std::deque<RollLog*> roll_log_list;
    sem_t _jobsem;
    pthread_mutex_t _jobmux;
};

class AsynRollLog {
public:
    AsynRollLog(std::string w_dir="./asnodefine_log", int fd_num = 4) {
        dir = w_dir;
        isstop = false;
        pthread_mutex_init(&_jobmux,NULL);
        if((sem_init(&_jobsem,0,0))<0){
            printf("AsynRollLog sem init fail");
            exit(1);
        }
        fd_num_ = fd_num;
        for (int i = 0; i < fd_num_; ++i) {
            pthread_t tid = 0;
            pthread_create(&tid,NULL,AsynRollLog::ThreadProcess,this);
            thread_list.push_back(tid);
        }
    }

    ~AsynRollLog() {
        isstop = true;
        for (int i = 0; i < fd_num_; ++i) {
            sem_post(&_jobsem);
        }
        for (int i = 0; i < fd_num_; ++i) {
            pthread_join(thread_list[i],NULL);
        }
        thread_list.clear();
        pthread_mutex_destroy(&_jobmux);
        sem_destroy(&_jobsem);
    }

    static void *ThreadProcess(void *argument);

    int Log(std::string log_data) {
        pthread_mutex_lock(&_jobmux);
        msg_queue.push_back(log_data);
        pthread_mutex_unlock(&_jobmux);
        sem_post(&_jobsem);
        return 0;
    }

    //string有浅复制功能
    std::string GetLog() {
        sem_wait(&_jobsem);
        pthread_mutex_lock(&_jobmux);
        std::string log_data = msg_queue.front();
        msg_queue.pop_front();
        pthread_mutex_unlock(&_jobmux);
        return log_data;
    }

private:
    std::deque<std::string> msg_queue;
    std::vector<pthread_t> thread_list;
    sem_t _jobsem;
    pthread_mutex_t _jobmux;
    int fd_num_;
    bool isstop;
    std::string dir;
};

extern int pool_size;
typedef AsynRollLog* ARLP;
extern ARLP* pool_ary;
extern void RLogInitFun(int k, std::string x, int y);


}

#endif
