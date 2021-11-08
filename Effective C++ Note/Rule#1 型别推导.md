# 条款一：理解型别推导

对于一段伪代码

```cpp
template<typename T>
void f(ParamType param);

f(expr);
```

在编译期，编译器会通过`expr`推导两个型别：`T`的型别和`ParamTye`的型别，这两个型别往往不一样

`ParamType`通常包含一些修饰词，例如`const`或引用符号等

例如

```cpp
template<typename T>
void f(const T& param);

int x=0;
f(x);
```

此例中，`T`被推导为`int`，`ParamType`被推导为`const int&`

T的型别推导结果不仅依赖`expr`的型别，也依赖于`ParamType`的形式，分三种情况

- `ParamType`具有指针或引用型别，但不是万能引用（区别于左值引用和右值引用）
- `ParamType`是一个万能引用
- `ParamType`既非指针也非引用

### 情况一：ParamType是个指针或引用，但不是万能引用

最简单的情形，此时型别推导会这样运作：

1. 若`expr`具有引用型别，先将引用部分忽略
2. 尔后，对`expr`的型别和`ParamType`的型别执行模式匹配，决定`T`的型别

```cpp
template<typename T>
void f(T& param);

int x=27; // int
const int cx=x; // const int
const int& rx=x; // const int&

f(x); // T的型别是int，param的型别是int&
f(cx); // T的型别是const int，param的型别是const int&
f(rx); // T的型别是const int，param的型别是const int&
```

```cpp
template<typename T>
void f(const T& param);

int x=27; // int
const int cx=x; // const int
const int& rx=x; // const int&

f(x); // T的型别是int，param的型别是const int&
f(cx); // T的型别是int，param的型别是const int&
f(rx); // T的型别是int，param的型别是const int&
```

```cpp
template<typename T>
void f(T* param);

int x=27; // int
const int* rx=x; // const int&

f(x); // T的型别是int，param的型别是int*
f(rx); // T的型别是const int，param的型别是const int*
```

### 情况二：ParamType是一个万能引用

- 如果`expr`是左值，T和`ParamType`都会被推导为左值引用
- 如果`expr`是个右值，则应用情况一的规则

```cpp
template<typename T>
void f(T&& param); // param是一个万能引用

int x=27; // int
const int cx=x; // const int
const int& rx=x; // const int&

f(x); // x是一个左值，T的型别是int&，param的型别是int&
f(cx); // rx是一个左值，T和param的型别都是const int&
f(rx); // rx是一个左值，T和param的型别都是const int&
f(27); // 27是一个右值，T的型别是int，param的型别是int&&
```

### 情况三：ParamType既非指针也非引用

当`ParamType`既不是指针也不是引用，那么则是按值传递

```cpp
template<typename T>
void f(T param);
```

这意味着无论传入的是什么，param都是它的一个副本，是一个全新对象

- 若`expr`具有引用型别，忽略其引用部分
- 忽略引用性后，若`expr`是一个`const`对象，也忽略。如果是个`volatile`对象，同样忽略

```cpp
int x=27;
const int cx=x;
const int& rx=x;

f(x); // T和Param都是int
f(cx); // T和Param都是int
f(rx);  // T和Param都是int
```

⚠️

- 即使`cx`和`rx`代表`const`值，`param`仍然不具有`const`型别，这是很合理的。`param`是完全独立于`cx`和`rx`存在的对象，是一个副本
- `const`和`volatile`仅会在按值传递形参时被忽略，若形参是`const`的引用或指针，则会被保留
- 但是考虑这种情况：`expr`是一个指涉`const`对象的`const`指针，且`expr`按值传递给`param`
```cpp
template<typename T>
void f(T param);

const char* const ptr="Fun with pointers";

f(ptr); // param是const char*，即一个指向const对象的指针，ptr本身是可以修改的，const丢失
```
---

### 数组实参

以上基本讨论完模板型别推导的主流情况，但还有边缘情况值得了解

数组型别有别于指针型别，尽管有时候他们看起来可以互换。形成这种假象的主要原因是在很多语境下，数组会退化成指涉首元素的指针

例如这段代码可以通过编译，就是因为退化的机制在发挥作用

```cpp
const char name[]="Syb is not Sb";  // name的型别是const char [14]
const char* ptrToName=name; // 数组退化为指针
```

而将一个数组传递给持有按值形参的模板时，又会怎么样呢？

```cpp
template<typename T>
void f(T param);

f(name);
```

首先观察到，没有任何函数形参具有数组型别。`void myFunc(int param[]);`虽然合法，但会被等价的声明为`void myFunc(int* param);`。这种数组和指针形参的等价性，是作为C++基础的C根源遗迹

由于数组形参声明会按照它们好像是指针形参那样去处理，因此在`f`的调用中，T会被推导为`const char*`

```cpp
f(name);  // name是个数组，但T却被推导为const char*
```

难点来了。尽管函数无法声明真正的数组型别的形参，他们却能够将形参声明为数组的引用。所以如果我们修改模板`f`，指定按引用方式传递实参

```cpp
template<typename T>
void f(T& param); // 按引用传递参数

f(name);  // 传递一个数组
```

在这种情况下，`T`的型别会被推导为实际的数组型别！这个型别中会包含数组尺寸。

本例中，`T`被推导为`const char [14]`，`param`被推导为`const char& [14]`

### 函数实参

数组并非C++中唯一可以退化为指针的东西。函数型别同样也会退化为函数指针，并且我们针对数组型别推导的一切讨论都适用于函数及其向函数指针的退化。

```cpp
void someFunc(int, double);

template<typename T>
void f1(T param);
void f2(T& param);

f1(someFunc); // param被推导为void(*)(int, double)

f2(someFunc); // param被推导为void(&)(int, double)
```
---
# 要点速记

- 在模板型别推导过程中，具有引用的实参会被当成非引用型别处理。引用行被忽略
- 对万能引用形参进行推导时，左值实参会特殊处理
- 对按值传递的形参推导时，若实参中含有`const`或`volatitle`饰辞，也会被忽略
- 在模板型别推导过程中，数组或函数型别的实参会退化为对应的指针，除非被用来初始化引用
