# 需要类型转换时为模板定义非成员函数

---

rule24讨论了为什么只有non-member函数才能在所有实参身上实施隐式类型转换

若将operator*和Rational模板化

```cpp
template<typename T>
class Rational{
public:
    Rational(const T& numerator=0, const T& denominator=1);
    const T numerator()const;
    const T denominator()const;
    ...
};

template<typename T>
const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs){...}
```

我们希望支持混合式运算，希望以下代码能通过编译

```cpp
Rational<int> oneHalf(1, 2);
Rational<int> result=oneHalf*2;     // 错误！无法通过编译
```

这里编译器知道我们尝试调用`operator*`函数，试图相处一个函数被名为`operator*`的`template`具现化出来。他们知道因该可以具现化某个名为`operator*`的，接受两个`Rational<T>`参数的函数。但他们不知道T是什么，无法完成具现化

为了推嗷T的类型，他们查找`operator*`调用中的实参类型，分别是`Rational<int>`和`int`

1. 以oneHalf开始推导，很容易推导出T是int
2. 推导`int`，而第二参数被声明为`Rational<T>`，如何推算出T？

在`template`实参推导过程中，从不将隐式类型转换函数纳入考虑。绝不！因为在`template`实参推导过程中，在调用一个函数之前，首先得知道那个函数存在，为了得知这一信息，必须先将相关的函数模板推导出参数类型

`template` class 内的`friend`声明式可以指涉某个特定函数，意味着Rational<T>可以声明`operator*`作为他的一个`friend`函数。class template不依赖`template`实参推导，所以编译器总能在class Rational<T>具现化时得知T

因此可以令Rational<T> class 声明适当的operator*函数为`friend`函数，简化问题

```cpp
template<typename T>
class Rational{
public:
    ...
    friend const Rational operator*(const Rational& lhs, const Rational& rhs);
};

template<typename T>
const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs){...}
```

现在对operator*混合计算可以通过编译了，当对象oneHalf被声明为一个Rational<int>，class Rational<int>被具现化出来，作为过程的一部分，`friend`函数`operator*`被自动声明出来，后者作为一个函数而不是函数模板，所以编译器可以在调用时使用隐式转换函数（例如Rational的non- explicit构造函数）

！但是，这段代码虽然通过编译，但无法连接

---

首先看看在Rational中声明`operator*`的语法

在一个类模板中，`template`名称可以被用来作为"template和其参数"的粗略表达。例如Rational<T>内可以用Rational替换Rational<T>。当参数比较多的时候可以让代码比较干净。

本例中的`operator*`使用Rational而非Rational<T>。以下的声明同样有效

```cpp
template<typename T>
class Rational{
public:
    ...
    friend const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs);
};
```

再回头看我们遇到的问题。混合式代码通过编译，因为编译器知道调用接受Rational<int>的版本，但这个函数植被声明与Rational内，没有被定义出来

我们试图在class外部让`operator*`template提供定义，但是行不通。连接器无法找到这个定义式。

最简单可行的办法就是将`operator*`函数本体合并到声明式内

```cpp
template<typename T>
class Rational{
public:
    ...
    friend const Rational operator*(const Rational& lhs, const Rational& rhs){
        return Rational(lhs.numerator()*rhs.numerator(), lhs.denominator()*rhs.denominator());
    }
};
```

现在代码便能如期运作啦！万岁

---

这个技术的趣味之一：我们虽然使用`friend`，却和`friend`的传统用途（访问`class`的non-public成分）毫不相干

为了让类型转换可能发生在所有实参身上，我们需要一个non-member函数；为了让函数被自动具现化，我们需要讲它声明在类内部；在`class`内声明non-member函数的唯一办法就是令他成为友元

这里定义在`class`内的函数暗自成为`inline`，当函数体较大时，可以让他调用外部辅助函数，辅助函数完成真正的工作

辅助函数一般也是个`template`，所以定义了Rational的头文件代码，典型实现是

```cpp
template<typename T> class Rational; // 前置声明

// helper
template<typename T>
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs);

template<typename T>
class Rational{
public:
    ...
    friend const Rational operator*(const Rational& lhs, const Rational& rhs){
        return doMultiply(lhs, rhs);
    }
}

// 许多编译器强迫将所有template定义式放在头文件中
template<typename T>
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs){
    return Rational<T>(lhs.numerator()*rhs.numerator(), lhs.denominator()*rhs.denominator());
}
```

虽然helper不支持混合式乘法，但调用他的函数operator*支持

---

# 请记住

- 编写一个class template，而他所提供的与此`template`相关的函数支持所有参数的隐式类型转换时，将这饿函数定义为class template内部的`friend`函数