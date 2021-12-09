字面型常量`0`的型别是`int`而不是指针

当C++在只能使用指针的语境中发现了一个`0`，也会勉强解释为空指针，但说到底这是一个不得已而为之的行为

`NULL`同样也不具备指针型别

```cpp
void f(int);      // f的三个重载版本
void f(bool);
void f(void*);

f(0);             // 调用f(int)
f(NULL);          // 调用f(int)，从来不会调用f(void*)
```

---

`nullptr`的优点在于，他不具备整形型别。实际上它也不具备指针型别，但可以想象为一种任意型别的指针

`nullptr`的实际型别是`std::nullptr_t`，并且在一个漂亮的循环定义下，`std::nullptr_t`的定义被指为`nullptr`的型别。型别`std::nullptr_t`可以隐式转换到所有的裸指针型别，这就是为什么`nullptr`可以扮演所有型别指针的原因

调用重载函数`f`时传入`nullptr`会调用`void*`的重载版本，因为`nullptr`无法视作任何一种整型

```cpp
f(nullptr);       // 调用f(void*)
```

因此使用`nullptr`而非`0`或`NULL`可以避免重载决议中的意外

---

### 提升代码清晰性

`nullptr`还可以提升代码清晰程度，尤其是在`auto`

```cpp
auto result = findRecord(/*实参*/);

if(result == 0){...}
```

如果不知道`findRecord`的返回值型别，那么`result`是指针还是整形就不清楚了

```cpp
if(result == nullptr){...}
```

这时候就没有多义性了

---

### 在有模板的情况下

---

# 要点速记

- 相对于`0`和`NULL`，优先选用`nullptr`
- 避免在整型和指针型别之间重载
