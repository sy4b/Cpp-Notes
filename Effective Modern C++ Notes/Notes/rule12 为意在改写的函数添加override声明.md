# 为意在改写的函数添加override声明

OOP围绕着类、继承和虚函数的基础演化而来，最基本的理念就是：在派生类中虚函数实现会改写基类中对应虚函数的实现。虚函数的改写使得通过基类接口调用派生类函数成为可能

---

```cpp
class Base{
public:
    virtual void doWork();
    ...
};

class Derived: public Base{
public:
    virtual void doWork();
    ...
};

// 创建基类指针，指涉派生类对象
std::unique_str<Base> upb=std::make_unique<Derived>();
...

upb->doWork();      // 结果是派生类doWork被调用
```

如果想要这个改写动作真实发生，需要满足以下条件

- 基类中的函数必须是虚函数
- 基类和派生类中的函数名字完全相同（析构函数除外
- 基类和派生类中函数形参型别必须完全相同
- 基类和派生类中函数常量性完全相同
- 基类和派生类中函数返回值和异常规格必须兼容
- （C++11）基类和派生类中函数引用饰辞完全相同

引用饰辞是C++11的特性，为了实现限制成员函数仅用于左值或右值；带有引用饰辞的成员函数不必是虚函数

```cpp
class Widget{
public: 
    void doWork()&;         // 仅在*this是左值时调用
    void doWOrk()&&;        // 仅在*this是右值时调用
};

Widget makeWidget();    // 工厂函数 返回右值
Widget w;               // 普通对象，左值

w.doWork();             // Widget::doWork &
makeWidget().doWork();  // Widget::doWork &&
```

包含改写方面的错误的代码一般都是合法的，但表达的意思背离初衷，无法依赖编译器来告知

```cpp
class Base{
public:
    virtual void mf1()const;
    virtual void mf2(int x);
    virtual void mf3()&;
    void mf4()const;
};

class Derived: public Base{
public:
    virtual void mf1();     // 缺少const
    virtual void mf2(unsigned int x);   // 形参型别不一致
    virtual void mf3()&&;   // 引用饰辞不一致
    void mf4()const;        // 基类未声明虚函数
}
```

----

对于派生类改写，保证正确性很重要，出错又很容易。C++提供一种显式指出函数需要改写的方法：添加`override`声明

```cpp
// 无法通过编译
class Derived: public Base{
public:
    virtual void mf1() override;
    virtual void mf2(unsigned int x)override;   
    virtual void mf3()&& override;
    void mf4()const override;
}
```

C++11的两个语境关键字`override` `final`的特点是，仅在特定语境下保留关键字。对于`override`，它仅在成员函数声明的末尾才有保留意义

这意味着如果有其他地方代码使用override名称，不必改名

---

关于引用饰辞

如果想写一个函数仅接受传入左值实参，我们会声明一个非`const`左值引用实参

```cpp
void doSth(Widget& w);  // 仅接受左值实参
void doSth(Widget&& w); // 仅接受右值实参
```

引用饰辞则是针对发起函数调用的对象，即`*this`；这与`const`位于成员函数末尾一样，表示发起成员函数调用的对象必须是`const`

带引用饰辞的成员函数并不常见，但也有可能出现

例如Widget类中包含一个`std::vector`类型的成员，我们提供一个访问器函数让客户能够直接访问这个数据成员

```cpp
class Widget{
public:
    using DataType = std::vector<double>;
    DataType& data(){return values;}
private:
    DataType values;
};
```

客户如下使用

```cpp
Widget w;
auto vals1 = w.data();  // 将w.values复制到vals1
```

这里Widget::data返回一个左值引用，我们使用一个左值初始化vals1

现在假设有个工厂函数`Widget makeWidget();`，而我们向使用data

```cpp
auto vals2 = makeWidegt().data();
```

这里更好的做法应该是移动而不是复制，但由于data返回左值引用，所以这里进行了无意义的对象复制

需要找到方法，指定让data在右值Widget上调用时，结果成为一个右值。运用引用饰辞进行重载，就可以实现

```cpp
class Widget{
public:
    using DataType = std::vector<double>;
    DataType& data()&{return values;}
    DataType data()&&{return std::move(values);}
private:
    DataType values;
};
```

---

# 要点速记

- 为意在改写的函数添加`override`声明
- 引用饰辞使得对于左值和右值对象的处理分开