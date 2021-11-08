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
