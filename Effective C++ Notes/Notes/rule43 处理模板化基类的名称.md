# 处理模板化基类的名称

假设写一个程序，传递信息到若干不同的公司，信息要么译成密码，要么就是未加工的文字。如果编译期间我们有足够的信息来决定哪一个信息传递到哪一家公司，就可以采用基于`template`的解法

```cpp
class CompanyA{
public:
    ...
    void sendCleartext(const std::string& msg);
    void sendEncrypted(const std::string& msg);
    ...
};

class CompanyB{
public:
    ...
    void sendCleartext(const std::string& msg);
    void sendEncrypted(const std::string& msg);
    ...
};
...         // 其他company

class MsgInfo{...};     // 保存信息，产生信息

template<typename Company>
class MagSender{
public:
    ...
    void sendClear(const MsgInfo& info){
        std::string msg;
        ... // 根据info产生信息
        Company c;
        c.sendCleartext(msg);
    }
    void sendSecret(const MsgInfo& info){...}   // 类似sendClear
};
```

以上做法行得通。假设我们有时候要在每次送出信息的时候记录某些信息，可以通过继承加上这样的生产力

```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{
public:
    ...
    void sendClearMsg(const MsgInfo& info){
        ... // 传送前的信息写到log
        sendClear(info);
        ... // 传送后的信息写到log
    }
    ...
};
```

这里derived class的信息传递函数有一个不同的名称，是一个好设计，避免了遮掩继承得到的名称，也避免了重新定义继承而来的Non-virtual函数

但上述代码无法通过编译，编译器看不到sendClear函数。why？

这里编译器看到class template LoggingMsgSender的定义式时，并不知道它继承怎样的`class`。当然他继承MsgSender<Company>，但其中包含一个模板参数，不到被具现化的时候无法得知他确切是什么，也就不能得知他是否有一个sendClear函数

为了具体化问题，考虑一个class CompanyZ坚持使用加密通讯

```cpp
class CompanyZ{
    // 不提供sendCleartext函数
public:
    ...
    void sendEncrypted(const std::string& msg);
    ...
};
```

一般性的MsgSender template不适用于CompanyZ，因为那个模板提供了sendClear函数。我们可以针对CompanyZ产生一个MsgSender特化版

```cpp
template<>      // 模板特例化
class MsgSender<CompanyZ>{
public:
    ...
    void sendSecret(const MsgInfo& info){...}
};
```

现在再次考虑derived class LoggingMsgSender

```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{
public:
    ...
    void sendClearMsg(const MsgInfo& info){
        // 传送前的信息写到log
        sendClear(info);        // 若Company==CompanyZ，该函数不存在
        // 传送后的信息写到log
    }
}
```

正如注释所言，base class被指定为MsgSender<CompanyZ>时这段代码就不合法，因为这个`class`未提供sendClear函数

C++知道base class templates有可能被特例化，而特例化版本可能不提供和一般性`template`一样的接口，因此往往拒绝在模板化基类中寻找继承而来的名称！

---

有三个办法令上述行为失效

1. 在base class函数调用动作之前加上`this->`

```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{
public:
    ...
    void sendClearMsg{
        //
        this->sendClear(info);  // 成立，假设sendClear将被继承
        //
    }
    ...
};
```

2. 使用using声明式

```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{
public:
    using MsgSender<Company>::sendClear;
    ...
    void sendClearMsg{
        //
        sendClear(info);  // 成立，假设sendClear将被继承
        //
    }
    ...
};
```

3. 明确指出被调用的函数位于base class内。这种方法不太好，如果调用`virtual`函数，那么这种修饰会关闭`virtual`绑定行为

```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{
public:
    ...
    void sendClearMsg{
        //
        MsgSender<Company>::sendClear(info);  // 成立，假设sendClear将被继承
        //
    }
    ...
};
```

---

# 请记住

- 可在derived class templates内通过`this->`指涉base class templates内的成员名称，活借由一个base class资格修饰符使得模板化基类的名称可见