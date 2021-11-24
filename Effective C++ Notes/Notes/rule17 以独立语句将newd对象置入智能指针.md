# 以独立语句将newd对象置入智能指针

假设有个函数用来解释处理程序的优先权，另一个函数用来在某个动态分配取得的Widget上进行某些带有优先权的处理

```cpp
int priority();
void processWidget(std::tr1::shared_ptr<Widget> pw, int priority);
```

由于谨记“以对象管理资源”，processWidget决定对其动态分配而来的Widget运用智能指针

```cpp
processWidget(new Widget, priority());      // 错误
```

以上的调用形式不能通过编译。`tr1::shared_ptr`构造函数需要一个原始指针，但该构造函数是个`explicit`构造函数，无法隐式转换

```cpp
processWidget(std::tr1::shared_ptr<Widget>(new Widget), priority());
```

以上就可以通过编译，但是这个调用可能泄漏资源

---


编译器阐述一个`processWidget`调用码之前，必须首先核算即将被传递的各个实参。上述第二个实参只是一个单纯的对`priority`函数的调用，第一实参`std::tr1::shared_ptr<Widget>(new Widget)`由两部分组成

- 执行 `new Widget`
- 调用`tr1::shared_ptr`构造函数

于是在调用`processWidget`之前，编译器必须创建代码，做以下三件事

- 调用`priority`
- 执行`new Widget`
- 调用`tr1::shared_ptr`构造函数

C++按照怎样的次序完成这些事情呢？弹性很大。可以肯定的是`new Widget`一定在`tr1::shared_ptr`之前执行，`priority`则都可以

假定`priority`第二顺位执行，当他出现异常，会发生什么事？此时`new Widget`返回的指针将遗失，因为他还没被置入智能指针之内

避免这类的问题的办法很简单：使用分离语句

```cpp
std::tr1::shared_ptr<Widget> pw(new Widget);    // 在单独语句哪以智能指针存储newd对象
processWidget(pw, priority());                  // 绝不会泄漏
```

---

# 请记住

- 以独立语句将newd对象置入智能指针内，否则异常被抛出时可能导致难以察觉的资源泄漏
