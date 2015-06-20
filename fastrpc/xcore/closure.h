// Copyright (c) 2013, tencent Inc.
// All rights reserved.
//
// Author: jack
// Created: 01/26/13
// Description:  A closure (also lexical closure or function closure) is a

#ifndef IPCS_COMMON_BASE_CLOSURE_H
#define IPCS_COMMON_BASE_CLOSURE_H
#pragma once

template <bool Enabled, typename T>
class AutoDeleter
{
public:
    AutoDeleter(T *object_pointer) : m_p(object_pointer) {}

    virtual ~AutoDeleter()
    {
        if (Enabled) {
            delete m_p;
            m_p = 0;
        }
    }

    bool IsAutoDelete()
    {
        return Enabled;
    }

private:
    T *m_p;
};

// base closure class template
class ClosureBase
{
public:
    virtual ~ClosureBase() {}
    virtual bool IsSelfDelete() const = 0;
};

// base closure template, sets a trap to
// let class to be overridden.
template <typename R,
          typename Arg1 = void,
          typename Arg2 = void,
          typename Arg3 = void,
          typename Arg4 = void,
          typename Arg5 = void,
          typename Arg6 = void,
          typename Arg7 = void,
          typename Arg8 = void,
          typename Arg9 = void,
          typename Arg10 = void,
          typename Arg11 = void,
          typename Arg12 = void,
          typename Arg13 = void,
          typename Arg14 = void>
class Closure : public ClosureBase
{
public:
    virtual R Run(
        Arg1 a1,
        Arg2 a2,
        Arg3 a3,
        Arg4 a4,
        Arg5 a5,
        Arg6 a6,
        Arg7 a7,
        Arg8 a8,
        Arg9 a9,
        Arg10 a10,
        Arg11 a11,
        Arg12 a12,
        Arg13 a13,
        Arg14 a14) = 0;
};

// we just provide at most 7 arguments for a class method,
// please refer to the coding style guide for the reason.
template <typename R>
class Closure<R> : public ClosureBase
{
public:
    virtual R Run() = 0;
};

template <typename R,
          typename Arg1>
class Closure<R, Arg1> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2>
class Closure<R, Arg1, Arg2> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Closure<R, Arg1, Arg2, Arg3> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Closure<R, Arg1, Arg2, Arg3, Arg4> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) = 0;
};

template <typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7> : public ClosureBase
{
public:
    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7) = 0;
};

// class method closure
template <bool SelfDelete,
          typename R,
          typename Class>
class Method_Closure_Bind0_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)();

public:
    Method_Closure_Bind0_Arg0(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg0> auto_deleter(this);
        return (m_cls->*m_method)();
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1>
class Method_Closure_Bind1_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1);

public:
    Method_Closure_Bind1_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2>
class Method_Closure_Bind2_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2);

public:
    Method_Closure_Bind2_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
class Method_Closure_Bind3_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3);

public:
    Method_Closure_Bind3_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
class Method_Closure_Bind4_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4);

public:
    Method_Closure_Bind4_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
class Method_Closure_Bind5_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5);

public:
    Method_Closure_Bind5_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
class Method_Closure_Bind6_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6);

public:
    Method_Closure_Bind6_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
class Method_Closure_Bind7_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7);

public:
    Method_Closure_Bind7_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Method_Closure_Bind7_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind7_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
class Method_Closure_Bind8_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8);

public:
    Method_Closure_Bind8_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Method_Closure_Bind8_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind8_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
class Method_Closure_Bind9_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9);

public:
    Method_Closure_Bind9_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Method_Closure_Bind9_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind9_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
class Method_Closure_Bind10_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10);

public:
    Method_Closure_Bind10_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10)
    {}

    virtual ~Method_Closure_Bind10_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind10_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
class Method_Closure_Bind11_Arg0 : public Closure<R>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11);

public:
    Method_Closure_Bind11_Arg0(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10,
                              PreArg11 arg11) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10),
                              m_arg11(arg11)
    {}

    virtual ~Method_Closure_Bind11_Arg0()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind11_Arg0> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10, m_arg11);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
    PreArg11 m_arg11;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1>
class Method_Closure_Bind0_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1);

public:
    Method_Closure_Bind0_Arg1(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1>
class Method_Closure_Bind1_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2);

public:
    Method_Closure_Bind1_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
class Method_Closure_Bind2_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3);

public:
    Method_Closure_Bind2_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
