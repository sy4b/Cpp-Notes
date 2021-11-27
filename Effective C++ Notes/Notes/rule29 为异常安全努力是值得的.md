# 为异常安全努力是值得的

假设有个`class`用来表现夹带图案背景的GUI菜单。这个`class`希望用于多线程环境，所以他有个互斥器作为并发控制之用：

```cpp
class PrettyMenu{
public:
    ...
    void changeBackground(std::istream& imgSrc);    // 改变背景图像
    ...
private:
    Mutex mutex;        // 互斥器
    Image* bgImage;     // 目前的背景图像
    int imageChanges;   // 背景图像被改变的次数
};
```

下面是`PrettyMenu::changeBackground`的一个可能实现

```cpp
void PrettyMenu::changeBackground(std::istream& imgSrc){
    lock(&nutex);       // 取得互斥器
    delete bgImage;     // 摆脱旧的背景图像
    ++imageChanges;
    bgImage=new Image(imgSrc);  // 安装新的背景图像
    unlock(&numtex);    // 释放互斥器
}
```

从异常安全性的观点来看，这个函数很糟糕。“异常安全”有两个条件，而这个函数没有满足其中任意一个

当异常被抛出时，带有异常安全性的函数会

- 不泄漏任何资源。一旦`new Image(imgSrc)`导致异常，对`unlock`的调用就绝对不会执行，互斥器就永远被把持住了
- 不允许数据败坏。如果`new Image(imgSrc)`抛出异常，`bgImage`就指向一个被删除的对象，`imageChanges`也已经被累加，但实际上没有新的图像被成功安装起来

解决资源泄漏的问题很容易，rule13讨论以对象管理资源，rule14导入了`Lock` class作为一种确保互斥器被及时释放的方法

```cpp
void PrettyMenu::changeBackground(std::istream& imgSrc){
    Lock m1(&nutex);
    delete bgImage;
    ++imageChanges;
    bgImage=new Image(imgSrc);
}
```

---

现在就可以专注解决数据的败坏。首先必须先面对一些属于

异常安全函数提供以下三个保证之一：

1. 基本承诺

如果异常被抛出，程序内的任何事物仍然保持在有效状态下。没有任何对象或数据结构因此而败坏

2. 强烈保证

如果异常被抛出，程序状态不改变。调用这样的函数需要有这样的认知：如果函数成功，就是完全成功；如果失败，程序会恢复到调用函数之前的状态

3. 不抛掷保证

承诺绝不抛出异常，因为他们总是能够完成他们原先承诺的功能。作用域内置类型（例如int，指针）身上的所有操作都提供nothrow保证。这是异常安全码中一个必不可少的关键基础材料

异常安全码必须提供以上三种保证之一。如果不这样做，就不具备异常安全性。因此我们的抉择时为所写的每一个函数提供哪一种保证？

一般而言会想要实施最强烈的保证。但是很难会有一个函数满足这种条件。任何使用动态内存的函数如果无法找到满足的内存满足需求，通常会抛出一个`bad_allco`异常。是的，可能的话请提供nothrow保证，但对大部分函数而言，抉择往往落在基本保证和强烈保证之间

对于`changeBackground`函数而言，提供强烈保证不困难。首先改变`bgImage`的类型，换位智能指针；其次重新排列语句顺序，更换图像后再累加次序

```cpp
class PrettyMenu{
    ...
    std::tr1::shared_ptr<Image> bgImage;
    ...
}
void PrettyMenu::changeBackground(std::istream& imgSrc){
    Lock m1(&mutex);
    bgImage.reset(new Image(imgSrc));
    ++imageChanges;
}
```

注意这里不需要手动`delete`旧图像，这个动作已经由智能指针内部处理掉了。只有进入`reset`函数才会被调用。

美中不足的是`imgSrc`，如果构造函数抛出异常，有可能输入流的读取极好已被移走，这样程序其余部分状态会改变。

然而我们先把它放在一旁，假装`changeBackground`的确提供了强烈保证

---

有一个一般化的设计策略很典型地导致强烈保证，值得熟悉他，这个策略被称为copy and swap

原则很简单：为你打算修改的对象原件做出一份副本，然后在副本身上做出一切必要修改。若有任何修改动作抛出异常，原对象仍然保持未改变状态。所有改变都成功后，再将修改过的副本和愿对象在一个不抛出异常的操作中置换

实际上，通常是将所有“隶属对象的数据”从愿对象放进另一个对象内，然后赋予原对象一个指针，指向所谓的实现对象（implementation object,即副本），这种手法称为pimpl idiom，rule31会详细描述

```cpp
struct PMImpl{
    std::tr1::shared_ptr<Image> bgImage;
    int imageChanges;
};

class PrettyMenu{
    ...
private:
    Mutex mutex;
    std::tr1::shared_ptr<PMImpl> pImpl;
};

void PrettyMenu::changebackground(std::istream& imgSrc){
    using std::swap;
    Lock m1(&nutex);
    std::tr1::shared_ptr<PMImpl> pNew(new PMImpl(*pImpl));  // 获得副本
    pNew->bgImage.reset(new Image(imgSrc)); // 修改副本
    ++pNew->imageChanges;
    swap(pImpl, pNew);      // 置换数据，释放nutex
}
```

这里选择让`PMImpl`成为一个`struct`而不是`class`，是因为`PrettyMenu`的数据封装性已经由于`PImpl`是`private`获得了保证

copy and swap策略是对对象做出“全有或全无”改变的一个很好的办法，但一般而言他不保证整个函数有强烈的异常安全性。为了了解原因，我们考虑一个`someFunc`，它使用copy and swap策略，但函数中还包括对另外两个函数`f1`和`f2`的调用

```cpp
void someFunc(){
    ...             // 对local状态做一份副本
    f1();
    f2();
    ...             // 置换
}
```

1. 如果`f1`或`f2`的异常安全性低，那么显然整个函数都不是强烈异常安全

2. 如果`f1`和`f2`都是强烈异常安全，情况也不会好转。假如`f1`正常完成，此时程序状态改变，而`f2`却抛出异常，这时候就GG啦

另一个主题关乎效率。copy and swap关键在修改对象数据的副本，然后在一个不抛出异常的函数中将修改后的数据和原始置换。因此必须对每一个即将改动的对象做副本，会有额外的时空间开销

---

当强烈保证不切实际时，必须提供基本保证。

---

# 请记住

- 异常安全函数即使发生异常，也不会泄露资源或允许任何数据结构败坏，分为：基本型、强烈型、不抛出异常型
