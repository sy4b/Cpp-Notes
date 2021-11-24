# 在资源管理类中提供对原始资源的访问

许多API直接指涉资源

---

[rule13](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule13%20以对象管理资源.md)导入一个观念：使用智能指针保存factory函数的调用结果

```cpp
std::tr1::shared_ptr<Investment> pInv(createInvestment());
```

假设希望用某个函数处理Investment对象：

```cpp
int daysHeld(const Investment* pi);     // 返回投资天数
int days=daysHeld(pInv);                // 错误！
```

以上错误，因为`daysHeld`需要`Investment*`，传递的却是类型`tr1::shared_ptr<Investment>`对象

这种时候需要一个函数将RAII class对象（tr1::shared_ptr）转换为包含的原始资源（Investment*）。可以通过显式转换和隐式转换

`tr1::shared_ptr`和`auto_ptr`都提供一个`get`成员函数，用来执行显式转换，它会返回智能指针内部的原始指针的副本

```cpp
int days=daysHeld(pInv.get());
```

`tr1::shared_ptr`和`auto_ptr`都重载了指针取值操作符`opertaor->`和`opertaor*`，他们允许隐式转换到底部原始指针

```cpp
class Investment{
public:
    bool isTaxFree() const;
    ...
};

Investment* createInvestment;           // factory函数
std::tr1::shared_ptr<Investment> pi1(createInvestment());
bool taxable=!(pi1->isTaxFree());       // operator->
...
stad::auto_ptr<Investment> pi2(createInvestment());
bool taxable2=!(((*pi2).isTaxFree()));  // opertaor*
...
```

---

有些时候还是必须取得RAII对象内的原始资源，某些RAII class设计者提供一个隐式转换函数。考虑一个用于字体的RAII class

```cpp
FontHandle getFont();               // 一个C API，暂时略去参数

void releaseFont(FontHandle fh);    // 同一组C API

class Font{
public:
    explicit Font(FontHandle fh): f(fh){}   // 获得资源，采用pass by value，因为C API这样做
    ~Font(){releaseFont(f);}                // 释放资源
private:
    FontHandle f;                           // 原始字体资源
};
```

假设有大量和字体相关的C API，处理的是`FontHandle`，那么将Font对象转换为FontHandle是一个很频繁的需求，对此Font class可以提供一个显式转换函数，类似`get`

```cpp
class Font{
public:
    ...
    FontHandle get() const{return f;}       // 显式转换
    ...
};
```

这样子的话客户想使用API就必须调用`get`

```cpp
void changeFontSize(FontHandle f, int newSize); // C API
Font f(getFont());
int newFontSize;
...
changeFontSize(f.get(), newFontSize);
```

这个做法令人感到恶心，另一个办法是Font提供隐式转换函数

```cpp
class Font{
public:
    ...
    operator FontHandle() const{
        return f;
    }
    ...
};

Font f(getFont());
int newFontSize;
...
changeFontSize(f, newFontSize);     // 隐式转换
```

这样使得客户调用API更加自然，但是会增加错误发生的机会，例如客户可能会在需要Font时意外创建一个FontHandle

```cpp
Font f1(getFont());
...
FontHandle f2=f1;       // Ah-Oh!原本是要拷贝一个Font对象，却将f1隐式转换为底部的FontHandle，然后才复制它
```

以上程序的FontHandle由Font对象f1管理，但是也可以通过f2取得，这几乎不会有好下场。例如f1被销毁，字体被释放，f2就成为空悬的

---

是否应该提供显式转换或隐式转换，答案主要取决于RAII class被设计执行的特定工作，以及被使用的情况

一般来说RAII返回原始资源的函数，虽然与“封装”有些小矛盾，但谈不上什么设计灾难。RAII class的设计不是为了封装，而是为了确保资源释放成功。

---

# 请记住

- API往往要求访问原始资源，所以每一个RAII class应该提供一个取得所管理的资源的办法
- 对于原始资源的访问可能显式转换或隐式转换，一般显式转换比较安全，隐式转换对客户比较方便