class Method_Closure_Bind3_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4);

public:
    Method_Closure_Bind3_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
class Method_Closure_Bind4_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5);

public:
    Method_Closure_Bind4_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
class Method_Closure_Bind5_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6);

public:
    Method_Closure_Bind5_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
class Method_Closure_Bind6_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7);

public:
    Method_Closure_Bind6_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
class Method_Closure_Bind7_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8);

public:
    Method_Closure_Bind7_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Method_Closure_Bind7_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind7_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
class Method_Closure_Bind8_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9);

public:
    Method_Closure_Bind8_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Method_Closure_Bind8_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind8_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
class Method_Closure_Bind9_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10);

public:
    Method_Closure_Bind9_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Method_Closure_Bind9_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind9_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
class Method_Closure_Bind10_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11);

public:
    Method_Closure_Bind10_Arg1(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10)
    {}

    virtual ~Method_Closure_Bind10_Arg1()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind10_Arg1> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind0_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2);

public:
    Method_Closure_Bind0_Arg2(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind1_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3);

public:
    Method_Closure_Bind1_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind2_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4);

public:
    Method_Closure_Bind2_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind3_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5);

public:
    Method_Closure_Bind3_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind4_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6);

public:
    Method_Closure_Bind4_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind5_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7);

public:
    Method_Closure_Bind5_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind6_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8);

public:
    Method_Closure_Bind6_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind7_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9);

public:
    Method_Closure_Bind7_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Method_Closure_Bind7_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind7_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind8_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10);

public:
    Method_Closure_Bind8_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Method_Closure_Bind8_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind8_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
class Method_Closure_Bind9_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11);

public:
    Method_Closure_Bind9_Arg2(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Method_Closure_Bind9_Arg2()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind9_Arg2> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind0_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

public:
    Method_Closure_Bind0_Arg3(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind1_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4);

public:
    Method_Closure_Bind1_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind2_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5);

public:
    Method_Closure_Bind2_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind3_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6);

public:
    Method_Closure_Bind3_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind4_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7);

public:
    Method_Closure_Bind4_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind5_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8);

public:
    Method_Closure_Bind5_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind6_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9);

public:
    Method_Closure_Bind6_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind7_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10);

public:
    Method_Closure_Bind7_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Method_Closure_Bind7_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind7_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Method_Closure_Bind8_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11);

public:
    Method_Closure_Bind8_Arg3(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Method_Closure_Bind8_Arg3()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind8_Arg3> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind0_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

public:
    Method_Closure_Bind0_Arg4(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind1_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5);

public:
    Method_Closure_Bind1_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind2_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6);

public:
    Method_Closure_Bind2_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind3_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7);

public:
    Method_Closure_Bind3_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind4_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8);

public:
    Method_Closure_Bind4_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind5_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9);

public:
    Method_Closure_Bind5_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind6_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10);

public:
    Method_Closure_Bind6_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Method_Closure_Bind7_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11);

public:
    Method_Closure_Bind7_Arg4(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Method_Closure_Bind7_Arg4()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind7_Arg4> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind0_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5);

public:
    Method_Closure_Bind0_Arg5(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind1_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6);

public:
    Method_Closure_Bind1_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind2_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7);

public:
    Method_Closure_Bind2_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind3_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8);

public:
    Method_Closure_Bind3_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind4_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9);

public:
    Method_Closure_Bind4_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind5_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10);

public:
    Method_Closure_Bind5_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Method_Closure_Bind6_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11);

public:
    Method_Closure_Bind6_Arg5(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Method_Closure_Bind6_Arg5()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind6_Arg5> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind0_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6);

public:
    Method_Closure_Bind0_Arg6(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind1_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7);

public:
    Method_Closure_Bind1_Arg6(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind2_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8);

public:
    Method_Closure_Bind2_Arg6(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind3_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9);

public:
    Method_Closure_Bind3_Arg6(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind4_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10);

public:
    Method_Closure_Bind4_Arg6(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Method_Closure_Bind5_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11);

public:
    Method_Closure_Bind5_Arg6(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Method_Closure_Bind5_Arg6()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind5_Arg6> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Method_Closure_Bind0_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7);

public:
    Method_Closure_Bind0_Arg7(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg7()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg7> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Method_Closure_Bind1_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8);

