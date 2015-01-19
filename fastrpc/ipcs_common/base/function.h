/// Jack
/// Description: generic function object and bind

#ifndef IPCS_COMMON_BASE_FUNCTION_H
#define IPCS_COMMON_BASE_FUNCTION_H
#pragma once

// 基于引用计数的通用函数对象和绑定函数
//
// Function 类模板是一个通用的函数对象实现。和 Bind 函数一起使用，提供了类型安
// 全的方式进行参数的科里化，以及创建闭包。
//
//
// 内存管理和传递
//
// Function 对象本身应该以 const 引用的方式传递，并以拷贝的方式存储。再其内部
// 通过引用计数管理存储状态，因此不需要显式地删除。
//
// 通过 const 引用传递的原因是为了避免不必要的引用计数增减带来的额外开销，
//
// 使用示例:
//
// // 最简单的例子，绑定一个一般的函数：
// int Return5() { return 5; }
// Function<int(int)> func_cb = Bind(&Return5);
// cout << func_cb(5);  // 输出 5.
//
// void PrintHi() { cout << "hi."; }
// Function<void ()> void_func_cb = Bind(&PrintHi);
// cout << void_func_cb();  // 输出: hi.
//
//
// // 常用的场合：绑定类方法，用作回调时很有用。
// class SomeClass {
//  public:
//   int Foo() { return 4; }
//   void PrintWhy() { cout << "why???"; }
// };
// SomeClass object;
// Function<int(void)> cb =
//     Bind(&SomeClass::Foo, &object);
// cout << cb();  // 输出 4.
//
// Function<void ()> void_cb = Bind(&SomeClass::PrintWhy, &object);
// void_cb();  // 输出 why???
//
// // 注意: 必须确保函数对象所绑定的对象的生存期。否则运行时就会出问题，
// //       崩溃是最好的结局。
// // 有时候，让你的类支持引用计数是一个办法。
//
// // 绑定支持引用计数的类的方法.
// // 类需要从 RefCountedBase 模板基类派生，并把自己的类名作为模板参数。
//
// #include "red_counted.hpp"
//
// class Ref : public RefCountedBase<Ref>
// {
// public:
//     int Foo() { return 3; }
//     void PrintBye() { cout << "bye."; }
// };
//
// scoped_refptr<Ref> ref = new Ref();
// Function<int(void)> ref_cb = Bind(&Ref::Foo, ref.get());
// cout << ref_cb();  // 输出 3.
//
// Function<void ()> void_ref_cb = Bind(&Ref::PrintBye, ref.get());
// void_ref_cb();  // 输出: bye.
// // 每个 Function 对象都持有对绑定的对象的一个引用，生存期也受其影响。
//
//
// // 对 Function 对象再次 Bind：
// int Add(int x, int y)
// {
//     return x + y;
// }
//
// Function<int (int x, int y)> add = Bind(Add);
// cout << add(1, 2); // 输出 3
//
// // 再次绑定
// Function<int (int x)> inc = Bind(add, 1);
// cout << inc(1);    // 输出 2
//
// // 进一步绑定
// Function<int ()> two = Bind(inc, 1);
// cout << two();     // 输出 2
//
//
// // 改变传参数时的行为：
// // 默认情况下，参数以值的方式绑定，但是也可以通过 ConstRef 修饰函数，强制以
// // 引用方式绑定参数：
// int Identity(int n) { return n; }
// int value = 1;
// Function<int(void)> bound_copy_cb = Bind(&Identity, value);
// Function<int(void)> bound_ref_cb = Bind(&Identity, ConstRef(value));
// cout << bound_copy_cb();  // 输出 1.
// cout << bound_ref_cb();   // 输出 1.
// value = 2;                // 把外面的 value 改为 2
// cout << bound_copy_cb();  // 输出 1.
// cout << bound_ref_cb();   // 输出 2 了，说明 bound_ref_cb 按引用绑定了参数.
//
// Unretained() 则可以用于绑定到支持引用计数的对象上时，不去加减引用计数，确保
// 生存期时没问题时，可以略微提高一些性能。
//
// 其他操作，大部分跟 tr1::function 用法一样：
//
// Function<void ()> f, g;
// g = f;              // 赋值
// f = NULL;           // 赋值为空
// if (f == g) { ... } // 判断是否指向同一个实体
// if (f) { ... }      // 判断非空
// if (!f) { ... }     // 判断为空
//
// 实现：
//
// 由三部分组成:
//   1) Function 类模板.
//   2) Bind() 函数.
//   3) 参数包装 (例如 Unretained() 和 ConstRef()).
//
// Function 类代表一个通用的函数指针。在内部，存储了带引用基数的内部状态，包括
// 目标函数和所有绑定了的参数。
//
// Bind 函数则创建这样的状态，用来构建 Function 对象。
//
// Bind() 默认会存储所有参数对象的拷贝，并且会尝试增加被绑定的对象的引用计数，
//
// 为了允许改变这些行为，定义了一些参数的修饰函数，比如 Unretained() 和
// ConstRef())。
//
// ConstRef() 类似于 tr1::cref.  Unretained() 是 Function 特有的.
//
//
// 与 C++ 标准库的 tr1::function/bind 的差异:
//
// tr1::function 和 tr1::bind 按值绑定参数，也按值深拷贝，而不是采用引用计数。
// 当被绑定的参数比较“重”的时候，比如一个长的 string 甚至 map<>，function 对象
// 每被拷贝一次，这些参数就也跟着复制一次。
//
// 如果是将来的 C++0x（目前看来是 0x0B），支持右值引用，可以减少拷贝。不过终究
// 不能避免，至少传递结束，最后存下来的时候。
//
// 不过 tr1::function 的实现也有一个优点，就是当绑定的参数很少时，总大小不超过
// 特定的大小时，信息直接放在 function 对象内，不需要分配动态内存。如果我们用
// tcmalloc，分配动态内存的开销也不是问题。
//
// 用 valgrind 在 GCC 上测试显示，调用一次 tr1::bind，会对每个参数拷贝三次。当
// 按值传递 function 对象时，每个参数又都被拷贝了一次。而如果使用 Function，即
// 使按值传递，比起按 const 引用传递，也只是多了一些不必要的引用计数维护的开销
// 而已。
//
// 另外，当 Function 绑定到类方法时，如果这个类对象支持引用计数，就会自动增加
// 它的引用计数，对于简化资源管理还是很有意义的。tr1::function 没有类似的特性。
//
//
// 与 Closure 的差异：
//
// 我们已经有了一个 Closure，可以提供最基本的绑定的功能。但是 Closure 不支持引
// 用计数，只支持调用后自动删除和人工删除。自动删除的模式，需要追踪是否已经调
// 过。而人工删除的则需要显式 delete（或者搭配 scoped_ptr）。有些接口需要自动
// 删除的 closure，有些则相反，如果传错了类型，编译期间也无法发现，只能靠运行
// 期检查。并且 NewClosure 对参数类型匹配的要求完全匹配，否则引起的编译错误信
// 息很难看懂。
//
// Closure 的参数不支持形参，而 Function 支持，有利于提高代码的可读性：
// Closure<void, int, int>* done;
// const Closure<void (int error_code, int tranfered_length)>& done;
//
// 另外，Closure 不支持对再次绑定。也不能作为函数对象传给 STL 算法。也不支持绑
// 定 const 引用。

#include "function_in.h"
#include "bind.h"

// base 命名空间用在实现中避免和别的库链接冲突，但是使用时不加命名空间。
//
// C++ 不支持在头文件中的 using namespce 只作用于本文件而不传播给包含它的其他
// 文件。所以在 C++ 中无法做到把名字放在很深的命名空间中而对使用者影响不大。
// 否则命名空间深了的话，用起来很痛苦。比如 Thread 如果要按目录加命名空间的话，
// 在头文件中要写 common::base::system::concurrency::Thread，会让人疯掉的。
// 所以不如在接口中统一放开命名空间，而不允许使用者自己在头文件中 using。
// 这样用起来既方便，也不容易造成混乱，而且遇到名名冲突时，也能很容易地显式地
// 加上命名空间而解决。
using ::common::Function;
using ::common::Bind;
using ::common::ConstRef;
using ::common::Unretained;

#endif // IPCS_COMMON_BASE_FUNCTION_H
