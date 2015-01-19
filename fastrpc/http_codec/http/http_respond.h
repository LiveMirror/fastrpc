#include <iostream>
#include <string>

#ifndef  HTTP_RESPOND_H
#define HTTP_RESPOND_H


using namespace std;

class Chttp_simple_rsp{
public:
	static Chttp_simple_rsp *  instance();
	void NotFound(string &strrsp, int error);
	void NormalResponse(string &strrsp, const string& http_body, const char *body_type, unsigned mode = 0);
	void RedirectResponse(string &strrsp, const string& location);
	void NotModified(string &strrsp);
      void Forbidden(string &strrsp, const string& http_body, const char *body_type);
      void BadRequest(string &strrsp, int error);
      void ServerBusy(string & strrsp);


	~Chttp_simple_rsp();

protected:
	Chttp_simple_rsp(){};
private:
	static Chttp_simple_rsp * _instance;

};


#define    HTTP_RSP       (Chttp_simple_rsp::instance())  



#endif