public:
    Method_Closure_Bind1_Arg7(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg7()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg7> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Method_Closure_Bind2_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9);

public:
    Method_Closure_Bind2_Arg7(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg7()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg7> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Method_Closure_Bind3_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10);

public:
    Method_Closure_Bind3_Arg7(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg7()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg7> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Method_Closure_Bind4_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11);

public:
    Method_Closure_Bind4_Arg7(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Method_Closure_Bind4_Arg7()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind4_Arg7> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Method_Closure_Bind0_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8);

public:
    Method_Closure_Bind0_Arg8(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg8()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg8> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Method_Closure_Bind1_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9);

public:
    Method_Closure_Bind1_Arg8(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg8()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg8> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Method_Closure_Bind2_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10);

public:
    Method_Closure_Bind2_Arg8(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg8()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg8> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Method_Closure_Bind3_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11);

public:
    Method_Closure_Bind3_Arg8(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Method_Closure_Bind3_Arg8()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind3_Arg8> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Method_Closure_Bind0_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9);

public:
    Method_Closure_Bind0_Arg9(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg9()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg9> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Method_Closure_Bind1_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10);

public:
    Method_Closure_Bind1_Arg9(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg9()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg9> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Method_Closure_Bind2_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11);

public:
    Method_Closure_Bind2_Arg9(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Method_Closure_Bind2_Arg9()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind2_Arg9> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
class Method_Closure_Bind0_Arg10 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10);

public:
    Method_Closure_Bind0_Arg10(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg10()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg10> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
class Method_Closure_Bind1_Arg10 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>
{
public:
    typedef R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11);

public:
    Method_Closure_Bind1_Arg10(Class *cls,
                              MethodType cls_method,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method),
                              m_arg1(arg1)
    {}

    virtual ~Method_Closure_Bind1_Arg10()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind1_Arg10> auto_deleter(this);
        return (m_cls->*m_method)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
class Method_Closure_Bind0_Arg11 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>
{
public:
    typedef R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11);

public:
    Method_Closure_Bind0_Arg11(Class *cls,
                              MethodType cls_method) :
                              m_selfDelete(SelfDelete),
                              m_cls(cls),
                              m_method(cls_method)
    {}

    virtual ~Method_Closure_Bind0_Arg11()
    {
        m_cls = 0;
        m_method = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
    {
        AutoDeleter<SelfDelete,
                    Method_Closure_Bind0_Arg11> auto_deleter(this);
        return (m_cls->*m_method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    Class *m_cls;
    MethodType m_method;
};

// helper functions
template <typename R,
          typename Class>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)()
                        )
{
    return new Method_Closure_Bind0_Arg0<true, R, Class
                                          >(object, MethodType);
}

template <typename R,
          typename Class>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)()
                        )
{
    return new Method_Closure_Bind0_Arg0<false, R, Class
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg0<true, R, Class,
                                           PreArg1
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg0<false, R, Class,
                                           PreArg1
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Method_Closure_Bind10_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Method_Closure_Bind10_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
Closure<R>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11)
{
    return new Method_Closure_Bind11_Arg0<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           PreArg11
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10,
                                            arg11);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
Closure<R>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11)
{
    return new Method_Closure_Bind11_Arg0<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           PreArg11
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10,
                                            arg11);
}

