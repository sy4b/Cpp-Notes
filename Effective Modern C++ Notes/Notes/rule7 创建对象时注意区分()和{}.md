C++11 中进行初始化的方式包括使用小括号、等号或大括号

```cpp
int x(0);

int y = 0;

int z{0};         // int z={0} 通常相同处理
```

对于用户定义的型别，初始化和赋值调用不同的函数

```cpp
Widget w1;        // 默认构造函数

Widget w2 = w1;   // 复制构造函数

w2 = w1;          // 复制赋值运算符
```

---

### 统一初始化

C++11引入统一初始化，它的基础是大括号形式。

```cpp
std::vector<int> v{1,3,5};
```

大括号可以用来为非静态成员指定默认初始化值，也可以用`=`，但不能用小括号

```cpp
class Widget{
private:
  int x{0};
  int y=0;
  int z(0); // Error！
};
```

不可复制的对象，例如`std::atomic`，可采用大括号和小括号来初始化，却不能使用`=`

```cpp
std::atomic<int> ai1{0};
std::atomic<int> ai2(0);
std::atomic<int> ai3=0; // Error!
```

---

### 大括号有一项新特性：禁止内建型别之间进行隐式窄化型别转换

```cpp
double x, y, z;
int sum1{x+y+z};      Error!double可能无法用int表达
```

---

### 大括号对解析语法带来的问题免疫

C++规定任何可以解析为声明的都要解析为声明。程序员可能想要默认构造一个对象，却被解析为声明一个函数

```cpp
Widget w1(10);          // 调用默认构造函数，传入参数10
Widget w2();            // 想要调用一个没有形参的构造函数，却声明了一个Widget型别返回对象的函数
```

由于不能用大括号指定形参列表，所以可以：

```cpp
Widget w3{};
```

---

### 大括号初始化的缺陷

大括号初始化的缺陷来自于初始化物、`std::initializer_list`以及构造函数重载决议之间的关系，例如`auto`推导的大括号初始化物型别是`std::initializer_list`

在构造函数被调用时，只要形参中没有任何一个具备`std::initializer_list`型别，那么小括号和大括号的意义没有区别

```cpp
class Widget{
public:
  Widget(int i, bool b);      // 1
  Widget(int i, double d);    // 2
};

Widget w1(10, true);          // 1
Widget w2{10, true};          // 1
Widget w3(10, 5.0);           // 2
Widget w4{10, 5.0};           // 2
```

如果一个或多个构造函数声明了任何一个具备`std::intializer_list`的形参，那么采用大括号初始化的调用语句会强烈地优先选用带有该类型形参的重载版本

```cpp
class Widget{
public:
  // 构造函数1 2 同上
  Widget(std::initializer_list<long double> i1);    // 3
};

Widget w1(10, true);      // 1
Widget w2{10, true};      // 3，10和true被强制转换为long double
Widget w3(10, 5.0);       // 2
Widget w4{10, 5.0};       // 3，10和5.0被强制转换为long double
```

平时执行复制或者移动的构造函数也可能被带有`std::intializer_list`的形参的构造函数劫持

```cpp
class Widget{
public:
  Widget(int i, bool b);
  Widget(int i, double d);
  Widget(std::initializer_list<long double> i1);
  operator float()const;  // 强制转换为float
};

Widget w5(w4);            // 调用复制构造函数

Widget w6{w4};            // 3，w4的返回值先被强制转换为float，然后被强制转换为long double

Widget w7(std::move(w4)); // 调用移动构造函数

Widget w8{std::move(w4)}; // 3
```

编译器想要将大括号初始化物匹配带有`std::intializer_list`型别形参的构造函数决心十分强烈，以至于最优选的构造函数无法被调用时，这种决心还是占上风

```cpp
class Widget{
public:
  Widget(int i, doubele);
  Widget(int i, bool b);
  Widget(std::initializer_list<bool> i1);
};

Widget w{10, 5.0};        // Error
```

这个例子中，前两个构造函数会被忽略，而要调用第三个构造函数就要求将一个`int`和`double`转换为`bool`，而这两个强制类型转换都是窄化的，在大括号初始化物内部是禁止的，因此整段代码无法通过编译

只有在找不到任何方法将大括号初始化物中的实参转换成`std::initializer_list`模板中的型别时，编译器才会去检查普通的重载决议。例如无法将`int`和`double`转换为`string`

```cpp
class Widget{
public:
  Widget(int i, bool b);                          // 1
  Widget(int i, double d);                        // 2
  
  Widget(std::initializer_list<std::string> i1);  // 3
};

Widget w1{10, true};      // 1
Widget w2{10, 5.0};       // 2
```

---

### 特殊边界用例

假设使用一对空大括号创建一个对象，而该对象支持默认构造函数，又带有支持`std::initializer_list`类型形参的构造函数。那么这对空大括号意义如何？

语言规定，在这种情况下应该执行默认构造，空大括号表示的是没有实参，而不是空的`std::initializer_list`

如果的确想要调用一个带有`initializer_list`型别形参的构造函数，并且传入一个空的`std::initializer_list`实参，可以将空大括号作为构造函数实参，即把一对空大括号放入一对小括号或大括号之中

```cpp
Widget w4({});    // 带有std::initializer_list型别的形参的构造函数，传入一个空的std::initializer_list

Widget w5{{}};    // 同上
```

---

直接受到上述规则影响的一个类就是`std::vector`

- 这个类中有一个不带`std::initializer_list`型别形参的构造函数，允许指定容器尺寸以及初始化值
- 还有一个带有`std::initializer_list`型别形参的构造函数，允许逐个指定`std::vector`中的元素

使用大小括号会有完全不同的结果

```cpp
std::vector<int> v1(10, 20);          // 容器内有10个元素，初始值均为20
std::vector<int> v2{10, 20};          // 容器内有2个元素，初始值为10和20
```

一般来说一个设计成功的类，构造函数设计为使用大小括号都不影响使用才算好，`std::vector`的接口算是失败的

开发模板时，在创建对象时选用小括号还是大括号是一个难题。举例来说，如果想以任意数量的实参创建一个任意型别的对象，那么一个可变模板可以让这种概念直截了当

```cpp
template<tyepname T, typename... Ts>
void doSomeWork(Ts&&... params){
  ... // 利用params创建局部对象T
}
```

要将伪代码变为实际代码有两种方式

```cpp
T localObject(std::forward<Ts>(params)...);

T localObject{std::forward<Ts>(params)...};
```

调用代码

```cpp
std::vector<int> v;
...
doSomeWork<std::vector<int>>(10, 20);
```

两种方式结果不一样，孰对孰错？只有开发者才知道

---

# 要点速记

- 大括号初始化可以应用的语境最宽泛，可以阻止隐式窄化型别转换，且对解析语法免疫
- 构造函数重载决议期间，只要有任何可能，大括号初始化物就会与带有`std::initializer_list`形参类别的函数相匹配，即使其他构造函数有更加匹配的形参列表
- 使用小括号或大括号可能结果大相径庭，例如`std::vector`
- 对模板内容进行对象创建时，究竟使用小括号还是大括号是一个棘手问题
