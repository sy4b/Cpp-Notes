# 优先选用const_iterator而不是iterator

`const_iterator`是STL中相当于指涉到`const`的指针的等价物，他们指涉到不可被修改的值。只要有可能就应该使用`const`的标准实践表明，任何时候只要你需要一个迭代器而指涉到的内容没有修改必要，就应该使用`const_iterator`

---

C++98中对于`const_iterator`的支持并不全面，使用起来很麻烦，而在C++11中得到了改观。容器的`cbegin`和`cend`函数都返回`const_iterator`性别，对于非`const`容器也是如此。并且STL成员函数如果要取用指示位置的迭代器，必须使用`const_iterator`型别

```cpp
std::vector<int> values;
...
auto it=std::find(values.cbegin(), values.end(), 1983);
values.insert(it, 1998);
```

只有一种情况下，C++11对于`const_iterator`的支持显得不够充分，那是当你想要撰写最通用的库代码的时候，这些代码会考虑到某些容器或数据结构是否会以非成员函数的形式提供`begin`和`end` `cbegin`等等，而不是以成员函数的方式。

上面的例子可以写成通用的模板形式

```cpp
// C++11不行，C++14可以
template<typename C, typename V>
void findAndInsert(C& container, const V& targetVal, const V& insertVal){
    using std::cbegin;
    using std::cend;
    auto it=std::find(cbegin(container), cend(container), targetVal);
    container.insert(it, insertVal);
}
```

以上的代码在C++11中无法运行，因为C++11只添加了非成员函数版本的`begin`和`end`，C++14把`rbegin`等补上了

如果是C++11，想要使用上面缺失的非成员函数模板，我们可以自己实现

```cpp
template<typename C>
auto cbegin(const C& container)->decltype(std::begin(container)){
    return std::begin(container);
}
```

---

# 要点速记

- 优先选用`const_iterator`
- 在最通用的代码中，优先选用非成员函数版本的`begin`, `end`, `rbegin`等