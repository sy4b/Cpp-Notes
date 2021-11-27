# 尽可能使用const

`const`允许指定一个语义约束，一个不该被改动的对象，而编译器会强制实施这项约束。

只要某个值保持不变，就应该显式指出，可以获得编译器的襄助，确保这条规则不被违反

---

关键字`const`用法多样。可以在`class`外部修饰global或namespace作用域中的常量，或修饰文件、函数或区块作用域中被声明为`static`的对象，也可以修饰`class`内部的`static`和`non-static`成员变量。对于指针，可以指出指针本身，指针所指物或两者是否为`const`

```cpp
char greeting[]="Hello";

char* p=greeting;                   // non-const pointer, non-const data

const char* p=greeting;             // non-const pointer, const data

char* const p=greeting;             // const pointer, non-const data

const char* const p=greeting;       // const pointer, const data
```

`const`出现在`*`左侧，说明被指物是常量；`const`出现在右侧说明指针本身是常量；出现在两边，表示都是常量

`const`写在类型之前之后没有区别

```cpp
void f1(const Widget* pw);      // 被指物是常量
void f2(Widget const* pw);      // 同上
```

---

STL迭代器的作用就像`T*`指针，声明迭代器为`const`就像声明指针为`const`一样（即`T* const`），表明这个迭代器不得指向不同的值，但所指的东西的值是可以改动的。

如果希望迭代器指向的东西不可改动（即一个`const T*`指针），需要使用`const_iterator`

```cpp
std::vector<int> vec;

const std::vector<int>::iterator iter=vec.begin();      // iter的作用类似 T* const
*iter=10;                                               // 没问题
++iter;                                                 // 错误

std::vector<int>::const_iterator cIter=vec.begin();     // cIter作用类似 const T*
*iter=10;                                               // 错误
++iter;                                                 // 正确
```

---

`const`最具威力的用法是面对函数声明时的应用

令函数返回一个常量，往往可以降低因客户错误造成的意外，又不至于放弃安全性和高效性。比如考虑有理数的`operator*`声明式

```cpp
class Rational{...};
const Rational operator* (const Rational& lhs, const Rational& rhs);

Rational a, b, c;
...
(a*b)=c;                // 可能想a*b==c 打错了
```

---

将const实施于成员函数，可以确认该成员函数可以作用于`const`对象。

这类成员函数之所以重要，基于两个理由：

1. 指明哪个函数可以改动对象内容，哪个不行，使`class`接口容易被理解
2. 使操作`const`对象成为可能，对于高效编码很关键

两个成员函数如果只是常量性不同，可以被重载，这是C++一个重要的特性

```cpp
class TextBlock{
public:
    const char& operator[](std::size_t position)const{return text[position];}
    char& operator[](std::size_t position){
        return text[position];
    }
private:
    std::string text;
};

// 调用
TextBlock tb("Hello");
std::cout<<tb[0];

const TextBlock ctb("World");       // 调用non-const版本
std::cout<<ctb[0];                  // 调用const版本
```

---

# 请记住

- 将某些东西声明为`const`可帮助编译器侦测出错误语法。`const`可被施加于任何作用域内的对象、函数参数、函数返回类型、成员函数本体
- 当`const`和`non-const`成员函数有实质等价的实现时，可以令`non-const`版本调用`const`版本避免代码重复
