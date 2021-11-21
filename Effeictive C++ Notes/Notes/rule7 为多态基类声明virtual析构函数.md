# 为多态基类声明virtual析构函数

从一个例子入手

有多种做法可以记录时间，设计一个TimeKeeper base class和一些derived classes作为不同的计时方法

```cpp
class TimeKeeper{
public:
    TimeKeeper();
    ~TimeKeeper();
    ...
};
class AtomicClock: public TimeKeeper{...};  // 原子钟
class WaterClock: public TimeKeeper{...};   // 水钟
class WristWatch: public TimeKeeper{...};   // 腕表
```

许多客户只想在程序中使用时间，不想操心时间如何计算等细节，这时候可以设计factory函数，返回指针指向一个计时对象。

factory函数返回一个base class指针，指向新生成的derived class对象

```cpp
TimeKeeper* getTimeKeeper();    // 指向一个TimeKeeper派生类的动态分配对象
```

为了遵守fatcor函数（工厂函数）的规定，返回的对象必须位于heap，因此为了避免内存泄漏，对于每一个返回对象适当的`delete`很重要

```cpp
TimeKeeper* otk=getTimeKeeper();
...
delete ptk;
```

[Rule#13]()说到“依赖客户执行`delete`动作，基本上就带有某种错误倾向”，[Rule#18]()则谈到factory函数接口如何修改以便预防常见的客户错误。

这里需要解决的是上述代码的一个更根本的弱点：客户无法知道程序如何行动

问题出在`getTimeKeeper`返回的指针指向一个derived class对象，那个对象却经由一个base class指针被删除，而目前的base class有个non-virtual析构函数

C++指出，derived class对象经由一个base class指针被删除，而该base class指针带有non-virtual析构函数时，结果未定义——实际中通常是该对象的derived成分没有销毁

消除这个问题很简单：给base class一个`virtual`析构函数，伺候删除derived `class`对象就会如你所愿，销毁整个对象，包括所有derived class成分

```cpp
class TimeKeeper{
public:
    TimeKeeper();
    virtual ~TimeKeeper();
    ...
};
TimeKeeper* ptk=getTimeKeeper();
...
delete ptk;             // 现在，行为正确
```

任何`class`只要带有`virtual`函数，几乎也都应该有一个`virtual`析构函数

---

如果`class`不含`virtual`函数，通常表示它不想被用作一个base class，此时令析构函数为`virtual`往往是个馊主意

例如一个用来表示二维空间点坐标的`class`

```cpp
class Point{
public:
    Point(int xCoord, int yCoord);
    ~Point();
private:
    int x, yl
};
```

如果`int`占用32bits，那么Point对象可以放入一个64bit缓存其中，或者被当做一个64bit量传递给其他语言撰写的函数。

而如果Point的析构函数时`virtual`，则不可以。想要实现`virtual`函数，对象必须携带某些信息，在运行期决定哪一个`virtual`函数被调用，通常是由vptr(virtual table pointer)指针指出，vptr指向一个由函数指针构成的数组，成为vtbl(virtual table，虚函数表)，每一个带有`virtual`函数的`class`都有一个相应的vtbl

如果Point内含`virtual`函数，则其对象的体积会增加，用于存储虚函数表，因此不再是64bits

---

有时候令`class`带一个纯虚函数，可能颇为便利。纯虚函数导致抽象基类——不能被实体化的`class`。做法是将希望成为抽象的`class`声明一个纯虚函数

```cpp
class AWOV{
public:
    virtual ~AWOV()=0;
};
AWOV::~AWOV(){}         // 纯虚析构函数的定义，需要提供
```

析构函数的运作方式是：最深层派生的class的析构函数先被调用，如果不提供定义，编译器会抱怨哦

---
# 请记住

- 带多态性质的base class应该声明一个`virtual`析构函数；如果一个`class`带有任何`virtual`函数，他就应该拥有一个`virtual`析构函数
- 不作为基类或不具备多态性的类不该声明`virtual`析构函数
