#ifndef logfile_h
#define logfile_h

#define DEFAULTLOGFILENAME "| /usr/local/apache-1.3.31/bin/rotatelogs /www/log//alllog.%Y%m%d.log 86400"
#define MAX_LOG_SIZE 4096

extern void debug_log (const char *msgfmt, ...);
extern char _logfilename[1024];

#endif