template <typename R,
          typename Class,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1)
                        )
{
    return new Method_Closure_Bind0_Arg1<true, R, Class,
                                           Arg1
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1)
                        )
{
    return new Method_Closure_Bind0_Arg1<false, R, Class,
                                           Arg1
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg1<true, R, Class,
                                           PreArg1,
                                           Arg1
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg1<false, R, Class,
                                           PreArg1,
                                           Arg1
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
Closure<R, Arg1>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Method_Closure_Bind10_Arg1<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Method_Closure_Bind10_Arg1<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           Arg1
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2)
                        )
{
    return new Method_Closure_Bind0_Arg2<true, R, Class,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2)
                        )
{
    return new Method_Closure_Bind0_Arg2<false, R, Class,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg2<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg2<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg2<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Method_Closure_Bind9_Arg2<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1,
                                           Arg2
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3)
                        )
{
    return new Method_Closure_Bind0_Arg3<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3)
                        )
{
    return new Method_Closure_Bind0_Arg3<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg3<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg3<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg3<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Method_Closure_Bind8_Arg3<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
                        )
{
    return new Method_Closure_Bind0_Arg4<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
                        )
{
    return new Method_Closure_Bind0_Arg4<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg4<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg4<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg4<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Method_Closure_Bind7_Arg4<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
                        )
{
    return new Method_Closure_Bind0_Arg5<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
                        )
{
    return new Method_Closure_Bind0_Arg5<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg5<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg5<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg5<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg5<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg5<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg5<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg5<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg5<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg5<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg5<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg5<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Method_Closure_Bind6_Arg5<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
                        )
{
    return new Method_Closure_Bind0_Arg6<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
                        )
{
    return new Method_Closure_Bind0_Arg6<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg6<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg6<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg6<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg6<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg6<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg6<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg6<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg6<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg6<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Method_Closure_Bind5_Arg6<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
                        )
{
    return new Method_Closure_Bind0_Arg7<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
                        )
{
    return new Method_Closure_Bind0_Arg7<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg7<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg7<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg7<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg7<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg7<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg7<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg7<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Method_Closure_Bind4_Arg7<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
                        )
{
    return new Method_Closure_Bind0_Arg8<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
                        )
{
    return new Method_Closure_Bind0_Arg8<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg8<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg8<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg8<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg8<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg8<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Method_Closure_Bind3_Arg8<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(object, MethodType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
                        )
{
    return new Method_Closure_Bind0_Arg9<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
                        )
{
    return new Method_Closure_Bind0_Arg9<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg9<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg9<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg9<true, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Method_Closure_Bind2_Arg9<false, R, Class,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(object, MethodType,
                                            arg1,
                                            arg2);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
                        )
{
    return new Method_Closure_Bind0_Arg10<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
                        )
{
    return new Method_Closure_Bind0_Arg10<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg10<true, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11),
                        PreArg1 arg1)
{
    return new Method_Closure_Bind1_Arg10<false, R, Class,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(object, MethodType,
                                            arg1);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>* NewClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
                        )
{
    return new Method_Closure_Bind0_Arg11<true, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10,
                                           Arg11
                                          >(object, MethodType);
}

template <typename R,
          typename Class,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>* NewPermanentClosure(Class *object,
                        R (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
                        )
{
    return new Method_Closure_Bind0_Arg11<false, R, Class,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10,
                                           Arg11
                                          >(object, MethodType);
}

template <bool SelfDelete,
          typename R>
class Function_Closure_Bind0_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)();

public:
    Function_Closure_Bind0_Arg0(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg0> auto_deleter(this);
        return (*m_function)();
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1>
class Function_Closure_Bind1_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1);

public:
    Function_Closure_Bind1_Arg0(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2>
class Function_Closure_Bind2_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2);

public:
    Function_Closure_Bind2_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
class Function_Closure_Bind3_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3);

public:
    Function_Closure_Bind3_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
class Function_Closure_Bind4_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4);

public:
    Function_Closure_Bind4_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
class Function_Closure_Bind5_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5);

public:
    Function_Closure_Bind5_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
class Function_Closure_Bind6_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6);

public:
    Function_Closure_Bind6_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
class Function_Closure_Bind7_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7);

public:
    Function_Closure_Bind7_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Function_Closure_Bind7_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind7_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
class Function_Closure_Bind8_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8);

public:
    Function_Closure_Bind8_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Function_Closure_Bind8_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind8_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
class Function_Closure_Bind9_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9);

public:
    Function_Closure_Bind9_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Function_Closure_Bind9_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind9_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
class Function_Closure_Bind10_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10);

public:
    Function_Closure_Bind10_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10)
    {}

    virtual ~Function_Closure_Bind10_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind10_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
class Function_Closure_Bind11_Arg0 : public Closure<R>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11);

public:
    Function_Closure_Bind11_Arg0(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10,
                              PreArg11 arg11) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10),
                              m_arg11(arg11)
    {}

    virtual ~Function_Closure_Bind11_Arg0()
    {
        m_function = 0;
    }

    virtual R Run()
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind11_Arg0> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10, m_arg11);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
    PreArg11 m_arg11;
};

template <bool SelfDelete,
          typename R,
          typename Arg1>
class Function_Closure_Bind0_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(Arg1 arg1);

public:
    Function_Closure_Bind0_Arg1(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg1> auto_deleter(this);
        return (*m_function)(arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1>
class Function_Closure_Bind1_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2);

public:
    Function_Closure_Bind1_Arg1(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
class Function_Closure_Bind2_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3);

public:
    Function_Closure_Bind2_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
class Function_Closure_Bind3_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4);

