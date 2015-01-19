/*
功能: 
       1.  Log类

Created by Song, 2003-01
Change list:

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#include "log.h"

#define WRITE_LOG_STR(str)\
    int iBufLen = 0;\
    time_t tnow;\
    struct tm tm;\
    tnow = time(NULL);\
    localtime_r(&tnow, &tm);\
    iBufLen = snprintf(_log_buf, sizeof(_log_buf), \
    "[%04d-%02d-%02d %02d:%02d:%02d] %d: %s",\
            tm.tm_year + 1900, tm.tm_mon + 1,\
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,\
            _proc_no, str);\
\
    va_list ap;\
    va_start(ap, fmt);\
    vsnprintf(_log_buf + iBufLen, sizeof(_log_buf) - iBufLen, fmt, ap);\
    va_end(ap);\
    return puts(_log_buf);



class CLog *g_pLog = NULL;

CLog::CLog(const char *log_base_path,
           int max_log_size, int max_log_num, int *log_level_ptr)
:  _fp(NULL),
_max_log_size(max_log_size),
_max_log_num(max_log_num),
_log_level_ptr(log_level_ptr), _write_count(0), _proc_no(0)
{
    strncpy(_log_base_path, log_base_path, sizeof(_log_base_path));
    _log_base_path[sizeof(_log_base_path) - 1] = '\0';
    snprintf(_log_file_name, sizeof(_log_file_name), "%s.log", _log_base_path);
}

CLog::~CLog()
{
    close();
}

int
CLog::close()
{
    if (_fp != NULL) {
        fclose(_fp);
        _fp = NULL;
    }
    return 0;
}

int
CLog::open()
{
#ifdef TEST
    return 0;
#endif

//    static int flags = O_WRONLY | O_CREAT | O_APPEND;
//    static mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    //以追加方式打开日志文件
    if (_fp == NULL) {
        if ((_fp = fopen(_log_file_name, "a")) == NULL) {
            sprintf(_error_text, " fopen %s: %s",
                    _log_file_name, strerror(errno));
            return -1;
        }
    }

    return 0;
}

int
CLog::error_log(const char *fmt, ...)
{
    if (get_log_level() < ERROR_LOG) {
        // 不写错误log
        return 0;
    }

    WRITE_LOG_STR("ERROR: ");
}

int
CLog::write_log(const char *fmt, ...)
{
    if (get_log_level() < NORMAL_LOG) {
        // 不写普通log
        return 0;
    }

    WRITE_LOG_STR("");
}

int
CLog::debug_log(const char *fmt, ...)
{
    if (get_log_level() < DEBUG_LOG) {
        // 不写调试log
        return 0;
    }

    WRITE_LOG_STR("DEBUG: ");
}

int
CLog::print_hex(const void *buf, size_t len)
{
    if (get_log_level() < DEBUG_LOG) {
        // 不写调试log
        return 0;
    }

    size_t i;
    int k;
    char str[256];
    const char *p = (const char *) buf;

    assert(buf != NULL);
    assert(0 < len);

    k = 0;
    for (i = 0; i < len; ++i, ++p) {
        if (i != 0 && i % 8 == 0) {
            k += sprintf(str + k, " ");
        }

        if (i != 0 && i % 16 == 0) {
            k += sprintf(str + k, "\n");
            puts(str);
            k = 0;
        }

        k += sprintf(str + k, "%02x ", (unsigned char) *p);
    }

    k += sprintf(str + k, "\n"); // 2003.11.15 frostburn
    puts(str);

    return 0;
}

int
CLog::puts(const char *str)
{
#ifdef TEST
    ::puts(str);
    fflush(stdout);
    return 0;
#else

    //以追加方式打开日志文件
    if (_fp == NULL) {
        if (open() != 0) {
            fprintf(stderr, "ERROR: %s\n", _error_text);
            return -1;
        }
    }

    int ret = fputs(str, _fp);
    if (ret < 0) {
        fclose(_fp);
        _fp = NULL;
        fprintf(stderr, "ERROR: fputs %s: %s\n",
                _log_file_name, strerror(errno));
        return ret;
    }

    fflush(_fp);

    ++_write_count;
    if (_write_count >= LOG_SHIFT_COUNT) {
        fclose(_fp);
        _fp = NULL;

        shilft_files();
        _write_count = 0;
    }

    return 0;

#endif
}

int
CLog::shilft_files()
{
    struct stat stStat;
    char sLogFileName[256];
    char sNewLogFileName[256];
    int i;

    sprintf(sLogFileName, "%s.log", _log_base_path);

    if (stat(sLogFileName, &stStat) < 0) {
        sprintf(_error_text, "stat %s: %s", sLogFileName, strerror(errno));
        return -1;
    }

    if (stStat.st_size < _max_log_size) {
        return 0;
    }

    sprintf(sLogFileName, "%s%d.log", _log_base_path, _max_log_num - 1);
    if (access(sLogFileName, F_OK) == 0) {
        if (remove(sLogFileName) < 0) {
            sprintf(_error_text, "remove %s: %s",
                    sLogFileName, strerror(errno));
            return -1;
        }
    }

    for (i = _max_log_num - 2; i >= 0; i--) {
        if (i == 0) {
            sprintf(sLogFileName, "%s.log", _log_base_path);
        }
        else {
            sprintf(sLogFileName, "%s%d.log", _log_base_path, i);
        }

        if (access(sLogFileName, F_OK) == 0) {
            sprintf(sNewLogFileName, "%s%d.log", _log_base_path, i + 1);
            if (rename(sLogFileName, sNewLogFileName) < 0) {
                sprintf(_error_text, "rename %s %s: %s",
                        sLogFileName, sNewLogFileName, strerror(errno));
                return -1;
            }
        }
    }

    return 0;
}

int
CLog::day_log(const char *day_log_path, const char *fmt, ...)
{
    assert(day_log_path != NULL);

    va_list ap;
    FILE *fp;
    char sLogFile[256];

    time_t timer;
    struct tm tm;
    timer = time(NULL);
    localtime_r(&timer, &tm);

    snprintf(sLogFile, sizeof(sLogFile), "%s_%04d%02d%02d.log",
             day_log_path, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    //以追加方式打开日志文件
    if ((fp = fopen(sLogFile, "a+")) == NULL) {
        write_log("fopen %s: %s\n", sLogFile, strerror(errno));
        return -1;
    }

    int ret = fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %d: ",
                      tm.tm_year + 1900, tm.tm_mon + 1,
                      tm.tm_mday, tm.tm_hour, tm.tm_min,
                      tm.tm_sec,
                      _proc_no);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    va_start(ap, fmt);
    ret = vfprintf(fp, fmt, ap);
    va_end(ap);

    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return 0;
}

int
CLog::day_log_raw(const char *day_log_path, const char *fmt, ...)
{
    assert(day_log_path != NULL);

    va_list ap;
    FILE *fp;
    char sLogFile[256];

    time_t timer;
    struct tm tm;
    timer = time(NULL);
    localtime_r(&timer, &tm);

    snprintf(sLogFile, sizeof(sLogFile), "%s_%04d%02d%02d.log",
             day_log_path, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    //以追加方式打开日志文件
    if ((fp = fopen(sLogFile, "a+")) == NULL) {
        write_log("fopen %s: %s\n", sLogFile, strerror(errno));
        return -1;
    }

    int ret;
    
    va_start(ap, fmt);
    ret = vfprintf(fp, fmt, ap);
    va_end(ap);

    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return 0;
}

char *
CLog::get_log_level_str(int log_level, char *str)
{
    switch (log_level) {
        case NO_LOG:
            strcpy(str, "no");
            break;

        case ERROR_LOG:
            strcpy(str, "error");
            break;

        case NORMAL_LOG:
            strcpy(str, "normal");
            break;

        case DEBUG_LOG:
            strcpy(str, "debug");
            break;

        default:
            strcpy(str, "unknown");
    }

    return str;
}

bool CLog::log_level_is_valid(int log_level)
{
    switch (log_level) {
        case NO_LOG:
        case ERROR_LOG:
        case NORMAL_LOG:
        case DEBUG_LOG:
            return true;
            break;
    }

    return false;
}
