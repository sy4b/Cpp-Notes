# 区分接口继承和实现继承

表面上直截了当的`public`继承概念，由两部分组成：函数接口继承和函数实现继承。这两种继承的差异在本节讨论

---

身为`class`设计者，有时候希望derived class只继承成员函数的接口（也就是声明）；有时候希望同时继承接口和实现，又能够覆盖（override）它们所继承的实现；有时候希望同时继承接口和实现，并且不允许覆盖任何东西

考虑一个展现绘图程序中各种几何形状的`class`继承体系

```cpp
class Shape{
public:
    virtual void draw()const=0;
    virtual void error(const std::string& msg);
    int objectID()const;
    ...
};
class Rectangle: public Shape{...};
class Ellipse: public Shape{...};
```

Shape是个抽象基类，客户不能创建他的实体，只能创建其derived class的实体。尽管如此，Shape还是影响了所有以`public`形式继承她的derived class

- 成员函数的接口总是会被继承

以上base class三个函数声明各不相同。有何影响呢？

首先考虑pure `virtual`函数draw，纯虚函数有两个最突出的特性：他们必须被任何继承了它们的具象`class`重新声明，而且他们在抽象基类中通常没有定义。这两个性质摆在一起你就会明白

- 声明一个pure virtual函数的目的是为了让derived class只继承其函数接口

令人意外的是我们可以为pure virtual函数提供定义。也就是说可以为`Shape::draw`提供一份实现代码，但调用他的唯一途径是指出其`class`名称

```cpp
Shape* ps= new Shape;       // error Shape抽象
Shape* ps1=new Rectangle;   // 正确
ps1->draw();                // Rectangle::draw
Shape* ps2=new Ellipse;
ps2->draw();                // Ellipse::draw
ps1->Shape::draw();
ps2->Shape::draw();         // Shape::draw
```

---

简朴的impure virtual函数呢？

- 声明简朴的非纯虚函数的目的，是让derived class继承该函数的接口和缺省实现

考虑`Shape::error`的例子

```cpp
class Shape{
public:
    virtual void error(const std::string& msg);
    ...
};
```

这个接口表示每个derived class都必须支持一个当遇上错误时可以调用的函数，但每个`class`可以自由处理错误。如果某个`class`不想针对错误做出任何特殊行为，可以退回到基类提供的缺省错误处理行为

但是这个同时制定函数声明和函数缺省行为却有可能造成危险。考虑XYZ航空公司设计的飞机继承体系，该公司只有AB两种飞机，以相同方式飞行

```cpp
class Airport{...};         // 表示机场
class Airplane{
public:
    virtual void fly(const Airport& destination);
    ...
};

void Airplane::fly(const Airport& destination){
    ...                     // 将飞机飞到指定目的地
}

class ModelA: public Airplane{...};
class ModelB: public Airplane{...};
```

以上的代码很好，表示了所有飞机都一定能飞，并且“不同型号飞机原则上需要不同的fly实现”

现在XYZ盈余大增，决定购买C型飞机。它的飞行方式和AB有所不同。然鹅程序员忘记宠幸定义fly函数

```cpp
class ModelC: public Airplane{
    ...                     // 未声明fly函数
};
```

然后代码中有这样的动作

```cpp
Airport PDX{...};           // PDX是我家附近的机场
Airplane* pa=new ModelC;
...
pa->fly(PDX);               // Airplane::fly
```

这个行为将酿成大灾难：试图以ModelA或ModelB的飞行方式来飞ModelC

问题在于ModelC未明确说出“我要”的情况下就继承了该缺省行为。幸运的是我们可以轻易做到“提供缺省实现给derived class，但除非它们明白要求否则免谈”。这个伎俩在于切断“`virtual`函数接口“和其”缺省实现“之间的链接。下面是一种做法

```cpp
class Airplane{
public:
    virtual void fly(const Airport& destination)=0;
    ...
protected:
    void defaultFly(const Airport& destination);
};

void Airplane::defaultFly(const Airport& destination){
    ...
}
```

这时候`Airplane::fly`被改成一个pure virtual函数，只提供飞行接口。其缺省行为以独立函数`defalutFly`给出。如果想要缺省实现，可以在其`fly`函数中对`defaultFly`函数做一个`inline`调用

```cpp
class ModelA: public Airplane{
public:
    virtual void fly(const Airport& destination){
        defaultFly(destination);
    }
    ...
};

class ModelB: public Airplane{
public:
    virtual void fly(const Airport& destination){
        defaultFly(destination);
    }
    ...
};
```

现在呢ModelC class不可能意外继承不正确的`fly`实现代码了，因为`Airplane`中的pure virtual函数迫使ModelC必须提供自己的`fly`版本

```cpp
class ModelC: public Airplane{
public:
    virtual void fly(const Airport& destination);
    ...
};

void ModelC::fly(const Airport& destination){
    ...         // C飞机飞到指定目的地
}
```

`Airplane::defaultFly`是个non-virtual函数，这一点也很重要，因为没有任何一个derived class应该重新定义这个函数

有些人返回用不同的函数分别提供接口和缺省实现，像上述的`fly`和`defaultFly`那样，他们关心因为过度雷同的函数名称引起的`class`命名空间污染问题。但他们也同意接口和缺省实现应该分开，这个表面上看起来的矛盾应该如何解决？

我们可以利用“pure virtual函数必须在derived class重重新声明但也可以拥有自己的实现”这一事实

```cpp
class Airplane{
public:
    virtual void fly(const Airport& destination)=0;
    ...
};

// pure virtual函数实现
void Airplane::fly(const Airport& destination){
    ...
}

class ModelA: public Airplane{
public:
    virtual void fly(const Airport& destination){
        Airplane::fly(destination);
    }
    ...
};

class ModelB: public Airplane{
public:
    virtual void fly(const Airport& destination){
        Airplane::fly(destination);
    }
    ...
};

void ModelC::fly(const Airport& destination){
    ...         // C飞机飞到指定目的地
}
```

---

最后再看看`Shape`的non-virtual函数`objectID`

```cpp
class Shape{
public:
    int objectID()const;
    ...
};
```

成员函数是个non-virtual函数，意味着他不打算在derived class重油不同的行为

- 声明non-virtual函数的目的是令derived class继承函数的接口以及一份强制性实现

---

纯虚函数、非纯虚函数以及虚函数使你可以精确制定想要派生类继承的东西。当你声明你的成员函数时，必须谨慎选择，避免两个常见错误

1. 将所有函数声明为非虚函数，这使得derived class没有余裕空间，non-virtual析构函数尤其会带来问题
2. 将所有成员函数声明为`virtual`，某些函数就是不该在派生泪被重新定义

---

# 请记住

- 接口继承和实现继承不同，在`public`继承之下，derived class总是继承base class的接口
- pure virtual函数只具体指定接口继承
- 简朴的虚函数具体制定接口继承和缺省实现继承
- non-virtual函数具体制定接口继承和强制性实现继承
