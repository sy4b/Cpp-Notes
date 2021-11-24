# 成对使用new和delete要采取相同形式

以下动作有何错误？

```cpp
std::string* stringArray=new std::string[100];
...
delete stringArray;
```

以上程序行为不明确。`stringArray`包含的100个`string`对象中的99个不太肯能被适当删除，他们的析构函数很可能没有被调用

---

使用`new`时，有两件事发生。第一：内存被分配出来，通过`operator new`函数。第二：针对该内存，会有一个或多个构造函数被调用。

使用`delete`时，第一：针对此内存会有一个或多个析构函数被调用，。第二：内存被释放，通过名为`operator delete`函数

`delete`的最大问题在于：即将被删除的内存里面究竟有多少个对象？这个问题的答案决定了有多少个析构函数必须被调用

---

实际上这个问题可以更简单，即将被删除的指针，指向的是单一对象还是对象数组。

当对一个指针使用`delete`，唯一能够让他知道内存中是否存在数组的办法是，加上中括号，`delete`便认定指针指向一个数组

```cpp
std::string* stringPtr1=new std::string;
std::string* stringPtr2=new std::string[100];
...
delete stringPtr1;              // 删除一个对象
delete[] stringPtr2;            // 删除对象组成的数组
```

错误混用导致的行为都是不明确的！

---

这个规则对于喜欢使用别名的人很重要，这意味着作者必须说清楚，程序员用`new`创建对象时，必须使用哪一种形式删除

```cpp
using 
typedef std::string AddressLines[4];

std::string* pal=new AddressLines;

delete [] pal;
```

为了避免错误，尽量不要对数组形式做`typedef`或`using`。这很容易达成，因为STL有其他替代品

---

# 请记住

- 在`new`表达式中使用`[]`，必须在对应的语句用`delete[]`
