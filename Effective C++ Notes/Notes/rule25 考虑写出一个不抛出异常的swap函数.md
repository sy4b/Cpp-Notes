# 考虑写出一个不抛出异常的swap函数

`swap`是个有趣的函数。原本他只是STL的一部分，后来成为异常安全性编程的脊柱，以及用来处理自我赋值可能性的一个常见机制

---

所谓swap就是将两个对象的值彼此赋予对象。缺省情况下swap动作可由标准程序库提供的`swap`算法完成。其典型实现完全如你预期

```cpp
namespace std{
    template<typename T>
    void swap(T& a, T& b){
        T temp(a);
        a=b;
        b=temp;
    }
}
```

只要类型`T`支持copying操作，缺省的swap实现代码就会自动置换类型`T`的对象

这样缺省的`swap`实现版本十分平淡。他涉及三个对象的复制：`a`复制到`temp`，`b`复制到`a`以及`temp`复制到`b`。但对某些类型而言，这些复制动作没有必要

其中最主要的就是“指针指向一个对象，内含真正数据”那种类型。这种设计的常见表现形式是所谓的"pimpl手法"

```cpp
class WidgetImpl{
public:
    ...
private:
    // 有许多数据，意味着复制时间很长
    int a, b, c;
    std::vector<double> v;
    ...
};

class Widget{                       // pimpl手法
public:
    Widget(const Widget& rhs);
    Widget& operator=(const Widget& rhs){
        ...
        *pImpl=*(rhs.pImpl);
    }
    ...
private:
    WidgetImpl* pImpl;              // 指针，所指对象内含Widget数据
}
```

一旦需要置换两个`Widget`对象值，唯一需要做的就是置换其pImpl指针，但缺省的`swap`算法不知道这一点。它不止复制三个`Widget`还复制三个`WidgetImpl`对象，非常缺乏效率

我们希望告诉`std::swap`，当`Widget`被置换时真正应该做的事置换内部的`pImpl`指针。实践这个思路的一个做法是：将`std::swap`针对`Widget`特例化

以下是基本思路，但无法通过编译

```cpp
namespace std{
    template<>
    void swap<Widget>(Widget& a, Widget& b){
        swap(a.pImpl, b.pImpl);         // 置换指针即可，但private，所以无法通过
    }
}
```

我们可以将这个特例化版本声明为`friend`，但和以往的规矩不太一样，我们令`Widget`声明一个`swap`的`public`成员函数做真正的特例化工作，然后将`std::swap`特例化，让他调用该成员函数

```cpp
class Widget{
public:
    ...
    void swap(Widget& other){
        using std::swap;            // 这个声明很有必要，稍后解释
        swap(pImpl, other.pImpl);
    }
    ...
};

namespace std{
    template<>
    void swap<Widget>(Widget& a, Widget& b{
        a.swap(b);
    })
}
```

这种做法不但能够通过编译，而且与STL容器具有一致性

---

假设`Widget`和`WidgetImpl`都是模板类，也许我们可以尝试将`WidgetImpl`内的数据类型参数化

```cpp
template<typename T>
class WidgetImpl{...};

template<typename T>
class Widget{...};
```

在`Widget`内放个`swap`成员函数就像以前那样简单，但特化`std::swap`确有困难。我们试图写成这样

```cpp
namespace std{
    template<typename T>
    void swap<Widget<T>> (Widget<T>& a, Widget<T>& b){
        a.swap(b);
    }
}
```

看起来合情合理，但不合法。这个例子中我们试图偏特化一个模板函数`std::swap`，但C++只允许对模板类偏特化。

当打算偏特化一个function template时，习惯做法是简单的为他添加一个重载版本

```cpp
namespace std{
    template<typename T>
    void swap(Widget<T>& a, Widget<T>& b){  // 不合法
        a.swap(b);
    }
}
```

一般来说重载function template没有问题，但`std`是个特殊的命名空间，管理规则也比较特殊：客户可以全特化`std`内的`template`，但不可以添加新的`template`到里面。

那该如何是好？答案是：我们还是声明一个non-member swap让他调用member swap，但不再将那个non-member swap声明为`std::swap`的特化版或重载版

```cpp
namespace WidgetStuff{
    ...
    template<typename T>
    class Widget{...};      // 内涵swap成员函数
    ...
    template<typename T>
    void swap(Widget<T>& a, Widget<T>& b){
        a.swap(b;)
    }
}
```

此时，任何地点的任何代码如果打算置换两个`Widget`对象，因而调用`swap`，都会查找到这个专属版本

这个做法对`class`和class template都行得通，所以我们似乎应该在任何时候都使用它。但不幸的是有一个理由使你应该为`class`特化`std::swap`

---

换位思考，从客户观点看看事情。

假设正在写一个function template，需要置换两个对象值

```cpp
template<typename T>
void doSomething(T& obj1, T& obj2){
    ...
    swap(obj1, obj2);
    ...
}
```

调用哪个`swap`？我们希望调用T专属版本，并且不存在该版本时，调用`std`内的一般化版本

```cpp
template<typename T>
void doSomething(T& obj1, T& obj2){
    using std::swap;        // 令std::swap在此函数内可用
    ...
    swap(obj1, obj2);       // 为T型对象调用最佳swap版本
    ...
}
```

编译器看到对`swap`的调用后，会查找适当的版本。C++的名称查找法则确保将找到global作用域或T所在命名空间内的任何T专属的`swap`。如果找不到，就用`std::swap`，这多亏`using`声明式。编译器还是偏爱特化版本。

但有些sb这样调用：

```cpp
std::swap(obj1, obj2);  // error
```

这样相当于是强迫编译器只认`std`内的`swap`。因此不可能调用T的专属版本。这也是“你的`class`应该对`std::swap`进行全特化”的重要原因：使得这样的代码不至于出错

---

做个小结

首先：如果`swap`的缺省实现对你的`class`或class template提供可接受的效率，那你不需要做其他事

其次，如果缺省版的效率不足，试着：

1. 提供一个public swap成员函数，让他高效地进行置换。这个函数绝不该抛出异常
2. 在你的`class`或`template`所在的命名空间内提供一个non-member swap，令他调用上述`swap`成员函数
3. 如果是编写一个`class`（不是template），为你的`class`特例化`std::swap`，令他调用你的`swap`成员函数

最后，如果你调用`swap`，请确保包含一个`using`声明式，然后不加任何`namespace`修饰符，赤裸裸地调用`swap`
