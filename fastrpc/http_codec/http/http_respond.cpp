#include "http_base_str.h"
#include "http_respond.h"
#include "stdio.h"
#include "string.h"


Chttp_simple_rsp * Chttp_simple_rsp::_instance = NULL;



Chttp_simple_rsp::~Chttp_simple_rsp()
{
	if(_instance)
	{
		delete _instance;
	}

}


Chttp_simple_rsp * Chttp_simple_rsp::instance()
{
	if(_instance == NULL)
	{
		_instance = new Chttp_simple_rsp;
	}

	return _instance;

}

void Chttp_simple_rsp::Forbidden(string &strrsp, const string& http_body, const char *body_type)
{
 	strrsp = "HTTP/1.1 403 Forbidden\r\nContent-Type: " + string(body_type) + "\r\nContent-Length: " + to_str(http_body.size()) + "\r\n\r\n" + http_body;
	//strrsp = "HTTP/1.1 500 Server Too Busy\r\nContent-Type: " + string(body_type) + "\r\nContent-Length: " + to_str(http_body.size()) + "\r\n\r\n" + http_body;
}


void Chttp_simple_rsp::NotFound(string &strrsp, int error)
{
    char rsp[200];
    snprintf(rsp, 200, "HTTP/1.1 404 Not Found\r\nServer: httpserver\r\nConnection:close\r\nContent-Length: 0\r\nerror:%d\r\n\r\n",error);
//    snprintf(rsp, 200, "HTTP/1.1 404 Not Found\r\nServer: httpserver\r\nConnection:close\r\nerror=%d\r\n\r\n",error);
    strrsp.assign(rsp, strlen(rsp));
}


void Chttp_simple_rsp::BadRequest(string &strrsp, int error)
{
    char rsp[200];
    snprintf(rsp, 200, "HTTP/1.1 400 Bad Request\r\nServer: httpserver\r\nConnection:close\r\nContent-Length: 0\r\nerror:%d\r\n\r\n", error);
    strrsp.assign(rsp, strlen(rsp));
}

void Chttp_simple_rsp::NormalResponse(string &strrsp, const string& http_body, const char *body_type, unsigned mode)
{
	if(mode == 0)
		strrsp = "HTTP/1.1 200 OK\r\nContent-Type: " + string(body_type) + "\r\nContent-Length: " + to_str(http_body.size()) + "\r\n\r\n" + http_body;
	else if(mode == 1)
		strrsp = "HTTP/1.1 200 OK\r\nServer: httpserver\r\nLast-Modified: Thu, 14 Dec 2006 07:25:14 GMT\r\nCache-Control: max-age=7200\r\nContent-Type: " + string(body_type) + "\r\nContent-Length: " + to_str(http_body.size()) + "\r\n\r\n" + http_body;
	else
		strrsp = "HTTP/1.1 200 OK\r\nServer: httpserver\r\nLast-Modified: Thu, 14 Dec 2006 07:25:14 GMT\r\nCache-Control: no-cache\r\nContent-Type: " + string(body_type) + "\r\nContent-Length: " + to_str(http_body.size()) + "\r\n\r\n" + http_body;
}

void Chttp_simple_rsp::RedirectResponse(string &strrsp, const string& location)
{
	strrsp = "HTTP/1.1 302 Moved Temporarily\r\nServer: httpserver\r\nLocation: " + location + "\r\nCache-Control: max-age=7200\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

}


void Chttp_simple_rsp::NotModified(string & strrsp)
{
	strrsp = "HTTP/1.1 304 Not Modified\r\nCache-Control: max-age=7200\r\nServer: httpserver\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

}

void Chttp_simple_rsp::ServerBusy(string & strrsp)
{
	strrsp = "HTTP/1.1 500 Server Too Busy\r\nCache-Control: max-age=7200\r\nServer: httpserver\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

}









