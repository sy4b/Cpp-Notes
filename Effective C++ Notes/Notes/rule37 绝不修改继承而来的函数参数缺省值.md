# rule37 绝不重定义继承而来的缺省参数初始值

一个派生类只能继承`virtual`和non-virtual函数，rule36指出重新定义一个继承而来的non-virtual函数永远是错误的。因此本rule的讨论限为：继承一个带有缺省参数值的`virtual`函数

这种情况下，本条款成立的理由很明确：`virtual`函数是动态绑定，而缺省参数值是静态绑定

---

复习一下什么是静态绑定，什么是动态绑定？

对象的所谓静态类型，就是他在程序中被声明时采用的类型

```cpp
class Shape{
public:
    enum ShapeColor{Red, Green, Blue};
    // 所有的♈️都必须提供一个函数，画出自己
    virtual void draw(ShapeColor color=Red)const=0;
    ...
};

class Rectangle: public Shape{
public:
    // 注意，赋予了不同的缺省参数值，这很糟糕！
    virtual void draw(ShapeColor color=Green)const;
    ...
};

class Circle: public Shape{
public:
    virtual void draw(ShaptColor color)const;
    // 这么写的话客户以对象调用此函数时，必须指定参数值
    // 因为静态绑定下这个函数不会从base class继承缺省参数值
    // 但如果用指针或reference调用，就可以不指定
    // 因为动态绑定下这个函数就会从base继承缺省参数值
    ...
};
```

现在考虑这些指针

```cpp
Shape* ps;                      // 静态类型为Shape*
Shape* pc=new Circle;           // 静态类型为Shape*
Shape* pr=new Rectangle;        // 静态类型为Shape*
```

对象所谓动态类型则是指”目前所指对象的类型“，也就是说，动态类型可以表述出一个对象将会有什么行为。以上例而言，pc的动态类型是Circle*，pr的动态类型是Rectangle*，ps没有动态类型，因为它尚未指向任何对象

动态类型一如其名称所示，可在程序执行过程中改变（通常是经由赋值动作）

```cpp
ps = pc;        // ps的动态类型如今是Circle*
ps = pr;        // ps的动态类型如今是Rectangle*
```

`virtual`函数是动态绑定而来，意思是调用一个`virtual`函数时，究竟调用哪一份函数的实现代码，取决于发出调用的那个对象的动态类型：

```cpp
pc->draw(Shape::Red);   // 调用Circle::draw(Shape::Red)
pr->draw(Shape::Red);   // 调用Rectangle::draw(Shape::Red)
```

---

以上都是老调重弹，但是当你考虑带有缺省参数值的`virtual`函数，花样来了！

`virtual`函数时动态绑定，而缺省参数值是静态绑定。你可能会在调用一个定义于derived class内的`virtual`函数的同时，却使用base class为它制定的缺省参数值

```cpp
pr->draw();         // 调用Rectangle::draw(Shape::Red)
```

此例之中，pr的动态类型是Rectangle*，所以调用的是Rectangle的`virtual`函数，一如你所预期。Rectangle::draw函数的缺省参数值应该是GREEN，但由于pr的静态类型时SHape*，所以这一个调用的缺省参数值来自Shape class而不是Rectangle class！结局是这个函数调用有着奇怪并且几乎绝对没人预料到的组合

以上事实不只局限于ps pc pr都是指针的情况；即使把指针换成reference，问题仍然存在。重点在于draw是个`virtual`函数，而他有个缺省参数值在derived class中被重新定义了

---

为什么C++用这种乖张的方式运作呢？答案在于运行期效率

如果缺省参数值是动态绑定，编译器就必须有某种办法在运行期为`virtual`函数决定适当的参数缺省值。这比目前实行的“在编译期决定”的机制更慢、很复杂

---

这一切都很好，但如果你试着遵循这条规则，并且同时提供缺省参数值给base class和derived class的用户，又会发生什么事呢

```cpp
class Shape{
public:
    enum ShapeColor{Red, Green, Blue};
    virtual void draw(ShapeColor color=Red)const=0;
    ...
};

class Rectangle: public Shape{
public:
    virtual void draw(ShapeColor color=Red)const;
    ...
};
```

啊呕，代码重复。更糟的是代码重复又带着相依性。如果Shape中的缺省参数值改变了，多有derived class都必须改变

聪明的办法是考虑替代设计。rule35列出了不少`virtual`函数的替代设计，其中之一是NVI手法：令base class中一个public non-virtual函数调用private virtual函数，后者可被derived class重新定义

```cpp
class Shape{
public:
    enum ShapeColor{Red, Green, Blue};
    void draw(ShapeColor color=Red)const{
        doDraw(color);
    }
    ...
private:
    virtual void doDraw(ShapeColor color)const=0;
};

class Rectangle: public Shape{
public:
    ...
private:
    virtual void doDraw(ShapeColor color)const; // 不用指定缺省参数值
}
```

由于non-virtual函数绝不可被derived class覆写，所以这个设计很清楚地使得draw函数的color缺省参数值总是Red

---

# 请记住

- 绝不要重新定义一个继承而来的缺省参数值，因为缺省参数值是静态绑定，`virtual`函数却是动态绑定