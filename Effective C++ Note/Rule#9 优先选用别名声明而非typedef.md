使用`typedef`可以简化代码，但`typedef`的C++98味太冲，它在C++11可以正常运行，但C++11还提供了别名声明

```cpp
using UPtrMapSS = std::unique_ptr<std::unordered_map<std::string, std::string>>;
```

---

使用别名声明在处理涉及函数指针的型别时比较容易理解

```cpp
typedef void (*FP)(int, const std::string&);

using FP = void (*)(int, const std::string&);
```

---

使用别名声明而不是`typedef`的压倒性理由是存在的，这就是模板

别名声明可以模板化（这种情况下成为别名模板）而`typedef`不可以。它给予了C++11程序员一种直截了当的表达机制，用以表达C++98程序员不得不嵌套在模板化的`struct`里的`typedef`才能硬搞出来的东西

例如要定义一个同义词，表达一个链表，它使用了一个自定义分配器`MyAlloc`

```cpp
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;       // 别名声明

MyAllocList<Widget> lw;                             // 客户代码
```

如果使用`typedef`，则几乎要从头自己动手

```cpp
template<typename T>
struct MyAllocList{
    typedef std::list<T, MyAlloc<T>> type;
}

MyAllocList<Widget>::type lw;                       // 客户代码
```

还有更坏的情况，如果在模板内使用`typedef`创建一个链表，容纳的对象型别由模板形参指定，那么需要给`typedef`的名字加上一个`typename`前缀（作用域解析符默认是名字而不是类别，需要显式指出）

```cpp
template<typename T>
class Widget{
private:
    typename MyAllocList<T>::type list;
};
```

这里`MyAllocList<T>::type`代表一个依赖于模板型别形参`T`的型别，所以`MyAllocList<T>::type`称为带依赖型别，C++规定带依赖型别必须在前面加上`typename`

而如果`MyAllocList`是使用别名模板来定义的，那么写`typename`的要求就消失了

```cpp
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

template<typename T>
class Widget{
private:
    MyAllocList<T> list;
};
```

---

# 要点速记

- `typedef`不支持模板化，但别名声明支持
- 别名模板可以让人免写`::type`后缀，并且在模板内，对于内嵌`typedef`的引用经常要求加上`typename`前缀
