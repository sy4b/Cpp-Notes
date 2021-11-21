# 了解C++默默编写并调用哪些函数

当C++处理过一个空类，这个空类不再是空类。如果自己不声明，编译器就会为它声明编译器版本的`copy`构造函数、一个`copy assignment`操作符和一个析构函数。如果没有声明任何构造函数，编译器也会声明一个`default`构造函数。

这些函数都是`public`且`inline`的

因此如果你写下

```cpp
class Empty{};
```

就好像写下这样的代码

```cpp
class Empty{
public:
    Empty(){...}                                // default构造函数
    Empty(const Empty& rhs){...}                // copy构造函数
    ~Empty(){...}                               // 析构函数

    Empty& operator=(const Empty& rhs){...}     // copy assignment运算符
};
```

只有当这些函数被需要时，它们才会被编译器创建出来

```cpp
Empty e1;           // default构造函数
Empty e1(e2);       // copy构造函数
e2=e1;              // copy assignment运算符
```

`default`构造函数和析构函数主要用于调用成员变量的构造函数和析构函数

编译器产出的析构函数不是一个虚函数，除非这个`class`的`base class`自身声明有`virtual`析构函数

至于`copy`和`copy assignment`操作符，编译器创建的版本只是单纯将源对象的每一个`non-static`成员变量拷贝到目标对象

```cpp
template<typename T>
class NamedObject{
public:
    NamedObject(const char* name, const T& value);
    NamedObjecr(const std::string& name, const T& value);
    ...
private:
    std::string nameValue;
    T objectValue;
}
```

由于声明了其中一个构造函数，编译器不再创建`default`构造函数

编译器会创建`copy`构造函数和`copy assignment`操作符（如果被调用的话）

```cpp
NamedObject<int> no1("Smallesr Prime Number", 2);
NamedObject<int> no2(no1);          // 调用copy构造函数
```

编译器的`copy`构造函数必须以`no1.nameValue`和`no1.objectValue`作为初值设定`no2`对应的值

对于`nameValue`，类型为`string`，所以`no2.nameValue`初始化方式是调用`string`的`copy`构造函数

---

# 请记住

编译器可以自动为`class`创建`default`构造函数，`copy`构造函数、`copy assignment`操作符以及析构函数
