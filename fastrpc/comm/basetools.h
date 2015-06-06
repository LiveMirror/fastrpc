#ifndef _BASE_TOOLS_H_
#define _BASE_TOOLS_H_

#include "baseheader.h"

static const size_t log_buffer_len = 1024*20;

class BaseTools {
public:
    BaseTools(string pdir):file_is_open(false), nHandle(-1) {
        dir = pdir;
        if (dir.empty()) {
            dir = "./nodefine_log";
        }
        szLine = new char[log_buffer_len];
        szMsg = new char[log_buffer_len];
    }
    ~BaseTools() {
        delete []szLine;
        delete []szMsg;
    }
    void SetLogDir(string pdir);
    int WriteLog(char *pszFmt,...);
private:
    bool file_is_open;
    int nHandle;
    string last_filepath;
    string dir;
    char *szLine;
    char *szMsg;
};

int WriteDeBase64(char *pDir,char *pStr,int nLength);
void DateAdd(int day, char *all_data, char *day_data,
             char *all_format, char *day_format, int all_size, int day_size);
bool Replace_c(char *pBuff, char *pOldStr, char *pNewStr);
unsigned long GetTickCount();
#endif
