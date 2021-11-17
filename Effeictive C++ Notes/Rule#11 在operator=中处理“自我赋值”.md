# 在operator=中处理“自我赋值”

自我复制发生在对象被赋值给自己时：

```cpp
class Widget{...};
Widget w;
...
w=w;                // 自我赋值
```

这看起来有点蠢，但它合法，所以不要认定客户不会那么做

---

有时候自我赋值不那么明显呢，例如

```cpp
a[i]=a[j];          // 潜在的自我赋值
*px=*py;            // 潜在的自我赋值
```

两个对象如果来自同一个继承体系，它们甚至不需要声明为相同类型就可能造成”别名“（所谓“别名”就是有多个方法指涉同一个对象），因为一个base class的reference或pointer可以指向一个derived class对象

```cpp
class Base{...};
class Derived: public Base{...};
void doSomething(const Base& rb, Derived* pd);  // rb和*pd有可能其实是同一个对象
```

---

如果尝试自己管理资源，可能会掉进”在停止使用资源之前意外释放了它“的陷阱

假设建立一个`class`用来保存一个指针，指向一块动态分配的位图

```cpp
class Bitmap{...};
class Widget{
    ...
private:
    Bitmap* pb;         // 指针，指向一个从heap分配得到的对象
};
```

下面是`operator=`的实现代码，表面上看起来合理，但自我赋值时并不安全

```cpp
Widget& Widget::operator=(const Widget& rhs){
    delete pb;
    pb=new Bitmap(*rhs.pb);
    return *this;
}
```

这里存在一个问题，`opertaor=`函数内的`*this`和`rhs`可能是同一个对象。那么`delete`后`rhs`的资源也被销毁了

要阻止这种错误，传统做法是在`opterator=`最前面添加一个测试

```cpp
Widget& Widget::operator=(const Widget& rhs){
    if(this==&rhs){
        return *this;           // 如果是自我赋值，就不做任何事情
    }
    delete pb;
    pb=new Bitmap(rhs.pb);
    return *this;
}
```

这种做法具备自我赋值安全性，但不具备异常安全性。new的操作可能会因为内存不足或者Bitmap的copy构造函数抛出异常，导致指针有害

让`operator=`获得异常安全性的操作汪汪也具有自我赋值安全性。[Rule29]()深度探讨了异常安全性

```cpp
Widget& Widget::operator=(const Widget& rhs){
    Bitmap* pOrig = pb;         // 记住原先的pb
    pb=new Bitmap(*rhs.pb);     // 令pb指向*pb的一个副本
    delete pOrig;               // 删除原先的pb
    return *this;
}
```

现在，如若new操作抛出异常，pb保持原状，并且即使没有测试，也会具有自我赋值安全性。这或许不是最高效的办法，但它行得通

如果很关心效率，也可以将测试放入函数起始处，但需要估计自我赋值发生的概率有多高，否则可能反而增加成本

---

在[Rule29]()中讲述了一种技术，确保代码不但异常安全，而且自我赋值安全，如下

```cpp
class Widget{
    ...
    void swap(Widget& rhs);
};

Widget& Widget::operator=(const Widget& rhs){
    Widget temp(rhs);       // 做出一份rhs的副本
    swap(temp);             // 交换*this和这份副本
    return *this;
}                           // 副本被析构，即rhs的资源被销毁
```

---

# 请记住

- 确保当对象自我赋值时，`operator=`有良好的行为
- 确保任何函数如果操作一个以上的对象，而其中多个对象是同一个对象时，其行为仍然正确