public:
    Function_Closure_Bind3_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
class Function_Closure_Bind4_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5);

public:
    Function_Closure_Bind4_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
class Function_Closure_Bind5_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6);

public:
    Function_Closure_Bind5_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
class Function_Closure_Bind6_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7);

public:
    Function_Closure_Bind6_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
class Function_Closure_Bind7_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8);

public:
    Function_Closure_Bind7_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Function_Closure_Bind7_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind7_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
class Function_Closure_Bind8_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9);

public:
    Function_Closure_Bind8_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Function_Closure_Bind8_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind8_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
class Function_Closure_Bind9_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10);

public:
    Function_Closure_Bind9_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Function_Closure_Bind9_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind9_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
class Function_Closure_Bind10_Arg1 : public Closure<R, Arg1>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11);

public:
    Function_Closure_Bind10_Arg1(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9,
                              PreArg10 arg10) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9),
                              m_arg10(arg10)
    {}

    virtual ~Function_Closure_Bind10_Arg1()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind10_Arg1> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, m_arg10, arg1);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
    PreArg10 m_arg10;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind0_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2);

public:
    Function_Closure_Bind0_Arg2(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg2> auto_deleter(this);
        return (*m_function)(arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind1_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3);

public:
    Function_Closure_Bind1_Arg2(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind2_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4);

public:
    Function_Closure_Bind2_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind3_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5);

public:
    Function_Closure_Bind3_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind4_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6);

public:
    Function_Closure_Bind4_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind5_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7);

public:
    Function_Closure_Bind5_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind6_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8);

public:
    Function_Closure_Bind6_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind7_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9);

public:
    Function_Closure_Bind7_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Function_Closure_Bind7_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind7_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind8_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10);

public:
    Function_Closure_Bind8_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Function_Closure_Bind8_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind8_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
class Function_Closure_Bind9_Arg2 : public Closure<R, Arg1, Arg2>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11);

public:
    Function_Closure_Bind9_Arg2(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8,
                              PreArg9 arg9) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8),
                              m_arg9(arg9)
    {}

    virtual ~Function_Closure_Bind9_Arg2()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind9_Arg2> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, m_arg9, arg1, arg2);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
    PreArg9 m_arg9;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind0_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

public:
    Function_Closure_Bind0_Arg3(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg3> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind1_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4);

public:
    Function_Closure_Bind1_Arg3(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind2_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5);

public:
    Function_Closure_Bind2_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind3_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6);

public:
    Function_Closure_Bind3_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind4_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7);

public:
    Function_Closure_Bind4_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind5_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8);

public:
    Function_Closure_Bind5_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind6_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9);

public:
    Function_Closure_Bind6_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind7_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10);

public:
    Function_Closure_Bind7_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Function_Closure_Bind7_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind7_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
class Function_Closure_Bind8_Arg3 : public Closure<R, Arg1, Arg2, Arg3>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11);

public:
    Function_Closure_Bind8_Arg3(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7,
                              PreArg8 arg8) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7),
                              m_arg8(arg8)
    {}

    virtual ~Function_Closure_Bind8_Arg3()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind8_Arg3> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, m_arg8, arg1, arg2, arg3);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
    PreArg8 m_arg8;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind0_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

public:
    Function_Closure_Bind0_Arg4(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg4> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind1_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5);

public:
    Function_Closure_Bind1_Arg4(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind2_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6);

public:
    Function_Closure_Bind2_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind3_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7);

public:
    Function_Closure_Bind3_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind4_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8);

public:
    Function_Closure_Bind4_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind5_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9);

public:
    Function_Closure_Bind5_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind6_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10);

public:
    Function_Closure_Bind6_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
class Function_Closure_Bind7_Arg4 : public Closure<R, Arg1, Arg2, Arg3, Arg4>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11);

public:
    Function_Closure_Bind7_Arg4(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6,
                              PreArg7 arg7) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6),
                              m_arg7(arg7)
    {}

    virtual ~Function_Closure_Bind7_Arg4()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind7_Arg4> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, m_arg7, arg1, arg2, arg3, arg4);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
    PreArg7 m_arg7;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind0_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5);

public:
    Function_Closure_Bind0_Arg5(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg5> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind1_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6);

