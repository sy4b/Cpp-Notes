# 在资源管理类中小心copying行为

[rule13](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C++%20Notes/Notes/rule13%20以对象管理资源.md)指出资源取得时机便是初始化时机，也描述了`auto_ptr`和`tr1::shared_ptr`如何将这个观念表现在heap-based资源上

然而并非所有资源都是heap-based，对于那些资源，智能指针汪汪不适合作为资源掌管者。这时候需要自己建立新的资源管理类

例如使用C API函数处理类型为`Mutex`的互斥器对象，有`lock`和`unlock`两个函数

```cpp
void lock(Mutex* pm);       // 锁定pm所指的互斥器
void unlock(Mutex* pm);     // 将互斥器解除锁定
```

为了确保不会将一个被锁住的`Mutex`解锁，可能会希望建立一个`class`来管理，这样的`class`的基本结构被RAII守则支配，也就是资源在构造期间获得，在析构期间释放

```cpp
class Lock{
public:
    explicit Lock(Mutex* pm): mutexPtr(pm){     // 获得资源
        lock(mutexPtr);
    }
    ~Lock(){                                    // 释放资源
        unlock(mutexPtr);
    }
private:
    Mutex* mutexPtr;
};
```

客户对`Lock`的用法符合RAII方式

```cpp
Mutex m;                // 互斥器
...
{                       // 建立一个区块来定义critical section
    Lock m1(&m);        // 锁定互斥器件
    ...                 // 执行操作
}                       // 在区块最末尾，自动解除互斥器锁定
```

以上很好，但如果`Lock`对象被复制，会发生什么事？

```cpp
Lock ml1(&m);           // 锁定m
Lock ml2(ml1);          // 将ml1复制到ml2，会如何？
```

这是一个一般化问题的特定例子。一般化问题是每一位RAII class坐着一定要面对的：当一个RAII对象被复制，会发生什么事？

大多数时候会有以下选择：

1. 禁止复制

许多时候允许RAII对象被复制并不合理。可以将`copying`操作声明为`delete`

2. 对底层资源使用引用计数法

有时候希望保有资源，直到他的最后一个使用者（某个对象）被销毁。这种情况下复制RAII对象时，应该将其“被引用数”递增，`tr1::shared_ptr`便是如此

通常只需要内含一个`tr1::shared_ptr`成员变量，便可以实现上述功能。如果`Lock`打算使用，可以将`Mutex*`改为`tr1::shared_ptr<Mutex>`，但是`tr1::shared_ptr`缺省的行为是引用次数为0时删除所指物，我们指向解锁而不是删除

幸运的事`tr1::shared_ptr`允许指定所谓的删除器deleter。删除器是一个函数或函数对象，当引用次数为0时便被调用。删除器对于`tr1::shared_ptr`构造函数是可有可无的第二参数

```cpp
class Lock{
public:
    explicit Lock(Mutex* pm): mutexPtr(pm, unlock){
        lock(numtexPtr.get());
    }
private:
    std::tr1::shared_ptr<Mutex> mutexPtr;
};
```

3. 复制底部资源

只要你想，可以针对一份资源拥有其任意数量的副本，需要资源管理类的理由是：当不需要某个副本时，保证他被释放

4. 转移底部资源的拥有权

类似`auto_ptr`的行为

---

# 请记住

- 复制RAII对象必须一并复制所管理的资源，资源的copying行为决定RAII对象的copying行为
- 普遍的RAII class copying行为是：抑制copying，使用引用计数法
