# 别让异常逃离析构函数

C++并不禁止析构函数抛出异常，但不鼓励这么做，是有理由的

---

```cpp
class Widget{
public:
    ...
    ~Widget(){...}      // 假设它抛出一个异常
};
void doSomething(){
    std::vector<Widget> v;
    ...
}                       // v在这里被自动销毁
```

当v被销毁时，它也负责销毁内含的所有Widget。假设v内含10个Widget，在析构第一个元素期间，有一个异常被抛出，其他九个还是得析构，不然会内存泄漏。若第二个异常也抛出。异常的数量太多了，程序不是执行停止就是导致不明确行为。

使用标准程序库的任何容器或TRI的任何容器，都会导致不明确行为

---

但若析构函数必须执行一个动作，该动作可能会在失败时抛出异常怎么办呢

假设使用一个`class`负责数据库连接

```cpp
class DBConnection{
public:
    ...
    static DBConnection create();
    void close();                       // 关闭联机：失败则抛出异常
};
```

为了确保客户不忘记在`DBConnection`对象身上调用`close()`，合理的想法是创建一个用来管理D`BConnection`资源的`class`，在其析构函数中调用`close`

```cpp
class DBConn{
public:
    ...
    ~DBConn(){
        db.close();
    }
private:
    DBConnection db;
};
```

这样便允许客户写出这样的代码

```cpp
{                                       // 开启一个区块
    DBConn dbc(DBConnection::create()); // 建立DBConnection对象并交给DBConn对象管理
                                        // 使用DBConnection对象

    ...

}                                       // 区块结束时DBConn对象被销毁，因而自动调用close
```

调用`close()`成功的话，一切都很美好，但如果该调用导致异常，`DBConn`析构函数会传播该异常，也就是允许它离开这个析构函数，会导致难以驾驭的麻烦

有两个办法可以避免这个问题：

1. `DBConn`的析构函数：如果`close`抛出异常就结束程序，通常通过调用`abort`完成

```cpp
DBConn::~DBConn(){
    try{
        db.close();
    }catch(...){
        std::abort();
    }
}
```

2. 吞下因`close`发生的异常

```cpp
DBConn::~DBConn(){
    try{
        db.close();
    }catch(...){

    }
}
```

这两种方法都没什么吸引力，无法对导致`close`抛出异常的情况作出反应

---

一个较好的策略是重新设计`DBConn`接口，使其客户有机会对可能出现的问题做出反应

```cpp
class DBConn{
public:
    ...
    void close(){               // 提供给客户使用的新函数
        db.close();             // 客户自定义close
        closed=true;
    }
    ~DBConn(){
        if(!closed){
            try{
                db.close();
            }catch(...){
                ...             // 异常时结束程序或者吞下一场
            }
        }
    }
private:
    DBConnection db;
    bool closed;
};
```

---

# 请记住

- 析构函数绝对不要吐出一场
- 如果客户需要对某个操作函数运行期间抛出的异常作出反应，那么`class`一个提供i个普通函数（而不是在析构函数中）执行该操作
