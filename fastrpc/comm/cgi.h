
//For GET method : max name and val len is CGI_PARA_LEN(256)

#if !defined(CGI_HPP)
#define CGI_HPP
//#include "basetools.h"
#include <sys/io.h>

#include <stdio.h>

typedef struct
{
	char *name;
	char *val;
}input;

struct CGIEnv
{
	char *authType;
	int  contentLength;
	char *contentType;
	char *gatewayInterface;
	char *httpAccept;
	char *httpHost;
	char *httpReferer;
	char *httpUserAgent;
	char *pathInfo;
	char *pathTranslated;
	char *queryString;
	char *remoteAddr;
	char *remoteHost;
	char *remoteIdent;
	char *remoteUser;
	char *requestMethod;
	char *scriptName;
	char *serverName;
	char *serverPort;
	char *serverProtocol;
	char *serverSoftware;
};


class CGI
{
	public:
		input input_data[50];	//	name + val
		char *cookie;		//	cookie
		char *upload_filename;	//	multipart/form-data file place
		int has_checkbox;	//	whether has checkobx or mulitple
		int var_count;  //query variable number
		int method_number;
		int method;
		char *method_string;
		char *query;
		char *password;
		struct CGIEnv cgiEnv;
		char *remote_user; 
	public:
		//char m_szBuffer[1024*20];
		char *m_szBuffer;
	public:
		CGI();
		CGI(char *pData);
		CGI(int i);
		~CGI();

	private:
		char *ReadStdin(FILE *f,char stop);
		void Convert(char *url);
		char *ReadData(char *line,char stop);
		char ConvertToHex(char *change);
		void print_java_err(char *s);
	public:
		char* FindValue(const char *s);
		char* operator[](const char *name);
		void PrintHTTPHead();
		void PrintHTTPHead(int nMsgLength);
		char *GetData();
	protected:
		void GetMethod(); //get=1,post=2,null=0
		void GetInput();
		int GetInputData();
		int GetRemoteUser(); //有用户名时返回1，否则返回0
};

extern  void AddToSpace(char *str);

#endif
