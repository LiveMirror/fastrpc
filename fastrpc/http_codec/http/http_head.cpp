#include "http_head.h"
#include "http_func.h"

using namespace std;
using namespace httpsvr;
using namespace httpsvr::func;

//////////////////////////////////////////////////////////////////////////

//解析一段data， 返回头部长度 skip_len
int CHttpHeader::infuse(const char* data, unsigned length, unsigned& skip_len)
{
	const char* p = data;
	skip_len = 0;
	
	static const unsigned C_MAX_HEAD_LINE = 1<<13;	//	max head line 8192
	for(unsigned i = 0; i < C_MAX_HEAD_LINE; i++)
	{
		//	first, get line from data
		unsigned line_len = 0, tail_len = 0;
		int ret = get_line(p, length, line_len, tail_len);
		if (ret)
		{
			return ret;
		}

		//	second, if no more line-feed, incomplete line data
		if (line_len == 0)
		{
			return 0;
		}
		
		skip_len += line_len;
		
		//	third, if empty line, mean the head finish
		if (line_len == tail_len)
		{
			_full = true;
			return 0;
		}

		//	offset, len, must be, reliable, otherwise ....
		unsigned name_len = 0, value_offset = 0, value_len = 0;
		ret = get_name_value(p, line_len-tail_len, name_len, value_offset
			, value_len);
		if (ret)
		{
			return 0;			//不完整的
			//return ret;		//可能有问题
		}
		//	make the upper name
		char* sName = new char[name_len+1];
		for(unsigned j = 0; j < name_len; j++)
		{
			sName[j] = toupper(p[j]);	//全转换成大写
		}
		sName[name_len] = 0;

		_head_attribute[sName] = string(p + value_offset, value_len);
		delete [] sName;
		p += line_len;
		//tencent-felixzhu-20080814,update buffer len to avoid buffer overflow,begin
		length -= line_len;
		//tencent-felixzhu-20080814,end
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///:~
