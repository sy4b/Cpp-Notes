# 绝不重定义继承而来的non-virtual函数

假设class D由class B以`public`形式派生而来，class B定义有一个`public`成员函数mf。由于mf的参数和返回值都不重要，所以假设两者都是`void`类型

```cpp
class B{
public:
    void mf();
    ...
};

class D: public B{...};
```

虽然现在对B D mf一无所知，但面对一个类型为D的对象x有以下的行为

```cpp
D x;
B* pB=&x;
pB->mf();

D* pD=&x;
pD->mf();
```

如果两次调用行为不相同，你会感觉十分惊讶

事实上可能还真的会出现行为不同的情况，那就是：如果mf是个non-virtual函数，而D定义有自己的mf版本

```cpp
class D: public B{
public:
    void mf();
    ...
};
pB->mf();
pD->mf();
```

造成的原因是：non-virtual函数是静态绑定的（rule37）。意思是，pB被声明为一个pointer-to-B，通过pB调用的non-virtual函数永远是B定义的版本，即使pB指向一个派生类

与之相对的是，`virtual`函数是动态绑定，所以他们不受这个问题之苦。如果mf是一个`virtual`函数，不论通过pB还是pD调用mf，都会导致调用D::mf，因为pB和pD都真正指向的都是一个类型为D的对象

---

从理论层面讨论为什么绝不能重新定义继承而来的non-virtual函数

rule32指出，`public`继承意味着is-a关系，rule34描述了在`class`内声明一个non-virtual函数会为该`class`建立一个不变性，凌驾于特异性之上

那么

- 适用于B的每一件事，也适用于D对象，因为每一个D对象都是一个B对象
- B的derived class一定会继承mf的接口和实现，因为mf是B的一个non-virtual函数

现在如果D重定义mf，你的设计就会出现矛盾

---

回到rule7，指出多态性base class的析构函数必须是`virtual`。现在来看，如果你违反这个规则，那也就违反了本条规则

---

# 请记住

- 绝不要重新定义继承而来的non-virtual函数
