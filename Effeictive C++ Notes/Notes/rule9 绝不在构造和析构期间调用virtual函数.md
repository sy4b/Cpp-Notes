# 绝不在构造和析构过程中调用virtual函数

绝不要在构造函数和析构函数期间调用`virtual`函数，这样的调用不会带来预想的结果，就算有你也不会高兴

C++在这点上和Java有所不同

---

假设有一个`class`继承体系，用来模拟交易订单，这样的交易要经过审计，所以每创建一个交易对象，在审计日志中也需要创建一笔适当记录

以下是一个看起来颇为合理的做法

```cpp
class Transaction{                          // 所有交易的base class
public:
    Transaction();
    virtual void logTransaction() const=0;  // 做出一份因为交易类型而不同的日志记录
    ...
};

Transaction::Transaction(){
    ...
    logTransaction();                       // 记录这笔交易
}

class BuyTransaction: public Transaction{
public:
    virtual void logTransaction() const;
    ...
};

class SellTransaction: public Transaction{
public: 
    virtual void logTransaction() const;
    ...
};
```

此时执行以下操作会发哼什么事情？

```cpp
BuyTransaction b;
```

会有一个`BuyTransaction`构造函数被调用，要调用这个函数之前，首先必须调用`Transaction`构造函数，先构造base class成分。而`Transaction`构造函数最后一行调用`virtual`函数`logTransaction`，此时构造的版本是`Transaction`版本，即使目前建立的对象类型是`BuyTransaction`

在base class构造期间，`virtual`函数绝不会下降到`derived`阶层，取而代之的是，对象的行为就像隶属base类型一样。

在base class构造期间，`virtual`函数不是`virtual`函数

---

更加根本的原理是

在derived class的base class构造期间，对象的类型是base class而不是derived class

不仅仅`virtual`函数会被编译器解析为base class，如果使用运行期间类型信息（例如`dynamic_cast`和`typeid`），也会把对象视作base class类型

---

相同的道理也适用于析构函数，一旦derived class析构函数开始执行，对象内的derived class成员变量便呈现未定义值，C++视作不存在。进入base class析构函数后对象就成为一个base class对象

---

侦测构造函数或析构函数运行期间是否调用`virtual`函数并不总是轻松。

如果`Transaction`有多个构造函数，每个都需要执行某些相同工作，那么避免代码重复的一个做法是将共同的初始化代码放进一个初始化函数中

```cpp
class Transaction{
public:
    Transaction(){init();}                  // 调用non-virtual
    virtual void logTransaction() const=0;
    ...
private:
    void init(){
        ...
        logTansaction();                    // 调用了virtual
    }
};
```

这段代码一般不会引起编译器和连接器的警告，但这种做法是错误的

---

其他方案可以解决这个问题，一种做法是在class Transaction内将`logTransaction`函数改为non-virtual，而后要求derived class构造函数传递必要信息给`Transcation`构造函数，而后便可以安全调用non-virtual logTransaction

```cpp
class Transaction{
public:
    explicit Transaction(const std::string& logInfo);
    void logTansaction(const std::string& longInfo) const;
    ...
};

Transaction::Transaction(const std::string& logInfo){
    ...
    logTransaction(logInfo);
}

class BuyTransaction: public Transaction{
public:
    BuyTransaction(parameters): Transaction(createLogString(parameters)){...}
    ...
private:
    static std::string createLogString(parameters){...};
};
```

---

# 请记住

- 在析构和构造函数期间不要调用`virtual`函数，因为这类调用从不下降至derived class  
