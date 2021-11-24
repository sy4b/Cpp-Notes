# 让接口更容易被正确使用，不容易被误用

C++在接口之海漂浮。`function`接口、`class`接口、`template`接口...每一种接口都是客户与我们的代码互动的手段。

---

想要开发一个“容易被正确使用，不容易被误用”的接口，必须考虑客户可能出现的错误

假设为一个用来表现日期的class设计构造函数

```cpp
class Date{
public:
    Date(int month, int day, int year);
    ...
};
```

乍一看这个接口通情达理，但客户很容易犯下至少两个错误

1. 以错误的次序传递参数
2. 传递一个无效的月份或天数

---

许多客户端错误可以因为导入新类型获得预防

```cpp
struct Day{
    int val;
    explicit Day(int d): val(d){}
};

struct Month{
    int val;
    explicit Month(int m): val(m){}
};

struct Year{
    int val;
    explicit Year(int y): val(y){}
};

class Date{
public:
    Date(const Month& m, const Day& d, donst Year& y);
    ...
};

Date d(30, 3, 1995);                    // 错误，类型不正确
Date d(Day(30), Month(3), Year(1995));  // 错误，类型不正确
Date d(Month(3), Day(30), Year(1995));  // OK
```

---

限制类型的值也是通情达理的，例如一年只有12个有效月份，所以`Month`应该反映这个事实，方法之一是用`enum`表现月份，但不具备类型安全性（例如可以转换为`int`）

```cpp
class Month{
public:
    static Month Jan(){return Month(1);}
    static Month Feb(){return Month(2);}
    ...
private:
    explicit Month(int m);          // 组织生成新的月份
    ...                             // 月份专属数据
};

Date d(Month::Mar(), Day(30), Year(1995));
```

---

预防客户端错误的另一个办法是，限制类型内什么事可以做，什么事不能做。常见的限制是加上`const`

---

另一个一般性准则：除非有好的理由，否则应该让自定义type的行为与内置types一致

---

任何接口然后要求客户必须记得做某些事情，就是有着“不正确使用”的倾向，因为客户很可能就忘记做这件事

例如rule13的一个factory函数，返回一只指针

```cpp
Investment* creatInvestment();
```

为了避免资源泄漏，这个返回的指针必须被删除，这至少开启了两个错误机会：忘记删除、删除多次

较佳的接口先发制人：令factory函数就返回一个智能指针

```cpp
std::tr1::shared_ptr<Investment> createInvestment();
```

实质上强迫客户将返回值存储在一个智能指针那，几乎消除了忘记删除底部Investment对象的可能性。可以阻止一大群客户犯下资源泄漏的错误

---

假设`class`设计者期望“从`createInvestment`取得`Investment*`指针”的客户将该指针传递给一个名为get`RidOfInvestment`的函数，而不是直接使用`delete`，这样的接口又开启了一个错误的大门：企图使用错误的资源析构机制

`createInvestment`的设计者可以针对此问题先发制人：返回一个将`getRidOfInvestment`绑定为删除器的`tr1::shared_ptr`

```cpp
// 试图创建一个null shared_ptr并携带一个自定删除器
std::tr1::shared_ptr<Investment> pInv(0, getRidIfInvesrment);           // 无法通过编译

// 类型转换cast
std::tr1::shared_ptr<Investment> pInv(static_cast<Invesrment*>(0), getRidIfInvesrment);
```

因此如果要实现`createInvestment`使其返回一个`tr1::shared_ptr`并夹带一个`getRidOfInvestment`函数作为删除器，代码看起来像这样：

```cpp
std::tr1::shared_ptr<Investment> createInvestment{
    std::tr1::shared_ptr<Investment> retVal(static_cast<Invesrment*>(0), getRidIfInvesrment);
    retVal=...;         // 令其指向正确对象
    return retVal;
}
```

当然啦，如果`pInv`管理的原始指针可以在建立之前确定下来，就不用初始化为`nullptr`再对它赋值

> 为什么不直接`nullptr`?

---

DLL问题发生在“对象在动态链接程序库（DDL）中被`new`创建，却在另一个DDL那被`delete`”，在许多平台上这类问题会导致运行期错误。

但`tr1::shared_ptr`没有这个问题，它缺省的删除器来自诞生所在的DLL的deleter

---

# 请记住

- 好的接口容易被正确使用，不容易被误用。你应该在所有的借口中努力达成这些性质
- 促进正确使用的方法包括接口的一致性、以及与内置类型的行为兼容
- 阻止误用的办法包括建立新类型、限制类型上的操作、束缚对象值，以及消除客户的资源管理责任
- `tr1::shared_ptr`支持定制删除器，可以防止DLL问题，可被用来自动解除互斥锁等等
