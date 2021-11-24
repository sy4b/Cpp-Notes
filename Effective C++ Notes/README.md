# Effective C++ Notes

---
## Chapter 3 资源管理

所谓资源就是，一旦使用了它，将来必须还给系统，否则会发生糟糕的事情

C++程序最常使用的资源就是动态分配内存，但内存只是必须管理的众多资源之一，其他常见的还有文件描述器、互斥锁、图形界面中的字形和笔刷、数据库连接、以及网络sockets。不论哪一种资源，当不再使用时，都必须还给系统

本章一开始是直接易懂的，基于对象的资源管理办法，建立在C++对构造函数】析构函数和`copying`函数的基础上

[rule13 用对象管理资源](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule13%20以对象管理资源.md)

[rule14 小心copying行为](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Notes/Notes/rule14%20在资源管理类中小心copying行为.md)
---
