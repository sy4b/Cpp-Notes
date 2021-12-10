# 只要有可能使用constexpr，就使用它

`constexpr`应用于对象时，就是一个加强版的`const`，应用于函数时却有着相当不同的定义

---

从`constexpr`对象讲起，这些对象具备`const`属性，在编译阶段就已知，值在翻译期间（包括编译和链接）决定。

在编译阶段已知的值可以拥有种种特权，例如放置在只读内存中（对于嵌入式系统开发很重要）。在更广泛的应用场景里，在编译阶段就已知的常量整型值可用在C++要求整型常量表达式的语境中，包括数组的尺寸规格、整型模板实参、枚举量、对齐规格等等。

```cpp
int sz;
...
constexpr auto arraySize1=sz;       // 错误，sz的值在编译阶段未知
std::array<int, sz> data1;          // 错误，同上
constexpr auto arraySize2=10;       // 没问题
std::array<int, arraySize2> data2;  // 没问题
```

`const`并未提供和`constexpr`一样的保证，因为`const`对象不一定经由编译期已知值进行初始化

```cpp
int sz;
const auto arraySize=sz;            // 没问题
std::array<int, arraySize> data;    // 错误
```

---

`constexpr`函数，在调用时如果传入编译期常量，则产出编译期常量，否则产出运行期值

- `constexpr`函数可以用在要求编译期常量的语境中，此时如果传给一个`constexpr`函数的实参值在编译期已知，那么结果也会在编译期计算出来。如果任何一个实参在编译期未知，那么代码无法通过编译
- 在调用`constexpr`函数时，如果传入的值有一个或多个在编译期未知，则该函数运作方式和普通函数没有差别。

假设使用`std::pow`函数计算的值设定`std::array`的尺寸。难点在于`std::pow`不是`constexpr`的，无法直接使用返回值。先来看看满足条件的`pow`的用法

```cpp
constexpr int pow(int base, int exp)noexcept{
    ...     // 如何实现？
}
constexpr auto numConds=5;
std::array<int, pow(3, numConds)> results;
```

函数前的`constexpr`并不表明函数返回一个`const`，而是说如果参数都是`constexpr`，函数的返回结果就可以当做一个`constexpr`值使用

由于`constexpr`函数必须在传入编译期常量时返回编译期结果，他们的实现就必须加以限制。

在C++11中，限制为：`constexpr`函数不得包含多于一个可执行语句，即只有一条`return`。而if-else可以用`?:`实现，循环可以用递归完成

```cpp
constexpr int pow(int base, int exp)noexcept{
    return (exp==0 ? 1 : base*pow(base, exp-1));
}
```

C++14中限制被放宽了，以下的实现也可以

```cpp
constexpr int pow(int base, int exp)noexcept{
    auto result=1;
    for(int i=0;i<exp;++i){
        result*=base;
    }
    return result;
}
```

`constexpr`函数仅限于传入和返回字面型别，这样的型别持有在编译期就能决议的值。在C++11中，所有的内建型别除了`void`都符合。用户自定义型别也有可能是字面型别，因为他的构造函数和其他成员函数可能也是`constexpr`型别

```cpp
class Point{
public:
    constexpr Point(double xVal=0, double yVal=0)noexcept:x(xVal), y(yVal){}
    constexpr double xValue()const noexcept{return x;}
    constexpr double yValue()const noexcept(return y;)
    void setX(double newX)noexcept{x=newX;}
    void setY(double newY)noexcept{y=newY;}
private:
    double x, y;
};
```

此处Point的构造函数被声明为`constexpr`函数，传入实参在编译期可知，则构造出来的Point对象的数据成员其值也是编译期可知，那么对象自然具有`constexpr`属性

```cpp
constexpr Point p1(9.4, 27.7);
constexpr Pount p2(28.8, 5.3);
```

类似的，访问器也可以声明为`constexpr`

```cpp
constexpr Point midPoint(const Point& p1, const Point& p2)noexcept{
    return {(p1.xValue()+p2.xValue())/2, (p1.yValue()+p2.yValue())/2};
}

constexpr auto mid=midPoint(p1, p2);
```

这意味着尽管对象mid的初始化过程涉及了构造函数、访问器、非成员函数的调用，却仍可以在只读内存中创建！你可以将诸如`mid.xValue()*10`的表达式运用到模板形参中，或者指定枚举量的表达式中。

传统上编译期和运行期完成的工作之间的界限已经开始模糊。并且某些传统上在运行期完成的工作可以迁移到编译期完成。迁移的越多，软件运行更快，但编译更慢

---

C++11中，有两个限制使Point的成员函数setX和setY不能声明为`constexpr`：第一，他们修改了操作对象，而在C++11中，`constexpr`函数都隐式声明为`const`；第二：他们的返回型别都是`void`，在C++11中不属于字面型别。

不过以上的限制在C++14中都被解除了，所以在C++14中，设置器也可以声明为`constexpr`

```cpp
class Point{
public:
    ...
    constexpr void setX(double newX)noexcept{
        x=newX;
    }
    constexpr void setY(double newY)noexcept{
        y=newY;
    }
    ...
};
// 可以写出这样的代码
constexpr Point reflection(const Point& p)noexcept{
    Point result;
    result.setX(-p.xValue());
    result.setY(-p.yValue());
    return result;
}
// 客户代码可以是这样子
constexpr Point p1(9.4, 27.7);
constexpr Point p2(28.8, 5.3);
constexpr auto mid=midPoint(p1, p2);
// 编译期已知
constexpr auto reflectedMid=reflection(mid);
```

---

# 要点速记

- `constexpr`对象都具有`const`属性，并且由编译期已知值完成初始化
- `constexpr`函数如果传入实参编译期已知，那么会产出编译期结果
- 比起非`constexpr`对象或`constexpr`函数而言，`constexpr`对象或函数可以用在一个作用域更广的语境中