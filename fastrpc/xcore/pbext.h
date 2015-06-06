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

// 需要多少参数自己再补

// 3 args begine
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
// 3 args end
//
// 4 args begine
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
class FunctionClosure4 : public ::google::protobuf::Closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

  FunctionClosure4(FunctionType function, bool self_deleting,
                   Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    : function_(function), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3), arg4_(arg4) {}
  ~FunctionClosure4() {}

  void Run() {
    function_(arg1_, arg2_, arg3_, arg4_);
    if (self_deleting_) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
class MethodClosure4 : public ::google::protobuf::Closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

  MethodClosure4(Class* object, MethodType method, bool self_deleting,
                 Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    : object_(object), method_(method), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3), arg4_(arg4) {}
  ~MethodClosure4() {}

  void Run() {
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_);
    if (self_deleting_) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
};

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
::google::protobuf::Closure* NewCallback(void (*function)(Arg1, Arg2, Arg3, Arg4 arg4),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
  return new FunctionClosure4<Arg1, Arg2, Arg3, Arg4>(
    function, true, arg1, arg2, arg3, arg4);
}

// See Closure.
template <typename Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline ::google::protobuf::Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2, Arg3, Arg4),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
  return new pbext::MethodClosure4<Class, Arg1, Arg2, Arg3, Arg4>(
	  object, method, true, arg1, arg2, arg3, arg4);
}
// 4 args end

// 6 args begine
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
class FunctionClosure6 : public ::google::protobuf::Closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6);

  FunctionClosure6(FunctionType function, bool self_deleting,
                   Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    : function_(function), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5), arg6_(arg6) {}
  ~FunctionClosure6() {}

  void Run() {
    function_(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_);
    if (self_deleting_) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
class MethodClosure6 : public ::google::protobuf::Closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6);

  MethodClosure6(Class* object, MethodType method, bool self_deleting,
                 Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    : object_(object), method_(method), self_deleting_(self_deleting),
      arg1_(arg1), arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5), arg6_(arg6) {}
  ~MethodClosure6() {}

  void Run() {
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_);
    if (self_deleting_) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;
};

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
::google::protobuf::Closure* NewCallback(void (*function)(Arg1, Arg2, Arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) {
  return new FunctionClosure6<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
    function, true, arg1, arg2, arg3, arg4, arg5, arg6);
}

// See Closure.
template <typename Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
inline ::google::protobuf::Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6),
                            Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) {
  return new pbext::MethodClosure6<Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>(
	  object, method, true, arg1, arg2, arg3, arg4, arg5, arg6);
}
// 6 args end

}

#endif  // _PB_EXT_H__
