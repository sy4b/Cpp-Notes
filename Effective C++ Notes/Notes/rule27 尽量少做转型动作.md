# 尽量少做转型动作

C++规则的设计目标之一是，保证“类型错误”绝不可能发生

不幸的是，转型破坏了类型系统，可能导致任何种类的麻烦

---

首先回顾转型语法，通常有三种不同的转型动作

```cpp
// 旧时风格
(T) expression      // C风格
T(expression)       // 函数风格

// 新式风格
const_cast<T> (expression)
dynamic_cast<T> (expression)
reinterpret_cast<T>(expression)
static_cast<T>(expression)
```

新式风格各有不同的目的：

- `const_cast`通常用来移除对象的常量性，也是唯一有这个能力的C++-style转型操作符
- `dynamic_cast`主要用来执行“安全向下转型”，也就是用来决定某对象是否归属继承体系中的某个类型。她是唯一无法由旧时语法执行的动作，也是唯一可能耗费重大运行成本的转型动作
- `reinterpret_cast`意图执行低级转型，实际动作可能取决于编译器，也就代表着不可移植。例如将一个pointer to int转型为一个`int`，这一转型在低级代码之外很少见。
- `static_cast`用来强迫隐式转换，例如将non-const转换为`const`对象，或者`int`转换为`double`等等。

旧时转型仍然合法，但新式转型更受欢迎。首先：容易在代码中被识别出来；其次，各种转型的目标越窄化，编译器越容易诊断出错误的运用

---

【我】唯一使用旧时转型的时机是：调用一个`explicit`构造函数将一个对象传递给一个函数时

```cpp
class Widget{
public:
    explicit Widget(int size);
    ...
};
void doSomeWork(const Widget& w);
doSomeWork(Widget(15));
doSomeWork(static_cast<Widget>(15));
```

---

任何一个类型转换都会令编译器编译出运行期间执行的码

```cpp
int x, y;
double d=static_cast<double>(x)/y;
```

将`int`转型为`double`几乎肯定会产生一些代码，因为大部分计算机体系结构中，他们的底层表述不一样。

再考虑下面的例子

```cpp
class Base{...};
class Derived: public Base{...};
Derived d;
Base* pb=&d;                        // 隐含地将Derived*转换为Base*
```

这里我们建立一个base class指针指向一个derived class对象，但有时候上述的两个指针值并不相同。这种情况下会有个偏移量在运行期被施行在`Derived*`指针上，用来取得正确的`Base*`指针值

上个例子表明，单一对象（例如一个类型为Derived的对象）可能拥有一个以上的地址（例如以`Base*`指向它时的地址，和以`Derived*`指向他时的地址）。实际上C++一旦使用多重继承，这件事几乎一直发生着，即使在单一继承中也可能发生

---

另一件关于转型的有趣的事情是：我们很容易写出某些似是而非的代码。例如许多应用框架都要求derived class内的`virtual`函数代码第一个动作就先调用base class的对应函数

假设有个window base class和一个specialWindow derived class，都定义了onResize。下面的实现看起来对但实际上错

```cpp
class Window{
public:
    virtual void onResize(){...}
};
class SpecialWindow: public Window{
public:
    virtual void onResize(){
        static_cast<Window>(*this).onResize();
        ... // 专属动作
    }
    ...
};
```

这段代码并不如你所愿，他先转型为Window，然后在当前对象的base class部分的副本上调用`Window::onResize`，然后在副本身上执行专属动作，而不是在当前对象上执行。如果专属动作修改了对象内容，那么改动的其实是副本。

解决的办法是拿掉转型动作，单纯的调用base class版本的onResize函数就好了

```cpp
class SpecialWindow: public Window{
public:
    virtual void onResize(){
        Window::onResize();     // 在*this身上调用
        ...
    }
};
```

---

下面来探讨`dynamic_cast`，此前值得注意的是，它的许多实现版本执行速度相当的慢。

之所以需要`dynamic_cast`通常是因为你想在一个你认定为derived class对象身上执行derived class操作函数，但你的手上只有一个指向base的pointer或reference。有两个一般性做法可以避免这个问题

1. 使用容器并在其中存储直接指向derived class对象的指针（通常是智能指针，见rule13），这样便消除了通过base class接口处理对象的需要

假设先前的Window/SpecialWindow继承体系只有SpecialWindow支持闪烁效果，试着不要这么做

```cpp
class Window{...};
class SpecialWindow: public Window{
public:
    void blink();
    ...
};

using VPW = std::vector<std::tri::shared_ptr<Window>>;
VPW winPtrs;
...
for(VPW::iterator iter = winPtrs.begin(); iter!=winPtrs.end();++iter){  // 不希望使用dynamic_cast
    if(specialWindow* psw=dynamic_cast<SpecialWindow*>(iter->get())){
        psw->blink();
    }
}
```

而应该这样做

```cpp
using WPSW = std::vector<std::tr1::shared_ptr<SpecialWindow>>;
VPSW winPtrs;
...
for(VPSW::iterator iter=winPtrs.begin(); iter!=winPtrs.end();++iter){
    (*iter)->blink();
}
```

这种做法使你无法在同一个容器内存储指针“指向所有可能的派生类”。如果有更多种窗口，需要多个容器

2. 在base class内提供`virtual`函数做你想对每个派生类做的事。

```cpp
class Window{
public:
    virtual void blink(){}      // 缺省实现代码表示什么都不做，但可能是个馊主意，见rule34
    ...
};

class SpecialWindow: public Window{
public:
    virtual void blink(){...}
    ...
};
using VPW = std::vector<std::tri::shared_ptr<Window>>;
VPW winPtrs;
...
for(VPW::iterator iter = winPtrs.begin(); iter!=winPtrs.end();++iter){
    (*iter)->blink();
}
```

以上无论哪一种写法都非放之四海皆准，当他们有效时，应该欣然接受

绝对要避免的一件事是所谓的“连串dynamic_casts”，也就是类似这样的东西

```cpp
class Window{...};
...                         // derived classes 定义在这
using VPW = std::vector<std::tri::shared_ptr<Window>>;
VPW winPtrs;
...
for(VPW::iteraotr iter=winPtrs.begin(); iter!=winPtrs.end();++iter){
    if(SpecialWindow1 * psw1 = dynamic_cast<SpecialWindow1*>(iter->get())){...}
    else if(SpecialWindow1 * psw2 = dynamic_cast<SpecialWindow2*>(iter->get())){...}
    ...
}
```

这样产出的代码又大又慢，基础不牢。

---

优秀的C++代码很少使用转型，但若要完全摆脱他们又非常不切实际

---

# 请记住

- 如果可以，尽量避免转型，特别是在注重效率的代码中避免`dynamic_cast`
- 如果转型是必要的，试着将它隐藏在某个函数背后。客户可以随后调用该函数，而不用把转型放进自己的代码内
- 多使用新式转型代替旧时转型
