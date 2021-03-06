# 设计class犹如设计type

C++中定义一个新`class`，就定义了一个新type。身为一个C++程序员，许多时间主要用来扩张你的类型系统，意味着不只是`class`设计者，还是type设计者。

重载函数和操作符、控制内存的分配和归还、定义对象的初始化和终结...全都掌握在你手上

设计优秀的`class`是一项艰巨的工作，首先必须了解面对的问题。几乎每一个`class`都要求你面对以下提问：

1. 新type对象应该如何被创建和销毁？

这会影响到构造函数、析构函数以及内存分配和释放函数（`operator new`、`operator new[]`、`operator delete`和`operator delete[]`）

2. 对象的初始值和对象的赋值应该有什么差别？

这个答案决定了构造函数和赋值操作符的行为及其之间的差异。重要的是不要混淆“初始化”和“赋值”，她们对应不同的函数调用

3. 新type的对象如果passed by value意味着什么？

记住，copy构造函数用来定义一个type的pass-by-value怎么实现

4. 什么是新type的合法值？

对于`class`的成员变量，通常只有某些数值集是有效的，这决定了你的`class`必须维护的约束条件，也就决定了成员函数必须进行的错误检查工作。也影响函数抛出的异常

5. 你的新type需要配合某个继承体系嘛？

如果是继承自某些既有的classes，就会受到他们设计的束缚，尤其是`virtual`与否的影响

6. 你的新type需要怎样的转换？

如果你希望隐式转换，就需要写一个类型转换函数`operator T2`。

如果你只允许`explicit`构造函数存在，就得写出专门负责执行转换的函数，且不得为类型转换操作符构造函数

7. 什么样的操作符和函数对此新type是合理的？

这个问题决定你为`class`声明哪些函数，其中哪些是成员与否

8. 什么养的标准函数应该驳回？

那些需要你声明为删除的函数

9. 谁该使用新type的成员？

这决定你让哪个成员为`public`、`private`或`protected`。也帮助你决定哪一个类和函数是否`friend`，以及如何嵌套

10. 什么是新type的未声明接口？

他对效率、异常安全性以及资源运用提供何种保证？这些方面能够为实现代码加上相应的约束条件

11. 新的type有多么一般化？

或许你应该定义一个新的class template

12. 你真的需要一个新type吗

如果只是定义新的derived class以便为现在有的class添加技能，说不定单纯定义几个non-member函数或者templates更加好

---

# 请记住

- class的设计就是type的设计。在定义一个新的type前，请确定是否考虑本rule的所有讨论主题
