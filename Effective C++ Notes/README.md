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

---
