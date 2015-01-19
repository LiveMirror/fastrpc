#ifndef _PB_EXT_H__
#define _PB_EXT_H__

#include <assert.h>
#include <stdlib.h>
#include <cstddef>
#include <string>
#include <string.h>
#if defined(__osf__)
// Tru64 lacks stdint.h, but has inttypes.h which defines a superset of
// what stdint.h would define.
#include <inttypes.h>
#elif !defined(_MSC_VER)
#include <stdint.h>
#endif

#include <google/protobuf/stubs/common.h>

namespace pbext {

template <typename Arg1, typename Arg2, typename Arg3>
class FunctionClosure3 : public ::google::protobuf::Closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

  FunctionClosure3(FunctionType function, bool self_deleting,
                   Arg1 arg1, Arg2 arg2, Arg3 arg3)
    : function_(function), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3) {}
  ~FunctionClosure3() {}

  void Run() {
    function_(arg1_, arg2_, arg3_);
    if (self_deleting_) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3>
class MethodClosure3 : public ::google::protobuf::Closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

  MethodClosure3(Class* object, MethodType method, bool self_deleting,
                 Arg1 arg1, Arg2 arg2, Arg3 arg3)
    : object_(object), method_(method), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3) {}
  ~MethodClosure3() {}

  void Run() {
    (object_->*method_)(arg1_, arg2_, arg3_);
    if (self_deleting_) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
};

template <typename Arg1, typename Arg2, typename Arg3>
::google::protobuf::Closure* NewCallback(void (*function)(Arg1, Arg2, Arg3),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new FunctionClosure3<Arg1, Arg2, Arg3>(
    function, true, arg1, arg2, arg3);
}

// See Closure.
template <typename Class, typename Arg1, typename Arg2, typename Arg3>
inline ::google::protobuf::Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2, Arg3),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new pbext::MethodClosure3<Class, Arg1, Arg2, Arg3>(
	  object, method, true, arg1, arg2, arg3);
}

}

#endif  // _PB_EXT_H__
