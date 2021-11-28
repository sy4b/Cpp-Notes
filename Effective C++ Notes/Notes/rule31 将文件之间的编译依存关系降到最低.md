# 将文件之间的编译依存关系降到最低

假设对C++程序的某个`class`实现文件做了轻微修改。注意不是修改`class`接口而是实现，而且只修改`private`成分，然后重新建置这个程序，并预计只要几秒就好，毕竟只有一个`class`被修改。你按下了bulid按钮，然后大吃一惊，你意识到整个世界都被重新编译和链接啦！

问题出现在，C++并没有把”将接口从实现中分离“这件事做得很好。`class`的定义式不止详细叙述了`class`接口，还包括十足的实现细节

---

例如

```cpp
class Person{
public:
    Person(const std::string& name, const Date& birthday, const Address& addr);
    std::string name()const;
    std::string birthDate()const;
    std::string address()const;
    ...
private:
    std::string theName;        // 实现细目
    Date theBirthDate;          // 实现细目
    Address theAddress;         // 实现细目
};
```

这里的class Person无法通过编译——如果编译器没有取得其实现代码所用到的class：string，Date和Address的定义式。这样的定义式通常由`#include`指示符提供，所以Person定义文件的上方很可能存在这样的东西

```cpp
#include<string>
#include"date.h"
#include"address.h"
```

不幸的是，这样一来就在Person定义文件和其含入文件之间形成了一种编译依存关系。如果这些头文件中有任何一个被改变，或者所依赖的其他甜味剂有改变，那么每一个含入Person class的文件都得重新编译，任何使用的文件也得重新编译。这样的连串编译依存关系会对许多项目造成难以形容的灾难

你或许觉得奇怪，为什么C++坚持将`class`的实现细目放在`class`定义式之中？为什么不像下面这样分开

```cpp
namespace std{          // 前置声明（不正确，详下）
    class string;
}
class Date;
class Address;
class Person{
public:
    Person(const std::string& name, const Date& birthday, const Address& addr);
    std::string name()const;
    std::string birthDate()const;
    std::string address()const;
    ...
};
```

如果可以这么做，Person的客户就只需要在`Person`的接口被重新修改才需要重新编译

这个想法存在两个问题。第一，`string`不是一个`class`，他是个`typedef`（定义为`basic_string<char>`），因此上述前置声明并不正确；正确的前置声明比较复杂，设计额外的`template`，并且你本来就不该尝试手动声明一部分标准程序库，应该仅仅使用恰当的`#include`完成目的

第二，编译器必须在编译期间知道对象的大小。考虑

```cpp
int main(){
    int x;
    Person p(params);
    ...
}
```

编译器看到x，他知道必须分配多少内存才能持有一个`int`（通常在stack内），但看到p的定义，怎么知道分配多少空间呢？唯一办法是询问`class`定义式，然而如果`class`定义式不列出实现细目，编译器怎么知道分配多少空间呢？

这个问题在Java等语言上并不存在，因为当我们用那种语言定义对象时，编译器只分配足够空间给一个指针指向该对象使用。也就是他们将上述代码视作：

```cpp
int main(){
    int x;
    Person* p;
    ...
}
```

这当然也是合格的C++代码，所以你也可以针对Person这样做：把Person分割为两个`class`，一个只提供接口，另一个负责实现接口

```cpp
#include<string>
#include<memory>    // 为了tr1::shared_ptr含入

class PersonImpl;   // Person实现类的前置声明
class Date;
class Address;

class Person{
public:
    Person(const std::string& name, const Date& birthday, const Address& addr);
    std::string name()const;
    std::string birthDate()const;
    std::string address()const;
    ...
private:
    std::tr1::shared_ptr<PersonImpl> pImpl;
};
```

这里main class只内含一个指针成员，指向实现类。这种设计被称为pimpl idiom(pimpl是pointer to implementation的缩写)

这样的设计下，Person的客户就完全与Data，Address以及Person的实现细目完全分离了。那些`class`的任何实现修改都不需要Person客户端重新编译。此外客户无法看到Person的实现细目，也就不可能写出什么“取决于那些细目”的代码，真正实现了！接口与实现分离

这个分离的关键在于用“声明的依存性”替换“定义的依存性”，这正是编译依存性最小化的本质：现实中让头文件尽可能自我满足，万一做不到，则让他与其他文件内的声明式（而不是定义式）相依