public:
    Function_Closure_Bind1_Arg5(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind2_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7);

public:
    Function_Closure_Bind2_Arg5(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind3_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8);

public:
    Function_Closure_Bind3_Arg5(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind4_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9);

public:
    Function_Closure_Bind4_Arg5(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind5_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10);

public:
    Function_Closure_Bind5_Arg5(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
class Function_Closure_Bind6_Arg5 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11);

public:
    Function_Closure_Bind6_Arg5(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5,
                              PreArg6 arg6) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5),
                              m_arg6(arg6)
    {}

    virtual ~Function_Closure_Bind6_Arg5()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind6_Arg5> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6, arg1, arg2, arg3, arg4, arg5);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
    PreArg6 m_arg6;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind0_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6);

public:
    Function_Closure_Bind0_Arg6(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg6> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind1_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7);

public:
    Function_Closure_Bind1_Arg6(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg6> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind2_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8);

public:
    Function_Closure_Bind2_Arg6(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg6> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind3_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9);

public:
    Function_Closure_Bind3_Arg6(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg6> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind4_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10);

public:
    Function_Closure_Bind4_Arg6(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg6> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
class Function_Closure_Bind5_Arg6 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11);

public:
    Function_Closure_Bind5_Arg6(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4,
                              PreArg5 arg5) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4),
                              m_arg5(arg5)
    {}

    virtual ~Function_Closure_Bind5_Arg6()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind5_Arg6> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
    PreArg5 m_arg5;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Function_Closure_Bind0_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7);

public:
    Function_Closure_Bind0_Arg7(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg7()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg7> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Function_Closure_Bind1_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8);

public:
    Function_Closure_Bind1_Arg7(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg7()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg7> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Function_Closure_Bind2_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9);

public:
    Function_Closure_Bind2_Arg7(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg7()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg7> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Function_Closure_Bind3_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10);

public:
    Function_Closure_Bind3_Arg7(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg7()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg7> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
class Function_Closure_Bind4_Arg7 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11);

public:
    Function_Closure_Bind4_Arg7(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3,
                              PreArg4 arg4) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3),
                              m_arg4(arg4)
    {}

    virtual ~Function_Closure_Bind4_Arg7()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind4_Arg7> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, m_arg4, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
    PreArg4 m_arg4;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Function_Closure_Bind0_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8);

public:
    Function_Closure_Bind0_Arg8(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg8()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg8> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Function_Closure_Bind1_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9);

public:
    Function_Closure_Bind1_Arg8(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg8()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg8> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Function_Closure_Bind2_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10);

public:
    Function_Closure_Bind2_Arg8(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg8()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg8> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
class Function_Closure_Bind3_Arg8 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11);

public:
    Function_Closure_Bind3_Arg8(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2,
                              PreArg3 arg3) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2),
                              m_arg3(arg3)
    {}

    virtual ~Function_Closure_Bind3_Arg8()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind3_Arg8> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, m_arg3, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
    PreArg3 m_arg3;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Function_Closure_Bind0_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9);

public:
    Function_Closure_Bind0_Arg9(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg9()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg9> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Function_Closure_Bind1_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10);

public:
    Function_Closure_Bind1_Arg9(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg9()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg9> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
class Function_Closure_Bind2_Arg9 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11);

public:
    Function_Closure_Bind2_Arg9(
                              FunctionType function,
                              PreArg1 arg1,
                              PreArg2 arg2) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1),
                              m_arg2(arg2)
    {}

    virtual ~Function_Closure_Bind2_Arg9()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind2_Arg9> auto_deleter(this);
        return (*m_function)(m_arg1, m_arg2, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
    PreArg2 m_arg2;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
class Function_Closure_Bind0_Arg10 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10);

