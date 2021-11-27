# 避免返回handles指向对象内部成分

假设你的程序设计矩形，每个矩形由左上角和右下角决定。为了让一个`Rectangle`对象尽可能小，你可能会决定不把定义矩形的点存放在`Rectangle`对象内，而是放在一个辅助的`struct`内，再让`Rectangle`指向它

```cpp
class Point{
public:
    Point(int x, int y);
    ...
    void setX(int newVal);
    void setY(int newVal);
    ...
}

struct RectData{
    Point ulhc;
    Point lrhc
};

class Rectangle{
    ...
private:
    std::tr1::shared_ptr<RectData> pData;
};
```

`Rectangle`的客户必须能够计算`Rectangle`的范围，所以这个`class`提供`upperLeft`函数和`lowerRight`函数。`Point`是个自定义类型，所以根据rule20，这些函数返回reference，代表底层的`Ponit`对象

```cpp
class Rectangle{
public:
    ...
    Point& upperLeft()const{return pData->ulhc;}
    Point& lowerRight()const{return pData->lrhc;}
    ...
};
```

这样的设计可以通过编译，但却是错误的。实际上他是自我矛盾的。一方面函数被声明为`const`成员函数，他们的目的只是为了提供客户一个得知相关坐标点的方法，而不是让客户修改`Rectangle`；另一方面两个函数都返回reference指向`private`内部数据，调用者于是可以通过这些`reference`更改内部数据

例如

```cpp
Point coord1(0,0);
Point coord2(100, 100);
const Rectangle rec(coord1, coord2);
rec.upperLeft().setX(50);               // 可以修改内部数据
```

这立刻带给我们两个教训：第一，成员变量的封装性最多只等于“返回其reference”的函数的访问级别。本例之中虽然`ulhc`和`lrhc`都被声明为`private`，但他们实际上却是`public`，因为`public`函数`upperLeft`和`lowerRight`传出了他们的reference；第二，如果`const`成员函数传出来一个reference，后者所指数据与对象自身有关联，而他又被存储在对象之外，那么这个函数的调用者可以修改那笔数据

---

上面所说的每件事情都是因为“成员函数返回references”，如果返回指针或迭代器，相同的情况还是发生，原因相同。references、指针和迭代器都是所谓的handles（号码牌，用来取得某个对象），而返回一个“代表对象内部数据”的handle，随之而来的便是降低对象封装性的风险。

不被公开的成员函数也是对象内部的一部分，因此也该留心不要返回他们的handles

---

只要在上述两个函数的返回类型加上`const`就可以解决问题

```cpp
class Rectangle{
public:
    ...
    const Point& upperLeft()const{return pData->ulhc;}
    const Point& lowerRight()const{return pData->lrhc;}
    ...
};
```

这样用户可以读取Points但不能涂写他们。至于封装问题，这里是蓄意放松。

---

但即便如此，upperLeft还是返回了代表对象内部的handles，有可能在其他场合带来问题。更确切地说，他可能导致dangling handles（空悬）：这种handles指向的东西不存在，最常见的来源就是函数返回值

例如某个函数返回GUI对象的外框，采用矩形形式

```cpp
class GUIObject{...};
const Rectangle boundingBox(const GUIObject& obj);

GUIObject* pgo;
...
const Point* pUpperLeft=&(boundingBox(*pgo).upperLeft());
```

对boundingBox的调用获得了一个全新的Rectangle，这个对象最后会被销毁，所以最终pUpperLeft指向一个被析构的对象，变成了空悬

这就是为什么函数返回一个handle代表对象内部成分总是危险的原因。

---

但不意味着绝不可以让成员函数返回handles，哟时候必须那么做。例如`operator[]`就允许。

---

# 请记住

- 避免返回handles（包括references、指针、迭代器）指向对象内部。这样可以增加封装性、帮助`const`成员函数的行为像个`const`，并将发生空悬的可能性降到最低