其他的每一件事都源自这个简单的设计策略：

- 如果使用object reference或pointers可以完成任务，就不要使用objects
- 如果可以，尽量用`class`声明式替换`class`定义式。注意，当你声明一个函数而它用到某个`class`时，你并不需要`class`的定义，即使函数用by value的方式传递参数

```cpp
class Date;
Data today();
void clearAppointments(Date d);
```

- 为声明式和定义式提供不同的头文件。为了严守上述准则，需要两个头文件，一个用于声明式，一个用于定义式。当然这些文件必须保持一致性，如果有个声明式被改变，两个文件都得改变。因此程序库客户总是`#include`一个声明文件而不是前置若干函数，程序库作者也必须提供这两个文件

---

像Person这样使用pimpl idiom的classes，往往被称为handle classes。这样的classes如何真正做点事情呢？办法之一是将他们的所有函数交给相应的实现类并由后者实际完成工作

```cpp
#include "Person.h"     // 我们正在实现Person class
#include "PersonImpl.h" // 为了调用其成员函数

Person::Person(const std::string& name, const Date& birthday, const Address& addr): pImpl(new PersonImpl(name, birthday, addr)){}

std::string Person::name()const{
    return pImpl->name();
}
```

上述代码用`new`调用PersonImpl构造函数，以及Person::name函数内调用PersonImpl::name

另一个制作handle class的办法是，让Person成为一种特殊的abstract base class，成为interface class，这种`class`的目的是详细一一描述derived class的接口，因此通常不带成员变量，没有构造函数，只有一个`virtual`析构函数以及一组pure virtual函数，用来叙述整个接口

一个针对Person的Interface class或许看起来像这样

```cpp
class Person{
public:
    virtual ~Person();
    virtual std::string name()const=0;
    virtual std::string birthDate()const=0;
    virtual std::string address()const=0;
}
```

这个`class`的客户必须以Person的pointers和reference来编写程序，因为它不可能针对Person类具现出实体，但是可以对他的派生类具现出实体

类似Handle class的客户，除非Interface class的接口被修改，否则其客户不需要重新编译

Interface class的客户必须有办法为这种`class`创建新对象。通常调用一个函数，扮演“真正将被具现化”的那个derived class的构造函数角色。这样的函数常被称为工厂（factory）函数或`virtual`构造函数。他们返回指针指向动态分配所得对象，而该对象支持Interface class的接口，这样的函数往往又在Interface class内被声明为`static`

```cpp
class Person{
    ...
    static std::tr1::shared_ptr<Person> create(const std::string& name, const Date& birthday, const Address& addr);
    ...
};
```

客户会这样使用他们

```cpp
std::string name;
Date dateOfBirth;
Address address;
...
// 创建一个对象，支持Person接口
std::tr1::shared_ptr<Person> pp(Person::create(name, dateOfBirth, address));

...
std::cout<<pp->name();      // 通过Person的接口使用这个对象

...
// pp离开作用域，对象会被自动删除
```

当然，支持Interface class的接口的那个具象类必须被定义出来，且真正的构造函数必须被调用。一切都在`virtual`构造函数实现码所在的文件内秘密发生

假设Interface class Person有个具象的derived class RealPerson，后者提供继承而来的`virtual`函数的实现

```cpp
class RealPerson: public Person{
public:
    RealPerson(const std::string& name, const Date& birthday, const Address& addr): theName(name), theBirthDate(birthday), theAddress(addr){}
    virtual ~RealPerson(){}
    std::string name()const;            // 这些函数的实现不显示在这，但他们很ring一想想
    std::string birthDate()const;
    std::string address()const;
private:
    std::string theName;
    Date theBirthDate;
    Address theAddress;
};
```

有了这个RealPerson之后，写出Person::creat就一点也不稀奇了

```cpp
std::tr1::shared_ptr<Person> pp(Person::create(name, dateOfBirth, address)){
    return std::tr1::shared_ptr<Person>(new RealPerson(name, birthday, addr));
}
```

这个示范实现Interface class两个最常见机制之一：从Interface class继承接口规格，然后实现出接口覆盖的函数

---

# 请记住

- 支持编译依存性最小化的一般构想是：相依于声明式，不要相依于定义式。基于此构想的两个手段是Handle class和Interface class
- 程序库头文件应该以“完全且仅有声明式”的形式存在。
