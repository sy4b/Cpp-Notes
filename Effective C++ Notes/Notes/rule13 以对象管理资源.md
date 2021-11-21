# 以对象管理资源

假设使用一个用于塑模投资行为（例如股票、债券等等）的程序库，其中各种投资类型继承自一个root class`Investment`

```cpp
class Investment{...};      // root class
```

进一步假设这个程序库通过一个工厂函数供应某个特定的`Investment`对象

```cpp
Investment* createInvestment();     // 返回指针，指向Investmenr继承体系内的动态分配对象，调用者有责任删除它。这里省略了参数
```

调用端使用了对象后，有责任将它删除

```cpp
void f(){
    Investment* pInv=creatInvestmnet();     // 调用factor函数
    ...
    delete pInv;                            // 释放pInv所指对象
}
```

这看起来妥当，但若干情况下`f`可能无法删除它得自`createInvestment`的投资对象

或许因为"..."区域中过早的`return`语句

类似情况还有在对`createInvestment`的使用及`delete`动作位于某循环内，而该循环由于`continue`或者`goto`语句过早退出

还有就是"..."区域内的语句抛出异常，这时控制流不会幸临`delete`

当然，谨慎地编写程序可以防止这类错误，但是代码在经过多次修改，维护之后，可能就会产生错误。单纯的依赖“`f`总是对调用`delete`语句”是不妥的

---

为了确保`createInvestment`返回的资源总是被释放，需要将资源放进对象内，当控制流离开`f`，该对象的析构函数会自动释放那些资源。实际上这正是隐身于本条款背后的半边想法：把资源放进对象内，我们便可以依赖C++的析构函数自动调用机制确保资源被释放

许多资源被动态分配于`heap`内，然后被用在单一区块或函数内。他们应该在控制流离开这个区域或函数时被释放。

标准程序库的`auto_ptr`正是针对这种形势设计的特制产品。`auto_ptr`是一类智能指针，其析构函数自动对其所指对象调用`delete`

```cpp
void f(){
    std::auto_ptr<Investment> pInv(createInvestment()); // 调用factory函数
    ...                                                 // 使用pInv
}                                                       // 经由auto_ptr的析构函数自动删除pInv
```

这个简单的例子示范了“以对象管理资源”的两个关键想法

- 获得资源后立即放进管理对象内。以上代码中`createInvestment`返回的资源被当作其管理者`auto_pyr`的初始值。取得资源的时机就是初始化时机（Resource Acquisition Is Initialization; RAII），因为我们几乎总是在获得一笔资源后于同一语句内以他初始化某个管理对象。有时候获得的资源被拿来赋值而不是初始化某个管理对象。但无论如何，每一笔资源在获得的同时应该立刻被放进管理对象中
- 管理对象运用析构函数确保资源被释放。无论控制流如何离开区域，一旦对象被销毁，其析构函数自然会被自动调用，于是资源被释放

由于`auto_ptr`被销毁时会自动删除它所指之物，所以一定要注意不能让多个`auto_ptr`指向同一个对象。为了预防这个问题，`auto_ptr`有一个性质：若通过`copy`构造函数或者`copy assignment`操作符复制他们，他们会变成`nullptr`，而复制所得到的指针将取得资源的唯一拥有权

```cpp
std::auto_ptr<Investment> pInv1(createInvestment());

std::auto_ptr<Investment> pInv2(pInv1);                 // 现在pInv2指向对象，pInv1被设为nullptr

pInv1=pInv2;                                            // 现在pInv1指向对象，pInv2被设为nullptr
```

这一诡异的性质意味着`auto_ptr`并非管理动态分配资源的神兵利器，例如STL容器要求发挥正常的复制行为，因此这些容器不能使用`auto_ptr`

---

`auto_ptr`的代替方案是“引用计数型智能指针(RCSP)”，持续追踪共有多少对象指向一笔资源，并在无人指向他时自动删除该资源。RCSP无法打破环状引用，例如两个其实已经没被使用的对象彼此互指，因而好像还处在被使用的状态

TR1的`tr1::shared_ptr`就是一中RCSP，所以可以这么写`f`

```cpp
void f(){
    ...
    std::tr1::shared_ptr<Investment> pInv(createInvestment());
    ...
}                   // 经由shared_ptr析构函数自动删除pInv
```

`tr1::shared_ptr`可以用于STL容器以及其他需要复制的情况

```cpp
void f(){
    ...
    std::tr1::shared_ptr<Investmenr> pInv1(createInvestment);
    std::tr1::shared_ptr<Investment> pInv2(pInv1);      // pInv1和pInv2指向同一个对象
    ...
}
```

---

`auto_ptr`和`tr1::shared_ptr`都在析构函数内做`delete`而不是`delete[]`，意味着在动态分配得到的array身上使用它们是个馊主意。可悲的是这么做可以通过编译

没有特别针对C++动态分配数组而设计的类似`auto_ptr`等东西，那是因为`vector`和`string`几乎总是可以取代动态分配得到的数组。

如果还是认为拥有针对数组设计的class比较好，那就看看`boost`吧，`boost::scoped_array`和`boost::shared_array`都提供你想要的行为

也建议不要手工释放资源，这容易发生某些错误

---

这里使用的`createInvestment`返回的未加工指针简直是对资源泄漏的一个死亡邀约，为了解决必须对其借口进行修改，见[rule18]()

---

# 请记住

- 为了防止资源泄漏，请使用RAII对象，他们在构造函数中获得资源并在析构函数中释放资源
- 两个最常被使用的RAII class是`tr1::shared_ptr`和`auto_ptr`，前者一般是更好的选择
