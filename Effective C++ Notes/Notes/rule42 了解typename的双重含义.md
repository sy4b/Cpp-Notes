# 了解typename的双重含义

---

对于以下代码，`class`和`typename`没有任何不同

```cpp
template<class T> class Widget;
template<typename T> class Widget;
```

声明`template`类型参数时，`class`和`typename`的意义完全相同

---

而C++并不总是将这两者视为等价，有时候一定得使用`typename`

首先看看可以在`template`中指涉的两种名称

假设有一个模板函数，接受一个STL容器作为参数，容器内对象可以被赋值为`int`，这个函数打印第二个元素值

```cpp
// 以下代码不能通过编译
template<typename C>
void print2nd(const C& container){
    if(container.size()>=2){
        C::const_iterator iter(container.begin());
        ++iter;
        int value=*iter;
        std::cout<<value;
    }
}
```

以上代码中iter的类型C::const_iterator取决于`template`的参数C

`template`内出现的名称如果依赖于某个`template`参数，我们称之为从属名称，如果从属名称在`class`内呈现嵌套状，称之为嵌套从属名称，C::const_iterator就是一个。他还是个嵌套从属类型名称，这个名称指涉一个类型

另一个local变量value类型为`int`，不依赖于任何`template`参数，称为非从属名称

嵌套从属名称可能导致解析困难，例如

```cpp
template<typename C>
void print2nd(const C& container){
    C::const_iterator* x;
    ...
}
```

看起来我们声明了一个变量x，是一个指针，指向一个C::const_iterator，但如果C::const_iterator不似一个类型呢？比如C有个成员变量被命名为const_iterator或者x是个global变量呢？这样的话上述代码就是一个相乘动作

在我们知道C是什么之前，没有任何办法得知C::const_iterator是否为一个类型。C++解析器在缺省情况下假设嵌套从属名称不是类型，除非你告诉他是（有个例外，下面说）

要解决这个歧义问题，需要在他面前放一个关键字`typename`

```cpp
template<typename C>
void print2nd(const C& container){
    if(container.size()>=2){
        typename C::const_iterator iter(container.begin());
        ...
    }
}
```

一般性规则很简单：任何时候想要在`template`中指涉一个嵌套从属类型名称，必须在紧临他的前一个位置加关键字`typename`。其他名称不该有他存在

```cpp
template<typename C>
void f(const C& container,              // 不允许使用typename
            typename C::iterator iter); // 必须使用typename
```

"`typename`必须作为嵌套从属类型名称的前缀词"这一规则的例外是，`typename`不可以出现在base classes list内的嵌套从属名称之前，也不可以在成员初始值列表中作为base class修饰符

```cpp
template<typename T>
class Derived: public Base<T>::Nested{  // 不允许
public:
    explicit Derived(int x): Base<T>::Nested(x){    // 不允许
        typename Base<T>::Nested temp;  // 嵌套从属名称
        ...
    }
    ...
};
```

这种不一致性令人恼火，但有一些经验后还勉强能够接受

来看一个例子，是在真实程序中看到的代表性例子。我们撰写一个函数模板，接受一个迭代器，我们打算为该迭代器指涉的对象做一份local附件temp

```cpp
template<typename IterT>
void workWithIterator(IterT iter){
    typename std::iterator_traits<IterT>::value_type temp(*iter);
    ...
}
```

std::iterator_traits<IterT>::value_type得到“IterT对象指向的类型”，这是一个嵌套从属类型，必须加前缀`typename`

可以使用using声明或`typedef`给这个类型取别名，少打几个字

---

# 请记住

- 声明`template`参数时，前缀关键字`class`和`typename`可以互换
- 必须使用关键字`typename`标识嵌套从属类型名称；但不得在base class list或member initialization list内以他作为base class修饰符