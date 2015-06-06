#include "baseheader.h"
#include "basetools.h"
#include <string>

/*int BaseTools::MultiProcessLog(char *pszFmt,...)
{
    char *szLine = new char[1024*20];
    memset(szLine, 0, sizeof(char)*1024*20);
    char *szMsg = new char[1024*20];
    memset(szMsg, 0, sizeof(char)*1024*20);
    std::string filepath = dir;
    pthread_mutex_lock(&logsmutex);
    try{
        va_list va;
        va_start(va, pszFmt);
        vsprintf(szMsg, pszFmt, va);
        va_end(va);
        char szDate[32];
        char logdate[32];
        DateAdd(0, szDate, logdate,
                (char *)"%Y-%m-%d %H:%M:%S",
                (char *)"__%Y-%m-%d",
                sizeof(szDate), sizeof(logdate));
        filepath.append(logdate);
        sprintf(szLine, "[%s] %s\n", szDate, szMsg);
    }
    catch(...)
    {
        strcpy(szLine, pszFmt);
    }
    struct flock lock;
    int nHandle = open(filepath.c_str(),  O_CREAT|O_APPEND|O_RDWR, 0666);
    if(nHandle)
    {
        lock.l_type = F_WRLCK;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        fcntl(nHandle,F_SETLKW, &lock);
        write(nHandle, szLine, strlen(szLine));
        flock(nHandle, LOCK_UN);
        close(nHandle);
    }

    pthread_mutex_unlock(&logsmutex);
    delete []szLine;
    delete []szMsg;
    return 1;
}*/

int BaseTools::WriteLog(char *pszFmt,...)
{
    //memset(szLine, 0, sizeof(char)*log_buffer_len);
    //memset(szMsg, 0, sizeof(char)*log_buffer_len);
    std::string filepath = dir;

    va_list va;
    va_start(va, pszFmt);
    vsnprintf(szMsg, log_buffer_len, pszFmt, va);
    va_end(va);
    char szDate[32];
    char logdate[32];
    DateAdd(0, szDate, logdate,
            (char *)"%Y-%m-%d %H:%M:%S",
            (char *)"__%Y-%m-%d",
            sizeof(szDate), sizeof(logdate));
    filepath.append(logdate);
    snprintf(szLine, log_buffer_len, "[%s] %s\n", szDate, szMsg);

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

    return 1;
}

void BaseTools::SetLogDir(string pdir) {
    dir = pdir;
}

int WriteDeBase64(char *pDir,char *pStr,int nLength)
{
        struct flock lock;
        int nHandle = open(pDir, O_CREAT|O_APPEND|O_RDWR, 0666);
        if(nHandle)
        {
                char Line[4] = {0x0d,0,0x0a,0};
                lock.l_type = F_WRLCK;
                lock.l_start = 0;
                lock.l_whence = SEEK_SET;
                lock.l_len = 0;
                fcntl(nHandle,F_SETLKW, &lock);
                write(nHandle, pStr, nLength);
                write(nHandle,Line,4);
                flock(nHandle, LOCK_UN);
                close(nHandle);
        } else {
                return 0;
        }
        return 1;
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
             int all_size, int day_size)
{
	struct tm t;
	t=get_time_tm(day);
	bzero(all_data, all_size);
	strftime(all_data, all_size, all_format, &t);
	bzero(day_data, day_size);
	strftime(day_data, day_size, day_format, &t);
}

unsigned long GetTickCount()
{
	unsigned long dCount = 0;
	struct timeval tp;
	gettimeofday(&tp, NULL);
	struct tm *ptm = localtime(&tp.tv_sec);

	dCount =ptm->tm_hour*60*60*1000 + ptm->tm_min*60*1000 + ptm->tm_sec*1000 + tp.tv_usec/1000;

	return dCount;
}

bool Replace_c(char *pBuff, char *pOldStr, char *pNewStr)
{
        if(pBuff == NULL)
                return false;

        string str = pBuff;
        string::size_type position = 0;
        while(1)
        {
                position = str.find(pOldStr);
                if(position > str.length())
                        break;
                str.replace(position,1, pNewStr);
        }
        return true;
}
