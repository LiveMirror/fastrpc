/// Jack
/// Description: scoped_refptr smart pointer

#ifndef IPCS_COMMON_BASE_SCOPED_REFPTR_H
#define IPCS_COMMON_BASE_SCOPED_REFPTR_H

#include <algorithm>
#include "ref_counted.h"

// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public RefCountedBase<MyFoo> {
//       ...
//   };
//
//   void some_function() {
//       scoped_refptr<MyFoo> foo = new MyFoo();
//       foo->Method(param);
//       // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//       scoped_refptr<MyFoo> foo = new MyFoo();
//       ...
//       foo = NULL;  // explicitly releases |foo|
//       ...
//       if (foo)
//           foo->Method(param);
//   }
//
// The above examples show how scoped_refptr<T> acts like a pointer to T.
// Given two scoped_refptr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//     {
//         scoped_refptr<MyFoo> a = new MyFoo();
//         scoped_refptr<MyFoo> b;
//
//         b.swap(a);
//         // now, |b| references the MyFoo object, and |a| references NULL.
//     }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//     {
//         scoped_refptr<MyFoo> a = new MyFoo();
//         scoped_refptr<MyFoo> b;
//
//         b = a;
//         // now, |a| and |b| each own a reference to the same MyFoo object.
//     }

// namespace common {

template <class T>
class scoped_refptr
{
public:
    scoped_refptr() : m_ptr(NULL)
    {
    }

    scoped_refptr(T* p) : m_ptr(p) // NOLINT(runtime/explicit)
    {
        if (m_ptr)
            m_ptr->AddRef();
    }

    scoped_refptr(const scoped_refptr<T>& r) : // NOLINT(runtime/explicit)
        m_ptr(r.m_ptr)
    {
        if (m_ptr)
            m_ptr->AddRef();
    }

    template <typename U>
    scoped_refptr(const scoped_refptr<U>& r) : // NOLINT(runtime/explicit)
        m_ptr(r.get())
    {
        if (m_ptr)
            m_ptr->AddRef();
    }

    ~scoped_refptr()
    {
        if (m_ptr)
            m_ptr->Release();
    }

    T* get() const { return m_ptr; }
    operator T*() const { return m_ptr; }
    T* operator->() const { return m_ptr; }

    // Release a pointer.
    // The return value is the current pointer held by this object.
    // If this object holds a NULL pointer, the return value is NULL.
    // After this operation, this object will hold a NULL pointer,
    // and will not own the object any more.
    T* release()
    {
        T* retVal = m_ptr;
        m_ptr = NULL;
        return retVal;
    }

    scoped_refptr<T>& operator=(T* p)
    {
        // AddRef first so that self assignment should work
        if (p)
            p->AddRef();

        if (m_ptr)
            m_ptr->Release();

        m_ptr = p;
        return *this;
    }

    scoped_refptr<T>& operator=(const scoped_refptr<T>& r)
    {
        return *this = r.m_ptr;
    }

    template <typename U>
    scoped_refptr<T>& operator=(const scoped_refptr<U>& r)
    {
        return *this = r.get();
    }

    void swap(T** pp)
    {
        T* p = m_ptr;
        m_ptr = *pp;
        *pp = p;
    }

    void swap(scoped_refptr<T>& r)
    {
        swap(&r.m_ptr);
    }

private:
    T* m_ptr;
};

// Handy utility for creating a scoped_refptr<T> out of a T* explicitly without
// having to retype all the template arguments
template <typename T>
scoped_refptr<T> make_scoped_refptr(T* t)
{
    return scoped_refptr<T>(t);
}

// } // namespace common

#endif // IPCS_COMMON_BASE_SCOPED_REFPTR_H
