# Effective C++ Notes

---
## Chapter 3 资源管理

所谓资源就是，一旦使用了它，将来必须还给系统，否则会发生糟糕的事情

C++程序最常使用的资源就是动态分配内存，但内存只是必须管理的众多资源之一，其他常见的还有文件描述器、互斥锁、图形界面中的字形和笔刷、数据库连接、以及网络sockets。不论哪一种资源，当不再使用时，都必须还给系统

本章一开始是直接易懂的，基于对象的资源管理办法，建立在C++对构造函数】析构函数和`copying`函数的基础上

[rule13 用对象管理资源](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule13%20以对象管理资源.md)

[rule14 小心copying行为](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule14%20在资源管理类中小心copying行为.md)

[rule15 在资源管理类中提供访问原始资源的方法](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule15%20在资源管理类中提供对原始资源的访问.md)

[rule16 配对使用new和delete](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule16%20成对使用new和delete要采取相同形式.md)

[rule17 以独立语句将newd对象置入智能指针](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule17%20以独立语句将newd对象置入智能指针.md)

---

## Chapter 4 设计与声明

所谓软件设计，就是“令软件做出你希望他做出的事情”的步骤和做法，通常从一般性的构想开始，最后演变成十足的细节，允许特殊接口的开发。这些借口而后必须转换为C++声明式。

本章探讨：让接口更容易被正确使用，不容易被误用。

[rule18 让接口更容易被正确使用，不容易被误用](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule18%20让接口更容易被正确使用，不容易被误用.md)

[rule19 设计class犹如设计type](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule19%20设计class犹如设计type.md)

[rule20 宁可以pass-by-reference-to-const替换pass-by-value](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule20%20宁可以pass-by-reference-to-const替换pass-by-value.md)

[rule21 必须返回对象时，不要妄想返回reference](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule21%20必须返回对象时，别妄想返回其reference.md)

[rule22 将成员变量声明为private](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule22%20将成员变量声明为private.md)

[rule23 宁以non-member、non-friend函数替换member成员函数](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule23%20宁以non-member、non-friend函数替换member成员函数.md)

[rule24 若所有参数都需要类型转换，请为此使用non-member函数](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule24%20若所有参数都需要类型转换，请为此使用non-member函数.md)

[rule25 考虑写出一个不抛出异常的swap函数](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule25%20考虑写出一个不抛出异常的swap函数.md)

---

## Chapter 5 实现

大多数情况下，适当提出你的`class`和class template定义，以及function声明，是花费最多心力的两件事。但在相应的实现上还要小心。

太快定义变量可能导致效率上的拖延；过度使用转型可能导致代码又慢又难维护，导致微妙难解的错误...

[rule26 尽量延后变量定义式的出现时间](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule26%20尽量延后变量定义式的出现时间.md)

[rule27 尽量少做转型动作](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule27%20尽量少做转型动作.md)

[rule28 避免返回handles指向对象内部成分](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule28%20避免返回handles指向对象内部成分.md)

[rule29 为异常安全努力是值得的](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule29%20为异常安全努力是值得的.md)

[rule30 透彻了解inlining的里里外外](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule30%20透彻了解inlining的里里外外.md)

[rule31 将文件之间的编译依存关系降到最低](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule31%20将文件之间的编译依存关系降到最低.md)

---

## Chapter 6 继承与面向对象设计

C++的OOP包含很多东西：继承、链接，成员函数的选项等等

本章对这些题目全面宣战

[rule32 确定你的public继承塑造出is-a关系](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule32%20确定你的public继承塑造出is-a关系.md)

[rule33 避免遮掩继承而来的名称](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule33%20避免遮掩继承而来的名称.md)

[rule34 区分接口继承和实现继承](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule34%20区分接口继承和实现继承.md)

[rule35 考虑virtual函数之外的选择](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule35%20考虑virtual函数以外的选择.md)

[rule36 绝不重定义继承而来的non-virtual函数](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule36%20绝不重定义继承而来的non-virtual函数.md)

[rule37 绝不重定义继承而来的缺省参数初始值](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule37%20绝不修改继承而来的函数参数缺省值.md)

[rule38 通过复合塑模实现出"has-a"或“根据某物实现出”](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule38%20通过复合塑模实现出%22has-a%22或“根据某物实现出”.md)

[rule39 明智而慎重使用private继承](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule39%20明智而慎重使用private继承.md)

[rule40 明智而慎重使用多重继承](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule40%20明智而慎重使用多重继承.md)

---

## Chapter 7 模板及泛型编程

[rule41 了解隐式接口和编译期多态](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule41%20了解隐式接口和编译期多态.md)

[rule42 typename的双重含义](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule42%20了解typename的双重含义.md)

[rule43 处理模板化基类中的名称](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule43%20处理模板化基类的名称.md)

[rule44 将与参数无关的代码抽离templates](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule44%20将与参数无关的代码抽离templates.md)

[rule45 运用成员函数模板接受所有兼容类型](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule45%20运用成员函数模板接受所有兼容类型.md)

[rule46 需要类型转换时为模板定义非成员函数](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule46%20需要类型转换时为模板定义非成员函数.md)

[rule47 用traits classes表现类型信息](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule47%20用traits%20classes表现类型信息.md)

[rule48 认识模板元编程](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule48%20认识模板元编程.md)

---

## Chapter 8 定制new和delete

