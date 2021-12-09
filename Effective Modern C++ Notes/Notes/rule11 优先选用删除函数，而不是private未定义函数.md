# 优先选用删除函数，而不是private未定义函数

如果写了代码给其他人用，并且想阻止他们调用某个特定函数，那么只需要不声明这个函数即可。但有时候C++会替你声明函数，此时就没那么简单了

以上的情况只会发生在特种成员函数身上，即C++会在需要时自动生成的成员函数。目前我们只需考虑复制构造函数和复制赋值运算符

C++98为了阻止这些函数被调用，采取的做法是将其声明为`private`，并且不去定义它们。例如C++标准库中输入输出流继承谱系的基类`basic_ios`，为了阻止对输入输出流对象的复制，定义如下：

```cpp
template<class charT, class traits=char_traits<charT>>
class basic_ios: public ios_base{
public:
    ...
private:
    basic_ios(const basic_ios&);            // not defined
    basic_ios& operator=(const basic_ios&); // not defined
};
```

在C++11中，有更好的选择来达成相同的效果：使用`=delete`将复制构造函数和赋值运算符标识为删除函数。以下是C++11中关于`basic_ios`相同规定的一段：

```cpp
template<class charT, class traits=char_traits<charT>>
class basic_ios: public ios_base{
public:
    ...
private:
    basic_ios(const basic_ios&)=delete;
    basic_ios& operator=(const basic_ios&)=delete;
};
```

删除函数无法通过任何手段使用，所以即使是成员和友元函数，也会因为试图复制`basic_ios`型别对象而无法工作，而C++98中这种错误直到链接阶段才会被发现

---

习惯上将删除函数声明为`public`，当客户代码尝试使用某个成员函数时，C++先校验可访问性，后校验删除状态。

---

删除函数的一个重要优点在于，任何函数都能成为删除函数，但只有成员函数能声明为`private`

假定有一个非成员函数，接受一个`int`，返回一个`bool`，为了杜绝隐式转换，需要定义删除函数

```cpp
bool isLucky(int number);

bool isLucky(char)=delete;
bool isLucky(double)=delete;        // 同时也会删除float版本
bool isLucky(bool)=delete;

```

尽管删除函数不可被调用，但还是程序的一部分，因此也会加入重载决议。

---

还有一个妙处是`private`成员函数做不到的，那就是阻止不应该进行的模板具现。假定需要一个和内建指针协作的模板（暂不考虑优先选用智能指针的Rule）

```cpp
template<typename T>
void processPointer(T* ptr);
```

而指针世界有两个异类，一个是`void*`，无法对其提领，自增自减的操作。还有一个是`char*`，它们基本上表示的是C风格的字符串而不是指涉到单个字符的指针。在`processPointer`中我们假设这样的特殊处理手法就是在采用这两个型别时拒绝调用

```cpp
template<>
void processPointer<char>(char* ptr)=delete;

template<>
void processPointer<void>(void* ptr)=delete;
```

而如果是类内部的函数模板，并且想通过`private`声明来禁用某些实现，这是做不到的。

问题在于模板特化必须是在名字空间在作用域而非类作用域内撰写

```cpp
class Widget{
public:
    ...
    template<typename T>
    void process(T* ptr){
        ...
    }
    ...
private:
    template<>
    void process<void>(void*);              // 错误！
};

template<>
void Widget::process<void>(void*)=delete;   // 正确
```

# 要点速记

- 优先选用删除函数而非`private`未定义函数
- 任何函数都可以删除，包括非成员函数和模板具现
