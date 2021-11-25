# 宁以non-member、non-friend函数替换member成员函数

想象有个`class`表示网页浏览器。这样的`class`可能提供的众多函数中，有一些用来清除下载元素高速缓存区、清除访问过的URLs的历史记录、以及移除系统中的所有cookies

```cpp
class WebBrowser{
public:
    ...
    void clearCache();
    void clearHistory();
    void removeCookies();
    ...
};
```

许多用户会想一个整个执行所有这些动作，因此WebBrowser也提供这一个函数

```cpp
class WebBrowser{
public:
    ...
    void clearEverything();     // 调用上述所有函数
    ...
};
```

当然，这一功能也可以用一个non-member函数调用适当的member函数提供出来

```cpp
void clearBrowser(WebBrowser& wb){
    wb.clearCache();
    wb.clearHistory();
    wb.removeCookies();
}
```

这两者哪个更好呢？答案是non-member版本更好

---

从封装开始讨论。如果某些东西被封装，就不再可见。越多东西被封装，越少人可以看到他，也就是说我们有更大的弹性去变化他。

现在考虑对象内的数据，粗略的看，能够访问数据的函数数量越多，封装性就越低。

rule22指出成员变量应该是`private`，如果不是，那么就有无限量的函数可以访问他们，数据毫无封装性。能够访问`private`的只有member函数和`friend`函数，于此相比，non-member函数和non-friend函数具有更大的封装性。这是选择他的一个原因

---

第二件值得注意的事情是：只因为在意封装性而让函数成为`class`的non-member并不意味着它不可以是另一个`class`的member，这对于那些习惯于所有函数都必须定义在`class`里面的语言的使用者而言是个好处。

例如我们可以让`clearBrowser`成为另一个工具类的static member函数。只要她不是`WebBrowser`的一部分，就不会影响它的封装性

在C++中，比较自然的做法是让`clearBrowser`成为一个non-member函数并且位于`WebBrowser`所在的同一个名称空间内

```cpp
namespace WebBrowserStuff{
    class WebBrowser{...};
    void clearBrowser(WebBrowser& wb);
    ...
}
```

这不仅只是为了看起来自然。`namespace`和`class`不同，前者可以跨越多个源码文件而后者不能。这很重要，因为这个函数是个“提供便利的函数”，如果既不是member也不是`friend`，就没法对`WebBrowser`有特殊访问权。例如，如果`clearBrowswe`不存在，客户端就只好自己调用那三个函数

---

一个像`WebBrowser`的`class`可能拥有大量的便利函数，例如某些和书签有关，某些和打印有关，某些与cookie的管理有关...大多数客户只关心其中一种。

合理的组织代码方式就是分别将相关函数声明在单独的头文件，例如

```cpp
// 头文件"webbrowser.h"——针对class WebBrowser自身以及核心功能
namespace WebBrowserStuff{
    class WebBrowser{...};
    ...                     // 核心功能，例如几乎所有客户都需要的non-member函数
}

// 头文件"webbrowserbookmarks.h"
namespace WebBrowserStuff{
    ...                     // 与书签相关的便利函数
}

// 头文件"webbrowsercookies.h"
namespace WebBrowserStuff{
    ...                     // 与cookie相关的便利函数
}

...
```

注意，这正是C++标准程序库的组织方式。

将所有便利函数放在多个头文件内单隶属于同一个命名空间，意味着哭乎可以轻松扩展这一组便利函数。需要做的就是添加更多non-member，non-friend函数到此命名空间内，这是`class`无法提供的功能

---

# 请记住

- 宁可拿non-member、non-friend函数替换member成员函数。这样做可以增加封装性、包裹弹性和技能扩充性
