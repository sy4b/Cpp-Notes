### 减少名字空间污染

一个通用规则：如果一对大括号里声明一个名字，则该名字的可见性就被限定在括号括起来的作用域内

但这个规则不适用于C++98风格的枚举型别中定义的枚举量。这些枚举量的名字属于包含这个枚举型别的作用域，这也意味着在这个作用域内不能有其他实体取相同的名字

```cpp
enum Color{black, white, red};          // black, white, red 的作用域和Color相同

auto white = false;                     // Error!已经有了white的声明

```

这些枚举量的名字泄漏到枚举型别所在的作用域这一事实，催生了此枚举型别的官方术语：不限范围的枚举型别。

他们在C++11中的对等物，限定作用域的枚举型别，却不会以这样的方式泄漏名字

```cpp
enum class Color{black, white, red};    // black white red 的作用域被限定在Color内

auto white = false;                     // 没有问题

Color c = white;                        // 错误，范围内没有white枚举量

Color c = Color::white;                 // 正确

auto c = Color::white;                  // 没错，并且符合Rule#5的建议
```

由于限定作用域的枚举型别是通过`enum class`声明的，所以有时也被称为枚举类

---

### 限定作用域的枚举型别的枚举量是更强的

不限范围的枚举型别中的枚举量可以隐式转换到整数型别，并由此进一步转换到浮点型别。如此一来以下的怪胎语义却成为完全合法的了

```cpp
enum Color{black, white, red};                          // 不限范围的枚举型别

std::vector<std::size_t> primeFactors(std::size_t x);   // 函数，返回x的质因数

Color c = red;
...
if(c<4.5){                                              // 将一个Color型别和double做比较
    auto factors = primerFactors(c);                    // 计算一个Color值的质因数
}
```

而限定作用域的枚举型别语义完全不同，从限定作用域的枚举型别到任何其他型别都不存在隐式转换路径

```cpp
enum class Color{black, white, red};                    // 限定作用域的枚举型别

std::vector<std::size_t> primeFactors(std::size_t x);   // 函数，返回x的质因数

Color c = Color::red;                                   // 需要加上饰辞
...
if(c<4.5){                                              // 错误！无法将一个Color型别和double做比较
    auto factors = primerFactors(c);                    // 错误！不能将Color参数传入要求std::size_t的函数
}
```

如果真的想到实施`Color`到另一型别的转换，可以`static_cast`强制型别转换，合法但很不自然

---

乍一看限定作用域的枚举型别还有一个优点，那就是可以进行前置声明。

```cpp
enum Color;         // 错误

enum class Color;   // 正确
```

其实C++11中不限范围的枚举型别也可以前置声明，但需要完成一些额外工作。这些额外工作是由这个事实带来的：一切枚举型别在C++中都会由编译器选择一个整数型别作为其底层型别

对于像`Color`这样的不限范围的枚举型别`enum Color{black, white, red}`，编译器会选择`char`作为底层型别，因为只有三个值需要表示

有些枚举型别取值范围就大得多，例如

```cpp
enum Status{
    good = 0, failed =1, incomplete = 100, corrupt = 200, indeterminate = 0xFFFFFFFF
};
```

这里除非是在不常见的某些机型上，编译器一般都会选取比`char`范围更大的整数型别来表示

为了节约内存，编译器通常会为枚举型别选用足够表示枚举量取值的最小底层型别。因此C++98只提供了枚举型别定义的支持，枚举型别声明则不允许。如此一来编译器就可以在枚举型别被使用之前，逐个确定地层类型选取哪个

前置声明能力的缺失会增加编译依赖性，可能只是添加一个枚举量，就导致整个系统重新编译。这种情况利用C++11位枚举型别提供的前置声明能力即可破处

```cpp
enum class Status;                      // 前置声明

void continueProcessing(Status s);      // 取用前置声明的枚举型别
```

若头文件包含这些声明，则`Status`定义发生修改时，不会要求重新编译

---

限定作用域的枚举型别的底层型别是已知的，而对于不限范围的枚举型别，可以指定这个底层型别

默认地，限定作用域的枚举型别的底层型别是`int`，当然也可以进行修改

```cpp
enum class Status;                          // 底层型别是int

enum class Status: std::unit32_t;           // 底层型别是std::unit32_t（该型别在<cstdint>中包含）
```

采用上述两种方法中的一种，编译器都能知晓限定作用域的枚举型别中的枚举量尺寸

如果要指定不限范围的枚举型别的底层型别，做法和限定作用域的枚举型别一样，这么做之后，不限范围的枚举型别也能进行前置声明了

```cpp
enum Color: std::unit8_t;                    // 不限范围的枚举型别，底层型别是std::unit8_t
```

底层型别同样也可以在枚举型别定义中指定

```cpp
enum class Status: std::unit32_t{
    good = 0, failed =1, incomplete = 100, corrupt = 200, indeterminate = 0xFFFFFFFF
};
```

---

在至少一种情况下，不限范围的枚举型别还是有用的，那就是当需要引用C++11中的`std::tuple`型别的各个域时。例如准备一个元组，表示名字、电子邮件和声望

```cpp
using UserInfo = std::tuple<
    std::string, 
    std::string, 
    std::size_t>;
```

在源文件中，程序员就不太清楚每个域代表什么，也不太可能去记住头文件中的声明，此时使用一个不限范围的枚举型别就可以消解这种记忆需要

```cpp
enum UserInfoFields{uiName, uiEmail, uiReutation};

UserInfo uInfo;             // UserInfo 定义同上
...
auto val = std::get<uiName>(uInfo);     // 取得第一个域
```

这段代码能够运作依赖于`UserInfoFields`向`std::size_t`的隐式转换

---

# 要点速记

- C++98风格的枚举型别，现在称为不限范围的枚举型别
- 限定作用域的枚举型别尽在枚举型别那可见，只能通过强制类型转换到其他型别
- 都支持底层型别指定，限定范围的枚举型别默认底层是int，而不限范围的没有默认底层
- 限定作用域的枚举型别总是可以前置声明，而不限范围的枚举型别只有在指定了默认底层型别后才可以前置声明
