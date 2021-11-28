# 避免遮掩继承而来的名称

这个rule其实和继承无关，而是和作用域有关。我们都知道在这样的代码中

```cpp
int x;              // global变量
void someFunc(){
    double x;       // local变量
    std::cin>>x;    // 读取一个新值赋予local变量x
}
```

这个读取数据的语句指涉的是local变量x而不是global变量x，因为内层作用域的名字会遮掩外围作用域的名称。

只要名称一样，不管类型是否相同，都会内层遮掩外层

---

现在导入继承，实际上derived class作用域被嵌套在base class作用域内。类似这样

![](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Graphs/截屏2021-11-28%20下午5.47.31.png)

假设derived class内的mf4实现如下

```cpp
void Derived::mf4(){
    ...
    mf2();
    ...
}
```

编译器看到这里使用名称mf2后，首先查找local作用域，找不到mf2，然后往外层移动查找，找到了mf2...

考虑更加复杂的情况，重载mf1和mf3

![](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Graphs/截屏2021-11-28%20下午5.58.09.png)

虽然看起来吓人，但以作用域为基础的名称遮掩规则并没有改变，因此base class内所有名为mf1和mf3的函数都被derived class内的mf1和mf3函数遮掩掉了。从名称查找观点来看，Base::mf1和Base::mf3不被Derived继承

```cpp
Derived d;
int x;
...
d.mf1();        // Derived::mf1()
d.mf1(x);       // error
d.mf2();        // Base::mf2()
d.mf3();        // Derived::mf3()
d.mf3(x);       // error
```

---

这些行为背后的基本理由是为了防止你在程序库或应用框架内建立新的derived class时时附带地从疏远的base class继承重载函数

可以用`using`声明式防止继承遮盖了重载函数

![](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Graphs/截屏2021-11-28%20下午6.47.53.png)

现在继承机制一如既往的进行了。如果你继承base class并加上重载函数，又希望重新定义或覆盖其中一部分，那么必须为那些原本会被遮掩的每个名称引入一个`using`声明，否则你希望继承的名称会被遮掩

有时候并不想继承base class的所有函数，这是可以理解的。在`public`继承之下，这点绝对不可能发生，因为它违反了`public`继承所暗示的base和derived class之间的is-a关系。

然而在`private`继承之下是有可能有意义的。此时我们不用`using`声明，需要一个简单的转交函数（forwarding function）

```cpp
class Base{
public:
    virtual void mf1()=0;
    virtual void mf1(int);
    ...                     // 和前面一样
};
class Derived: private Base{
public:
    virtual void mf1(){Base::mf1()} // 转交函数，inline
    ...
};
Derived d;
int x;
d.mf1();            // Derived::mf1()
d.mf1(x);           // error Base::mf1()被遮掩了
```

`inline`转交函数的另一个用途是为那些不支持`using`声明的老旧编译器开辟一条新路

---

# 请记住

- derived class内的名称会遮掩base class内的同名名称
- 为了让被遮掩的名称重见天日，可以用`using`声明或转交函数
