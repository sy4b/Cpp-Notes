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
auto dereFUPLess=[](const std::unique_ptr<Widget&>& p1, const std::unique_ptr<Widget&>& p2){return *p1<*p2;}
```
