# 只要函数不会发射异常，就为其加上noexcept声明

C++98中异常规格可谓喜怒无常。你必须梳理出可能发射的所有异常型别。如果修改函数，那么一般异常规格也需要修订，可能会破坏客户代码

C++11则专注函数是否发射异常，`noexcept`声明就是为不发射异常的函数准备

---

函数是否加上`noexcept`声明关乎接口设计。是否发射异常的行为是客户端关注的核心，调用方可以查询函数的noexcept状态，决定如何调用。这样一来`noexcept`和`const`一样都是重要的信息；

此外`noexcept`可以让编译器生成更好的目标代码，理解这一点需要考察C++98和C++11在表达函数不会发射异常时的差异。如下

```cpp
int f(int x)throw();        // C++98
int f(int x)noexcept;       // C++11
```

如果在运行期一个异常逃离f的作用域，则f的异常规格被违反。

C++98异常规格下，调用栈会开解到f的调用方，执行一些其他动作后，终止程序；C++11异常规格下，可能只是开解栈，然后终止程序。这两者的差别对于代码生成造成很大影响。

在带有`noexcept`声明的函数中，优化器不需要在异常传出函数的前提下，将执行期栈保持在可开解状态，也不用在异常逃离函数的前提下，保证其中的对象以其被构造顺序的逆序完成析构。

而异常规格为`throw()`的函数就享受不到以上的优化灵活性

```cpp
RetType function(params)noexcept;       // 最优化
RetType function(params)throw();        // 优化不够
RetType function(params);               // 优化不够
```

优化已经构成了一个很好的理由，给任何已知不会发生异常的函数加上`noexcept`声明

---

某些情况更加典型，例如**移动操作**

假设有一段C++98代码，使用一个`vector<Widget>`型别对象，而且不时地通过`push_back`向其中加入Widget型别对象

```cpp
std::vector<Widget> vw;
...
Widget w;
...
vw.push_back(w);
```

你想要使用C++11的移动语义来提高性能，并且不想将所有代码都升级到C++11。需要保证Widget具有移动操作。

向vector添加元素的时候可能出现空间不够的情况，也就是size和capacity相等的时刻，此时`std::vector`会分配一个更大的内存存储元素，并且将现存的元素转移到新的内存中，将旧的对象析构，这使得`push_back`可以提供强异常安全保证：如果复制元素过程抛出异常，旧的对象会保持不变。

C++11中一个自然而然的优化是针对`std::vector`的对象元素复制操作替换成移动操作，但这样违反了`push_back`的强异常安全保证：如果取出了n个元素，而移动第n+1个元素抛出异常，则`push_back`无法完成，此时原始的对象已经被修改，可能无法恢复原始状态

这是个严重的问题，遗留代码的行为可能会依赖`push_back`的强异常安全保证。这样C++11就不能一声不吭地将复制全部换成移动，除非他知道移动操作不会发生异常。

`std::vector::push_back`采用了“能移动就移动，必须复制才复制”的策略。而这并不是标准库中唯一这样做的函数，C++98中其他因为强异常安全保证的函数的行为也是一样。此时`noexcept`声明就极其重要

---

swap函数是另一个极其需要`noexcept`的例子。

标准库中的`swap`是否带有`noexcept`声明，取决于用户定义的swap是否带有`noexcept`声明。例如标准库为`std::pair`准备的`swap`如下：

```cpp
template<class T, size_t N>
void swap(T(&a)[N], T(&b)[N])noexcept(noexcept(swap(*a, *b)));      // 外层noexcept是指定符，内层是运算符

template<class T1, class T2>
strcut pair{
    ...
    void swap(pair& p)noexcept(noexcept(swap(first, p.first)) &&
                                noexcept(swap(second, p.second)));
    ...
};
```

---

讲到这，不要忘记`noexcept`声明是函数接口的组成部分，所以必须保证在函数实现长期具有`noexcept`的前提下，才能给他`noexcept`声明。

事实上大部分函数是异常中立的，这类函数自身不抛出异常，但他们的函数可能会抛出异常。这种情况出现时，异常中立的函数会允许发射的异常传递到函数调用栈更深一层。这导致异常中立函数永远不具备`noexcept`性质

有些函数具有不发射异常的自然实现，只要有可能，就应该为它们加上`noexcept`性质。

对于某些函数，比如内存释放函数、析构函数都隐式具备`noexcept`性质，这样一来就无需显式说明`noexcept`

---

# 要点速记

- `noexcept`声明是函数接口的组成部分，意味着调用方可能对其有所依赖
- 相对于不带`noexcept`的函数，带有`noexcept`的函数有更多机会得到优化
- `noexcept`性质对于移动操作、swap、内存释放函数和析构函数最有价值
- 大多数函数都是异常中立的，不具备`noexcept`性质