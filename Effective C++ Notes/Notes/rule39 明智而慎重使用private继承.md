# 明智而慎重使用private继承

---

rule32论证C++将`public`继承视为is-a关系，在这个例子中我们有个继承体系，class Student以`public`形式继承class Person，于是编译器在必要时刻暗自将Student类型转换为Person

现在重复该例的一部分，用`private`继承替换`public`继承

```cpp
class Person{...};
class Student: private Person{...};

void eat(const Person& p);          // 任何人都会吃
void study(const Students& s);      // 学生才会学习

Person p;
Student s;

eat(p);                             // 没问题
eat(s);                             // 错误！害，难道学生不是人
```

显然`private`继承不意味着is-a关系，那她意味着什么？

---

首先搞清楚`private`继承的行为：第一，编译器不会自动将一个derived class对象转换为base class对象；第二，private base class继承而来的所有成员，在derived class中都会变成`priavte`属性，不论他们在base class中是`protected``public`还是`private`

`private`继承意味着implemented-in-terms-of。如果class D以`private`形式继承class B，意味着D是为了采用class B中已经备妥的某些性质，不是因为B对象和D对象存在任何观念上的关系

`private`继承纯粹是一种实现技术，只有实现部分被继承，接口部分都被略去

---

rule38刚刚指出，复合的意义也有implemented-in-terms-of，那么如何选取？

答案是尽量使用复合，必要时才采用`private`继承。什么是必要情况？主要是当`protected`成员或`virtual`函数牵扯进来的时候

假设我们的程序涉及Widget类型，我们要了解如何使用Widget，想知道成员函数多么频繁地被调用，以及一段时间过后调用比例如何变化。

我们决定修改class Widget，记录每个成员函数的被调用次数，运行期间周期性地审查这份信息。我们需要设定某种计时器，使我们知道收集统计数据的时候是否到了

```cpp
class Timer{
public:
    explicit Timer(int tickFrequency);
    virtual void onTick()const;         // 定时器滴答一次，该函数就自动调用一次
    ...
};
```

我们可以重新定义那个`virtual`函数，让后者取出Widget的当时状态。这里使用`public`继承不合适，我们必须以`private`形式继承Timer

```cpp
class Widget: private Timer{
private:
    virtual void onTick()const;         // 查看Widget的数据等等
    ...
};
```

这是个好设计，但没必要，我们如果决定用复合取而代之，只需要在Widget中声明一个嵌套式`class`，后者以`public`形式继承Timer并重新定义onTick

```cpp
class Widget{
private:
    class WidgetTimer: public Timer{
    public:
        virtual void onTick()const;
        ...
    };
    WidgetTimer timer;
    ...
};
```

这个设计只比使用`private`继承复杂一些些，同时设计了`public`继承和复合，导入一个新`class`。

有两个理由让你选择后者的做法

首先你或许会想设计Widget使它有derived class，同时阻止derived重新定义onTick。如果Widget继承自Timer，上面的想法就不可能实现，即使是`private`继承也不可能，因为onTick是`virtual`函数

但如果WidgetTimer是Widget的一个`private`成员并继承自Time，Widget的derived class将无法取用WidgetTimer，因此无法继承它或重新定义它的`virtual`函数

第二，你或许想将Widget的编译依存性降到最低。如果Widget继承自Timer，当Widget被编译时Timer的定义必须可见。但如果WidgetTimer移出Widget之外而Widget内涵一个指针指向WidgetTimer，那么Widget可以只带着一个简单的WidgetTimer声明

`private`继承主要用于“一个想要成为derived class者想访问=问一个想要称为base class者的`protected`成分，或为了重新定义一或多个`virtual`函数”，这时候两个`class`之间的概念关系是is-implemented-in-terms-of而非is-a

有一种激进情况设计空间最优化，可能会促使你选择`private`继承而不是继承加复合

这种情况只适用于你处理的classes不带任何数据。这样的`class`不存在non-static成员变量，没有`virtual`函数（因为这样的存在会给每个对象带来一vptr（虚函数表）），也没有virtual base classes（这样的base class也会带来体积上的额外开销，rule40）。

于是这种所谓的empty class对象不使用任何空间，因为没有任何隶属于对象的数据需要存储。然鹅技术上的理由，C++裁定凡是独立对象都必须有非零大小

```cpp
class Empty{};
class HoldsAnInt{
private:
    int x;
    Empty e;    // 应该不需要任何内存
};
```

你会发现上述代码sizeof(HoldsAndInt)>sizeof(int)，啊嘞，Empty成员变量居然要求内存。大多数编译器中sizeof(Empty)获得1，因为面对“大小为0的独立对象”，C++通常默默安插一个`char`，而齐位要求（rule50）可能不止增加一个`char`的大小

而这个约束不适用于derived class对象内的base class成分，因为他们并非独立对象。如果你继承Empty

```cpp
class HoldsAnInt: private Empty{
private:
    int x;
};
```

此时sizeof(HoldsAnInt)==sizeof(int)，这就是所谓的EBO(empty base optimization; 空白基类最优化)。

如果你的客户很在乎空间，那么值得注意EBO

EBO一般只在单一继承下才可能

现实中的“empty classes”并不是真的empty。他们可能包含`typedef`, `enum`, `static`成员变量，或者non-virtual函数

---

# 请记住

- `private`继承意味着is-implemented-in-terms-of(根据某物实现出)。通常他比复合的级别低
- `private`继承可以造成empty class最优化。这对致力于“对象尺寸最小化”的程序库开发者可能很重要