// 2010-12-02
// xcore_noncopyable.h
// 
// 不可拷贝类基类


#ifndef _XCORE_NONCOPYABLE_H_
#define _XCORE_NONCOPYABLE_H_

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// interface XNoncopyable
///////////////////////////////////////////////////////////////////////////////
class XNoncopyable
{
protected:
	XNoncopyable() {}
	~XNoncopyable() {}

private:
	XNoncopyable(const XNoncopyable&);
	XNoncopyable& operator=(const XNoncopyable&);
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_NONCOPYABLE_H_
