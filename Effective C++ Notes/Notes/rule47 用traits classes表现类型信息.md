# 用traits classes表现类型信息

STL主要由“用来表示容器、迭代器和算法”的`template`构成，但也覆盖了若干工具性templates

---

其中一个名为`advance`，用来将某个迭代器移动给定距离

```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d);
```

观念上`advance`只是做iter+=d操作，但其实不可以全部这么实践。只有随机访问迭代器才支持+=操作；对于其他迭代器种类，必须反复施行`++`或`--`

回顾一下迭代器种类

1. Input迭代器：只能向前移动，一次一步，可读不可写他们指向的东西并且只能读取一次；`istream_iterator`就属于这一分类
2. Output迭代器：类似Input迭代器，但只能输出；`ostream_iterator`就属于这一分类
3. forward迭代器：可以实现上述两者的功能，并且可以多次读写，使得他们可以施行于某些多次性操作算法
4. Bidirectional迭代器：具有上述功能，并且可以双向移动
5. random access迭代器：可以执行“迭代器算数”，在O(1)时间内向前向后任意距离

对于这五种分类，C++标准程序库分别提供专属的卷标结构(tag struct)加以确认

```cpp
struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag: public input_iterator_tag{};
struct bidirectional_iterator_tag: public forward_iterator_tag{};
struct random_accrss_iterator_tag: public bidirectional_iterator_tag{};
```

这些structs之间都是is-a关系

设计advance函数，我们更希望使用random access迭代器

```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d){
    if(.../*iter为random access迭代器*/){
        iter+=d;
    }else{
        if(d>=0){
            while(d--){
                ++iter;
            }else[
                while(d++){
                    --iter;
                }
            ]
        }
    }
}
```

---

**Traits**技术要求之一：对内置类型和用户自定义类型的表现一样好。例如我们自定的advance函数也必须能够作用在内置类型上。

这意味着将信息嵌套在类型之内的做法不行，因为我们无法对内置指针这么做。类型的traits信息必须位于类型自身之外。

标准技术将其放进一个`template`及其一个或多个特化版本中。这样的模板在标准程序库中有多个，其中针对迭代器的模板为`iterator_traits`

```cpp
template<typename IterT>
struct iterator_traits;
```

习惯上将tratis实现为structs

`iterator_traits`的运作方式之一：针对每一个类型IterT，一定声明某个`typedef`名为`iterator_category`，用来确认IterT的迭代器分类

例如`deque`的迭代器可随机访问，所以针对`deque`迭代器设计的`class`看起来如下

```cpp
template<...>       // 这里没有写出模板参数
class deque{
public:
    class iterator{
    public:
        typedef random_access_iterator_tag iterator_category;
        ...
    };
    ...
};
```

`list`的迭代器可以双向进行

```cpp
template<...>       // 这里没有写出模板参数
class list{
public:
    class iterator{
    public:
        typedef bidirectional_iterator_tag iterator_category;
        ...
    };
    ...
};
```

而`iterator_traits`效仿以上做法指出自己是什么

```cpp
template<typename Iter>
struct iterator_traits{
    typedef typename IterT::iterator_category iterator_category;    // typename必须有
    ...
}
```

这对自定义类型有效，但对指针行不通，因为指针无法嵌套`typedef`。`iterator_traits`第二部分则专门用于对付指针

为了支持指针迭代器，`iterator_traits`针对指针类型提供偏特化版本，由于指针也支持`+=`等操作，所以`iterator_traits`为指针指定random access迭代器类型

```cpp
template<typename IterT>
struct iterator_traits<IterT*>{
    typedef random_access_iterator_tag iterator_category;
    ...
};
```

现在你知道如何设计实现一个traits class了

- 确认若干希望将来可以取得的类型相关信息。例如针对迭代器，我们希望可以取得其分类
- 为想取得的信息选择一个名称
- 提供一个模板和一组特例化版本，内含你所希望支持的类型相关信息

---

现在尝试实现advance

```cpp
template<typename IterT, typename DistT>
void advance(IterT & iter, DistT d){
    if(typeid(typename std::iterator_traits<IterT>::iterator_category)==typeid(std::random_access_iterator_tag)){
        ...
    }
    ...
}
```

以上代码有些问题。首先它会导致编译问题，在rule48讨论这一点。

这里更大的问题是：IterT类型在编译期获得，所以iterator_traits<IterT>::iterator_category也是在编译期核定，而if语句在运行期才会核定。将可在编译期完成的事延后到运行期才做，不仅浪费时间而且造成可执行文件膨胀

真正需要的是一条件式用来判断“编译期核定成功”的类型。C++提供重载用来完成这种行为

重载函数根据参数类型完成最佳匹配

```cpp
// random access
template<typename IterT, typename DistT>
void doAdvance(Iter& iter, DistT& d, std::random_access_iterator_tag){
    iter+=d;
}

// bidirectional
template<typename IterT, typename DistT>
void doAdvance(Iter& iter, DistT& d, std::bidirectional_iterator_tag){
    if(d>=0){
        while(d--){
            ++iter;
        }
    }else{
        while(d++){
            --iter;
        }
    }
}

// input
template<typename IterT, typename DistT>
void doAdvance(Iter& iter, DistT& d, std::input_iterator_tag){
    if(d<0){
        throw std::out_of_range("Negative distance");
    }
    while(d--){
        ++iter;
    }
}
```

因为`forward_iterator_tag` `public`继承自`input_iterator_tag`，所以doAdvance的input版本也可以处理forward迭代器

检验d的距离后进入冗长循环呢，可能会带来不明确行为

有了这些重载版本后，advance只需要调用他们，并额外传递一个类型对象即可

```cpp
template<typename IterT, typename DistT>
void advance(IterT & iter, DistT d){
    doAdvance(iter, d, typename std::iterator_traits<IterT>::iterator_category());
    ...
}
```

---

总结如何使用一个traits class：

- 建立一组重载函数或函数模板，彼此差异只在各自的traits参数。令每个函数实现码与其接受的traits信息相符合
- 建立一个控制函数或函数模板，调用杉树重载函数并传递必要信息

标准程序库还提供许多traits class

---

# 请记住

- traits classes使“类型相关信息”在编译期可用，以模板和模板特化完成实现
- 整合重载技术后，traits classes可能可以在编译期对类型执行if else测试