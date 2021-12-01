# 了解隐式接口和编译期多态

面向对象编程世界总是以显式接口和运行期多态解决问题

---

例如以下的`class`和function

```cpp
class Widget{
public:
    Widget();
    virtual ~Widget();
    virtual std::size_t size()const;
    virtual void normalize();
    void swap(Widget& other);
    ...
};

void doProcessing(Widget& w){
    if(w.size()>10 && w!=someNastyWidget){
        Widget temp(w);
        temp.normalize();
        temp.swap(w);
    }
}
```

doProcewssing中的w有这样的特点：

- w的类型被声明为Widget，所以w必须支持Widget接口，我们可以在源码中找到这个接口，所以成为一个显式接口
- Widget的某些成员函数事`virtual`，w对于那些函数的调用将表现出运行期多态，也就是在运行期根据w的动态类型决定调用哪一个函数

templates及泛型编程的世界与面向对象有根本上的不同。在此世界中显式接口和运行期多态仍然存在，但重要性降低。反倒是隐式接口和编译器多态更重要

我们将doProcessing从函数变成函数模板

```cpp
template<typename T>
void doProcessing(T& w){
    if(w.size()>10 && w!=someNastyWidget){
        Widget temp(w);
        temp.normalize();
        temp.swap(w);
    }
}
```

- w必须支持哪一种接口，由`template`中执行于w身上的操作决定。本例中w的类型T必须支持size normalize和swap copy构造函数 !=比较。这组表达式是T必须支持的一组隐式接口
- 凡涉及w的任何函数调用，例如operator> 和operator!=，有可能造成模板具现化。这样的具现化行为发生在编译期，以不同的模板参数具现化函数模板会导致调用不同的函数，这就是所谓编译期多态

---

通常显式接口由函数的签名式（包括函数名称、参数、返回类型）构成

例如Widget class，其`public`接口🈶️各个函数及其参数类型、返回类型、常量性构成...此外也可以包括`typedef`

```cpp
class Widget{
public:
    Widget();
    virtual ~Widget();
    virtual std::size_t size()const;
    virtual void normalize();
    void swap(Widget& other);
    ...
};
```

隐式接口则由**有效表达式**组成

```cpp
template<typename T>
void doProcessing(T& w){
    if(w.size()>10 && w!=someNastyWidget){
        ...
    }
}
```

T（w的类型）的隐式接口看起来有这样的约束：

- 必须提供一个size成员函数，返回一个整数值
- 必须支持operator!=函数，用来比较两个T对象（这里假设omeNastyWidget类型为T）

由于操作符重载带来的可能性，上述两个约束其实不一定需要得到满足

例如size成员函数，可以从base class继承得到；operator>也不一定两个操作数都是T类型，可能是可以转化为T类型的X类型，或者X类型重载operator>与T类型对象比较；operator!=函数同理

但其实不用想得这么复杂。隐式接口仅仅由一组有效表达式构成，表达式自身看起来很复杂，但他们的约束条件一般而言相当直接明确

例如`if(w.size()>10 && w!=someNastyWidget)`要求表达式能与`bool`兼容即可，这是T的隐式接口的一部分

其他隐式接口例如copy构造函数等要求对T型对象有效

---

# 请记住

- classes和templates都支持接口和多态
- 对classes而言，接口是显示的，以函数签名为中心；多态则是通过`virtual`函数发生于运行期
- 对templates参数而言，接口是隐式的，基于有效表达式；多态则是通过`template`具现化和函数重载解析发生于编译期