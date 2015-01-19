#include "RollLog.h"

namespace RollL {

//int RollLog::Log(char *pszFmt,...)
//{
//    if (lock) pthread_mutex_lock(&_jobmux);
//    std::string filepath = dir;
//
//    va_list va;
//    va_start(va, pszFmt);
//    vsnprintf(szMsg, rool_log_buffer_len, pszFmt, va);
//    va_end(va);
//    char szDate[32];
//    char logdate[32];
//    DateAdd(0, szDate, logdate,
//            (char *)"%Y-%m-%d %H:%M:%S",
//            (char *)".%Y-%m-%d",
//            sizeof(szDate), sizeof(logdate));
//    filepath.append(logdate);
//    snprintf(szLine, rool_log_buffer_len, "[%s] %s\n", szDate, szMsg);
//
//    if (filepath.compare(last_filepath) != 0 && file_is_open) {
//        close(nHandle);
//        file_is_open = false;
//        last_filepath = filepath;
//    }
//    if (!file_is_open) {
//        nHandle = open(filepath.c_str(),  O_CREAT|O_APPEND|O_RDWR, 0666);
//        if(nHandle < 0) {
//            printf("can not open log file:%s\n", filepath.c_str());
//        } else {
//            file_is_open = true;
//        }
//    }
//    write(nHandle, szLine, strlen(szLine));
//    if (lock) pthread_mutex_unlock(&_jobmux);
//
//    return 1;
//}

int RollLog::Log(std::string data)
{
    if (lock) pthread_mutex_lock(&_jobmux);
    std::string filepath = dir;

    char szDate[32];
    char logdate[32];
    DateAdd(0, szDate, logdate,
            (char *)"%Y-%m-%d %H:%M:%S",
            (char *)".%Y-%m-%d",
            sizeof(szDate), sizeof(logdate));
    filepath.append(logdate);
    snprintf(szLine, rool_log_buffer_len, "[%s] %s\n", szDate, data.c_str());

    if (filepath.compare(last_filepath) != 0 && file_is_open) {
        close(nHandle);
        file_is_open = false;
        last_filepath = filepath;
    }
    if (!file_is_open) {
        nHandle = open(filepath.c_str(),  O_CREAT|O_APPEND|O_RDWR, 0666);
        if(nHandle < 0) {
            printf("can not open log file:%s\n", filepath.c_str());
        } else {
            file_is_open = true;
        }
    }
    write(nHandle, szLine, strlen(szLine));
    if (lock) pthread_mutex_unlock(&_jobmux);

    return 1;
}

void RollLog::SetLogDir(std::string pdir) {
    dir = pdir;
}

struct tm get_time_tm(int day)
{
    struct tm t;
    time_t t1;
    time_t t2;
    t1=time(0);
    t2=t1+day*24*3600;
    localtime_r(&t2, &t);
    return t;
}

void DateAdd(int day, char *all_data, char *day_data,
             char *all_format, char *day_format,
             int all_size, int day_size) {
    struct tm t;
    t=get_time_tm(day);
    bzero(all_data, all_size);
    strftime(all_data, all_size, all_format, &t);
    bzero(day_data, day_size);
    strftime(day_data, day_size, day_format, &t);
}

void *AsynRollLog::ThreadProcess(void *argument) {
    AsynRollLog &aslog = (  *((AsynRollLog*)argument) );
    RollLog roll_log;
    roll_log.SetLogDir(aslog.dir);
    while (!aslog.isstop) {
        std::string log_data = aslog.GetLog();
        roll_log.Log(log_data);
    }
    return NULL;
}

int pool_size = 0;
ARLP* pool_ary = NULL;

void RLogInitFun(int k, std::string x, int y) {
    if (k < 1) {
        printf("RLogInit Param 1 cant not less than 1");
        exit(1);
    }
    if (k < RollL::pool_size) {
        AsynRollLog* p_pool = RollL::pool_ary[k-1];
        delete p_pool;
        p_pool = new RollL::AsynRollLog(x,y);
        pool_ary[k-1] = p_pool;
    } else {
        int new_size = k + 5;
        ARLP* new_ary = new ARLP[new_size];
        for (int i = 0; i < new_size; ++i) {
            if (i < RollL::pool_size)
                new_ary[i] = RollL::pool_ary[i];
            else
                new_ary[i] = NULL;
        }
        ARLP* old_ary = RollL::pool_ary;
        delete []old_ary;
        RollL::pool_ary = new_ary;
        RollL::pool_size = new_size;
        AsynRollLog* p_pool = new RollL::AsynRollLog(x,y);
        pool_ary[k-1] = p_pool;
    }
}

}