public:
    Function_Closure_Bind0_Arg10(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg10()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg10> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

template <bool SelfDelete,
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
class Function_Closure_Bind1_Arg10 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>
{
public:
    typedef R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11);

public:
    Function_Closure_Bind1_Arg10(
                              FunctionType function,
                              PreArg1 arg1) :
                              m_selfDelete(SelfDelete),
                              m_function(function),
                              m_arg1(arg1)
    {}

    virtual ~Function_Closure_Bind1_Arg10()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind1_Arg10> auto_deleter(this);
        return (*m_function)(m_arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
    PreArg1 m_arg1;
};

template <bool SelfDelete,
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
class Function_Closure_Bind0_Arg11 : public Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>
{
public:
    typedef R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11);

public:
    Function_Closure_Bind0_Arg11(
                              FunctionType function) :
                              m_selfDelete(SelfDelete),
                              m_function(function)
    {}

    virtual ~Function_Closure_Bind0_Arg11()
    {
        m_function = 0;
    }

    virtual R Run(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
    {
        AutoDeleter<SelfDelete,
                    Function_Closure_Bind0_Arg11> auto_deleter(this);
        return (*m_function)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
    }

    virtual bool IsSelfDelete() const
    {
         return m_selfDelete;
    }

private:
    bool m_selfDelete;
    FunctionType m_function;
};

// helper functions
template <
          typename R>
Closure<R>* NewClosure(
                        R (*FunctionType)()
                        )
{
    return new Function_Closure_Bind0_Arg0<true, R
                                          >(FunctionType);
}

template <
          typename R>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)()
                        )
{
    return new Function_Closure_Bind0_Arg0<false, R
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg0<true, R,
                                           PreArg1
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg0<false, R,
                                           PreArg1
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg0<true, R,
                                           PreArg1,
                                           PreArg2
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg0<false, R,
                                           PreArg1,
                                           PreArg2
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Function_Closure_Bind10_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Function_Closure_Bind10_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
Closure<R>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11)
{
    return new Function_Closure_Bind11_Arg0<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           PreArg11
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10,
                                            arg11);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename PreArg11>
Closure<R>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, PreArg11 arg11)
{
    return new Function_Closure_Bind11_Arg0<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           PreArg11
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10,
                                            arg11);
}

template <
          typename R,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(Arg1 arg1)
                        )
{
    return new Function_Closure_Bind0_Arg1<true, R,
                                           Arg1
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1)
                        )
{
    return new Function_Closure_Bind0_Arg1<false, R,
                                           Arg1
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg1<true, R,
                                           PreArg1,
                                           Arg1
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg1<false, R,
                                           PreArg1,
                                           Arg1
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
Closure<R, Arg1>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Function_Closure_Bind10_Arg1<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename PreArg10,
          typename Arg1>
Closure<R, Arg1>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10, Arg1 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, PreArg10 arg10)
{
    return new Function_Closure_Bind10_Arg1<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           PreArg10,
                                           Arg1
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9,
                                            arg10);
}

