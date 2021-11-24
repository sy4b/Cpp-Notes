# 将成员变量声明为private

为什么不采用`public`成员变量呢？

---

我们从语法一致性开始。如果成员变量不是`public`，客户就只能通过成员函数才能访问。如果`public`借口哪的每样东西都是函数，客户就不要在访问时记住是否应该使用小括号

---

使用函数可以让你对成员变量的处理油更加精确的控制。假设成员变量为`public`，那么每个人都可以读写他

但如果以函数取得或者设定其值，就可以实现：不准访问、只读访问以及读写访问。见鬼，甚至可以实现只写访问

```cpp
class AccessLevel;{
public:
    ...
    int getReadOnly()const{return readOnly;}
    void setReadWrite(int value){readWrite=value;}
    int getReadWrite()const(return readWrite;)
    void setWriteOnly(int value){wirteOnly=value;}
private:
    int noAccess;
    int readOnly;
    int readWrite;
    int writeOnly;
};
```

如此细致的划分访问控制很有必要

---

更具有说服力的原因是：封装。如果通过函数访问成员变量，日后可以改用某个计算替换这个成员变量而`class`用户一点也不会知道内部已经改变

举个例子，写一个自动测速程序：汽车通过时，其速度就被计算并填入一个速度收集器内

```cpp
class SpeedDataCollection{
    ...
public:
    void addValue(int speed);       // 添加一笔新数据
    double averageSoFar()const;     // 返回平均速度
    ...
};
```

现在考虑成员函数averageSoFar。做法之一就是在`class`内设计一个成员变量，记录所有速度的平均值，函数返回成员变量即可。另一个做法是每次调用函数时重新计算平均值

第一种做法会使得每个对象变大，因为必须存放当前平均值，累积和，数据数量；第二个做法就十分节省空间，但执行速度会比较慢。具体哪个更好，得看情况。

可见将成员变量封装，可以提供较大的弹性

封装的重要性还表现在：如果对客户封装成员变量，就可以确保`class`的约束条件总是会获得维护，因为只有成员函数可以影响他们。

---

`protected`成员变量呢？

rule23指出，某些东西的“封装性”与“当其内容改变时可能造成的代码破坏量”成反比。所谓改变，也许是从`class`中删除它

假设我们有一个`public`成员变量，最终移除了他，多少代码会被破坏呢？所有使用它的客户代码都会被破坏

假设我们有一个`protected`成员变量，删除它后所有使用它的derived class都会被破坏

这两者的封装性都很差，因此实际上只有两种访问权限：`private`（提供封装）和其他（不提供封装）

---

# 请记住

- 切记将成员变量声明为`private`，这可以赋予客户访问数据的一致性、可细致划分访问控制、允许约束条件获得保证，并提供作者十足的实现弹性
- `protected`并不比`public`更具有封装性
