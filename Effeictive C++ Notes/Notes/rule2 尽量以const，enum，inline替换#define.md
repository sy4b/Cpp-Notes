# 尽可能以const, enum, inline替换 #define

这个条款指出以编译器替换预处理器，因为或许`#define`不被视作语言的一部分，例如做出以下事情时

```cpp
#define ASPECT_RATIO 1.563
```

记号名称`ASPECT_RATIO`或许从未被编译器看见，在编译器处理源码时就被预处理器移动走了。于是`ASPECT_RATIO`没有进入记号表，当使用这个常量获得一个编译错误时，可能错误信息只提到`1.653`，这时候追踪错误就很麻烦

---

解决办法是用一个常量替换上述宏

```cpp
const double AspectRatio = 1.563;
```

作为一个语言常量`AspectRatio`一定会被编译器看到，进入记号表内

---

当我们用常量替换`#define`，有两种特殊情况值得说说。

第一是定义常量指针。常量定义式通常放在头文件内以便被不同源码含入，因此有必要将指针本身声明为`const`

```cpp
const std::string authorName("sy4b");
```

第二个值得注意的是`class`专属常量。为了将常量的作用域限制在`class`内，必须让它成为`class`的一个成员；而为了确保此常量至多只有一份实体，必须让它成为一个`static`成员

```cpp
class GamePlayer{
private:
    static const int NumTurns = 5;      // 声明
    int scores[NUmTurns];
    ...
};
```

然鹅上面只是声明，还需要一个定义式

```cpp
const int GamPlayer::NumTurns;          // NumTurns的定义
```

这个式子应该放进一个实现文件而非头文件，由于`class`常量在声明时已经获得初始值，因此定义时不可以给初始值

---

一个枚举类型的数值可以权当`ints`被使用

```cpp
class GamePlayer{
private:
    enum {NumTurns = 5};                // enum hack——令NumTurns成为5的一个记号名称
    int scores[NUmTurns];
    ...
};
```

enum hack的行为比较像`#define`而不像`const`，例如无法取一个`enum`或者`#define`的地址。

如果不想让别人获得一个pointer或reference指向某个整数常量，`enum`可以帮助实现这个约束

enum hack是template metaprogramming的基础技术

---

另一个常见的`#define`误用情况是用它来实现宏。宏看起来像函数，但不会产生额外开销

```cpp
#define CALL_WITH_MAX(a,b) f(a)>f(b)? (a):(b)
```

这种形式的宏缺点很多，容易出错。可以用template inline函数代替，同样具有效率和安全性

```cpp
template<typename T>
inline void callWithMax(const T& a, const T& b){
    f(a>b?a:b);
}
```

---

# 要点速记

- 对于单纯常量，最好用`const`对象或`enum`替换`#define`
- 对于形似函数的宏，最好改用`inline`函数代替`#define`
