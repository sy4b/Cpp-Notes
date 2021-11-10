# 优先选用auto，而非显式型别声明

用`auto`声明的变量，类型都从初始值推导而来，因此必须被初始化，这也意味着，旧标准的一系列由于未初始化变量带来的问题都不存在

```cpp
int x1;         // 有潜在的未初始化风险

auto x2;        // 错误，auto未初始化

auto x3=27;     // 正确
```

再例如

```cpp
template<typename It>
void dwiw(It b, It e){  //  迭代器b，e范围内
  while(b!=e){
    typename std::iterator_traits<It>::value_type currValue=*b;
    ...
  }
}

// 可以用auto避免初始化的坑
template<typename It>
void dwiw(It b, It e){  //  迭代器b，e范围内
  while(b!=e){
    auto currValue=*b;
    ...
  }
}

```

并且由于`auto`使用了型别推导，就可以用它表示只有编译器掌握的型别

```cpp
auto dereFUPLess=[](const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2){return *p1<*p2;}

// C++14中，连形参都可以使用auto
auto derefLess=[](const auto& p1, const auto& p2){return *p1<*p2;};
```

也许会感觉并不需要声明变量来持有闭包，因为可以使用`std::function`对象来完成这件事

---

- 补充：什么是std::function对象

`std::function`是C++11中的一个标准库模板，它将函数指针的思想加以推广，可以指涉任何可调用对象，需要指定指涉的函数的型别

---

使用`std::function`来定义上述的闭包`lambda`

```cpp
std::function<bool (const std::unique_ptr<Widget>&, const std::unique_ptr<Widget>&)> derefUPLess=[](constconst std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2){return *p1<*p2;}
```

这样带来诸多不便

- 词法啰嗦，重复很多
- `std::function`是一个模板，必然带来额外的内存开销
- 编译器限制内联，产生间接函数调用，运行慢

---

`auto`可以避免”型别捷径“的问题，例如

```cpp
std::vector<int> v;
...
unsigned sz=v.size();       // warning

auto sz1=v.size();          // 推荐
```

标准规定`v.size()`返回`std::vector<int>::size_type`，这是一个无符号整型，但在不同系统（32win和64win）上可能不一样。

使用`auto`就可以避免这个问题

---

对于下面这段代码

```cpp

//
暗藏隐患
std::unordered_map<std::string, int> m;

for(const std::pair<std::string, int>& p: m){
  ...
}
```

`std::unordered_map`的键值部分是`const`，所以哈希表中的元素`p`的类型应该是`std::pair<const std::string, int>`，编译器需要进行类型转换，方法就是每个对象都进行复制出临时对象，在循环结束时都会被析构。

这样的错误可以用`auto`轻松化解，不仅少打字，运行效率也大幅提升

---

# 要点速记

- auto变量必须初始化，基本上会对导致兼容性和效率问题的型别不匹配现象免疫
