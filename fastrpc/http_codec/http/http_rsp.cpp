#include "http_base_str.h"
#include "http_rsp.h"
#include "string.h"

using namespace std;
using namespace httpsvr;


//////////////////////////////////////////////////////////////////////////

void CHttpStatusLine::make_line()
{
	_line = "HTTP/" + to_str(_http_version_primary) + "."
		+ to_str(_http_version_seconday) + " " + to_str(_status_code) + "\r\n";
	return ;
}

int CHttpRsp::simple_rsp(unsigned status_code)
{
	_status_line._status_code = status_code;
	_status_line._http_version_primary = 1;
	_status_line._http_version_seconday = 0;
	_status_line.make_line();
	return 0;
}

int CHttpRsp::output(char* buffer, unsigned buf_size, unsigned& data_len)
{
	//	status_line
	_status_line.make_line();
	unsigned offset = 0;
	memcpy(buffer, _status_line._line.c_str(), _status_line._line.size());
	offset += _status_line._line.size();

	//	header
	data_len = 0;
	int ret = _header.output(buffer + offset, buf_size - offset, data_len);
	if (ret) return ret;
	offset += data_len;

	//	body
	memcpy(buffer + offset, _body._buf, _body._data_len);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///:~
