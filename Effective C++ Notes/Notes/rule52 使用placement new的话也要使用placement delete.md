# 使用placement new的话也要使用placement delete

---

当一个new表达式如下

```cpp
Widget* pw=new Widget;
```

这里共有两个函数被调用：分配内存的operator new和Widget的默认构造函数。假设第一个函数调用成功第二个抛出异常，这里第一个函数分配的内存必须取消并恢复旧观，否则会造成内存泄漏。这里客户没有能力归还内存，因为pw尚未赋值。

运行期系统会调用第一步调用的`operator new`对应版本的`operator delete`版本，前提是他知道应该调用哪一个版本（可能有多个）

正常的`operator new`与对应的`operator delete`

```cpp
void* operator new(std::size_t)throw(std::bad_alloc);

void operator delete(void* rawMemory)throw();   // global作用域中的签名式
void operator delete(void* rawMemory, std::size_t size)throw();     // class作用域中典型的签名式
```

当你只使用正常形式的`new`和`delete`，运行期系统可以找到配对的组合，而当你声明非正常形式的`operator new`，就要注意配对问题

```cpp
class Widget{
public:
    static void* operator new(std::size_t size, std::ostream& logStream)throw(std::bad_alloc);
    static void operator delete(void* pMemory, std::size_t size)throw();
    ...
};
```

这个设计有问题，首先看一些术语

如果`operator new`接受的参数除了一定会有的size_t之外还有其他，便称为一个placement new。众多placement版本中特别有用的是，接受一个指针指向该对象被构造之处

```cpp
void* operator new(std::size_t, void* pMemory) throw();
```

这个版本的new已经被纳入C++标准程序库，只要`#include<new>`就可以使用，其用途之一是在`vector`的未使用空间上创建对象

回到Widget的声明，这里Widget将引起微妙的内存泄漏

```cpp
Widget* pw=new(std::cerr) Widget;
```

如果内存分配成功，Widget构造函数抛出异常，运行期系统就有责任取消`operator new`的分配并恢复旧观，然鹅运行期系统无法知道真正被调用的`operator new`如何运作，因此无法取消分配并恢复旧观；取而代之的是寻找某个参数个数和类型斗鱼`operator new`相同的某个`operator delete`，如果找得到就调用

所以上述`operator new`对应的delete应该是

```cpp
void operator delete(void*, std::ostream&)throw();
```

这是个placement delete。现在找不到对应版本的delete，所以运行期系统什么都不做，导致内存泄漏

如果没有抛出异常，而客户代码中有个对应的`delete`，此时会调用正常形式的`operator delete`而不是placement delete

这意味着我们需要同时提供一个正常的`operator delete`和一个placement版本（用于构造期间有异常抛出），后者额外参数必须和placement版本的`operator new`额外参数相同

---

此外，由于成员函数名称会掩盖外层作用域名称，必须小心避免`class`专属`new`掩盖客户期望的其他news（包括正常版本）

```cpp
class Base{
public:
    static void* operator new(std::size_t size, std::ostream& logStream)throw(std::alloc);  // 掩盖正常的global形式new
    ...
};

Base* pb=new Base;  // error
Base* pb=new(std::cerr) Base;   // correct
```

同理，derived class中的operator news会掩盖global版本和继承到的operator news版本

```cpp
class Derived: public Base{
public:
    static void* operator new(std::size_t size)throw(std::bad_alloc);
    ...
};

Derived* pd=new(std::clog) Derived; // error
Derived* pd=new Derived;            // 没问题
```

需要记住的是，缺省情况下C++在global作用域内提供以下形式的operator new

```cpp
void* operator new(std::size_t) throw(std::bad_alloc);
void* operator new(std::size_t, void*)throw();      // placement
void* operator new(std::size_t, const std::nothrow_t&)throw();      // 旧式new
```

在`class`内声明任何operator news都会掩盖上述标准形式，除非你就是要阻止客户使用这些形式，否则请确保他们不被掩盖

一个做法是建立一个基类，内含所有正常形式的new和delete

```cpp
class StandardNewDelereForms{
public:
    // normal
    static void* operator new(std::size_t size)throw(std::bad_alloc){
        return ::operator new(size);
    }
    static void operator delete(void* pMemory)throw(){
        ::operator delete(pMemory);
    }
    // placement
    static void* operator new(std::size_t size, void* ptr)throw(){
        return ::operator new(size, ptr);
    }
    static void operator delete(void* pMemory, void* ptr)throw(){
        ::operator delete(pMemory, ptr);
    }
    // nothrow
    static void* operator new(std::size_t size, const std::nothrow_t& nt)throw(std::bad_alloc){
        return ::operator new(size);
    }
    static void operator delete(void* pMemory, const std::nothrow_t& nt)throw(){
        ::operator delete(pMemory);
    }
}
```

如果想要用自定义形式扩充标准形式，就可以通过继承和using声明（避免遮掩名称）实现

```cpp
class Widget: public StandardNewDelereForms{
public:
    using StandardNewDelereForms::operator new;
    using StandardNewDelereForms::operator delete;

    // 添加自定义new
    static void* operator new(std::size_t size, std::ostream& logStream)throw(std::bad_alloc);
    static void operator delete(void* pMemory, std::ostream& logStream)throw();

    ...
};
```

---

# 请记住

- 如果定义一个placement operator new，请确保也写出了相应的placement operator delete；两者的额外参数数量和类型必须完全相同
- 声明placement new和delete后确保不要无意识地掩盖正常版本