template <
          typename R,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2)
                        )
{
    return new Function_Closure_Bind0_Arg2<true, R,
                                           Arg1,
                                           Arg2
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2)
                        )
{
    return new Function_Closure_Bind0_Arg2<false, R,
                                           Arg1,
                                           Arg2
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg2<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg2<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg2<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename PreArg9,
          typename Arg1,
          typename Arg2>
Closure<R, Arg1, Arg2>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9, Arg1 arg10, Arg2 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, PreArg9 arg9)
{
    return new Function_Closure_Bind9_Arg2<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           PreArg9,
                                           Arg1,
                                           Arg2
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8,
                                            arg9);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3)
                        )
{
    return new Function_Closure_Bind0_Arg3<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3)
                        )
{
    return new Function_Closure_Bind0_Arg3<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg3<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg3<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg3<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename PreArg8,
          typename Arg1,
          typename Arg2,
          typename Arg3>
Closure<R, Arg1, Arg2, Arg3>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8, Arg1 arg9, Arg2 arg10, Arg3 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, PreArg8 arg8)
{
    return new Function_Closure_Bind8_Arg3<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           PreArg8,
                                           Arg1,
                                           Arg2,
                                           Arg3
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7,
                                            arg8);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
                        )
{
    return new Function_Closure_Bind0_Arg4<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
                        )
{
    return new Function_Closure_Bind0_Arg4<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg4<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg4<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg4<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename PreArg7,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4>
Closure<R, Arg1, Arg2, Arg3, Arg4>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7, Arg1 arg8, Arg2 arg9, Arg3 arg10, Arg4 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, PreArg7 arg7)
{
    return new Function_Closure_Bind7_Arg4<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           PreArg7,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6,
                                            arg7);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
                        )
{
    return new Function_Closure_Bind0_Arg5<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
                        )
{
    return new Function_Closure_Bind0_Arg5<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg5<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg5<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg5<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg5<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg5<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg5<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg5<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg5<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg5<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg5<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg5<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename PreArg6,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6, Arg1 arg7, Arg2 arg8, Arg3 arg9, Arg4 arg10, Arg5 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, PreArg6 arg6)
{
    return new Function_Closure_Bind6_Arg5<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           PreArg6,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5,
                                            arg6);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
                        )
{
    return new Function_Closure_Bind0_Arg6<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
                        )
{
    return new Function_Closure_Bind0_Arg6<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg6<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg6<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg6<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg6<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg6<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg6<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg6<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg6<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg6<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename PreArg5,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5, Arg1 arg6, Arg2 arg7, Arg3 arg8, Arg4 arg9, Arg5 arg10, Arg6 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, PreArg5 arg5)
{
    return new Function_Closure_Bind5_Arg6<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           PreArg5,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4,
                                            arg5);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
                        )
{
    return new Function_Closure_Bind0_Arg7<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
                        )
{
    return new Function_Closure_Bind0_Arg7<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg7<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg7<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg7<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg7<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg7<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg7<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg7<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename PreArg4,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4, Arg1 arg5, Arg2 arg6, Arg3 arg7, Arg4 arg8, Arg5 arg9, Arg6 arg10, Arg7 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, PreArg4 arg4)
{
    return new Function_Closure_Bind4_Arg7<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           PreArg4,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3,
                                            arg4);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
                        )
{
    return new Function_Closure_Bind0_Arg8<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
                        )
{
    return new Function_Closure_Bind0_Arg8<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg8<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg8<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg8<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg8<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg8<true, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename PreArg3,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, PreArg3 arg3, Arg1 arg4, Arg2 arg5, Arg3 arg6, Arg4 arg7, Arg5 arg8, Arg6 arg9, Arg7 arg10, Arg8 arg11),
                        PreArg1 arg1, PreArg2 arg2, PreArg3 arg3)
{
    return new Function_Closure_Bind3_Arg8<false, R,
                                           PreArg1,
                                           PreArg2,
                                           PreArg3,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8
                                          >(FunctionType,
                                            arg1,
                                            arg2,
                                            arg3);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
                        )
{
    return new Function_Closure_Bind0_Arg9<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
                        )
{
    return new Function_Closure_Bind0_Arg9<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg9<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg9<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg9<true, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename PreArg1,
          typename PreArg2,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, PreArg2 arg2, Arg1 arg3, Arg2 arg4, Arg3 arg5, Arg4 arg6, Arg5 arg7, Arg6 arg8, Arg7 arg9, Arg8 arg10, Arg9 arg11),
                        PreArg1 arg1, PreArg2 arg2)
{
    return new Function_Closure_Bind2_Arg9<false, R,
                                           PreArg1,
                                           PreArg2,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9
                                          >(FunctionType,
                                            arg1,
                                            arg2);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
                        )
{
    return new Function_Closure_Bind0_Arg10<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
                        )
{
    return new Function_Closure_Bind0_Arg10<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(FunctionType);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg10<true, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename PreArg1,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10>* NewPermanentClosure(
                        R (*FunctionType)(PreArg1 arg1, Arg1 arg2, Arg2 arg3, Arg3 arg4, Arg4 arg5, Arg5 arg6, Arg6 arg7, Arg7 arg8, Arg8 arg9, Arg9 arg10, Arg10 arg11),
                        PreArg1 arg1)
{
    return new Function_Closure_Bind1_Arg10<false, R,
                                           PreArg1,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10
                                          >(FunctionType,
                                            arg1);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>* NewClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
                        )
{
    return new Function_Closure_Bind0_Arg11<true, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10,
                                           Arg11
                                          >(FunctionType);
}

template <
          typename R,
          typename Arg1,
          typename Arg2,
          typename Arg3,
          typename Arg4,
          typename Arg5,
          typename Arg6,
          typename Arg7,
          typename Arg8,
          typename Arg9,
          typename Arg10,
          typename Arg11>
Closure<R, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11>* NewPermanentClosure(
                        R (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10, Arg11 arg11)
                        )
{
    return new Function_Closure_Bind0_Arg11<false, R,
                                           Arg1,
                                           Arg2,
                                           Arg3,
                                           Arg4,
                                           Arg5,
                                           Arg6,
                                           Arg7,
                                           Arg8,
                                           Arg9,
                                           Arg10,
                                           Arg11
                                          >(FunctionType);
}

#endif // IPCS_COMMON_BASE_CLOSURE_H
