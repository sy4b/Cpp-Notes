# 宁可以pass-by-reference-to-const替换pass-by-value

在缺省的情况下，C++以by value方式传递对象到函数。除非另外指定，否则函数参数都是实参的副本为初始值，调用端口得到的也是函数返回值的一个副本。这些副本从对象的copy构造函数产出，可能使得pass-by-value成为昂贵的操作

---

考虑以下`class`继承体系

```cpp
class Person{
public:
    Person();               // 简化，省略参数
    virtual ~Person();      // rule7 指出为什么是virtual
    ...
private:
    std::string name;
    std::string address;
};

class Student: public Person{
public:
    Student();
    ~Student();
    ...
private:
    std::string schoolName;
    std::string schoolAddress;
};
```

考虑以下代码

```cpp
bool validateStudent(Student s);
Student plato;
bool platoIsOK=validateStudent(plato);
```

上述函数被调用，会产生多少开销？

首先Student的copy构造函数被调用，valid返回后s会销毁

此外Student对象内有两个string对象成员，基类又有两个string对象成员...开销巨大

---

使用pass by reference-to-const就可以完美规避所有构造和析构

```cpp
bool validateStudent(const Student& s);
```

这种传递方式效率高得多：没有任何构造函数或析构函数被调用。修订后的参数声明中的`const`很重要，不这样子哦的话，调用者会担心函数是否会改变传入的对象

---

by reference方式传递参数也可以避免对象切割问题。

当一个derived class对象用by value方式传递并被视为一个base class对象，base class的copy构造函数会被调用，而其他派生的特点都被切割掉了，这几乎不会是你想要的。

考虑一个图形窗口系统

```cpp
class Window{
public:
    ...
    std::string name() const;           // 返回窗口名称
    virtual void display() const;       // 显示窗口内容
};

class WindowWithScrollBars: public Window{
public:
    ...
    virtual void display() const;       // 比较华丽丽的显示
};
```

现在希望写一个函数打印窗口名称，然后显示该窗口

```cpp
void printNameAndDisplay(Window w){     // 不正确，参数可能被切割
    std::cout<<w.name();
    w.display();
}

WindowWithScrollBars wwsb;
printNameAndDisplay(wwsb);
```

以上的调用使得无论传递的对象是什么类型，都会被认为是Window类型

解决对象切割问题的方法，就是by reference-to-const方式传递w

```cpp
void printNameAndDisplay(const Window& w){
    std::cout<<w.name();
    w.display();
}
```

现在传进来的窗口是什么类型，w就表现出哪种类型

---

C++编译器的底层中，reference往往以指针表现出来，因此实际上传递的是指针

如果对象属于内置类型，pass by value往往效率更高，有时候也可以使用

---

# 请记住

- 尽量用pass-by-reference-to-const替换pass-by-value。前者通常更高效，并可以避免切割问题
- 以上规则不适用于内置类型以及STL的迭代器和函数对象，pass-by-value往往更适合他们
