假定一个函数接受一个`Widget`并返回一个`std::vector<bool>`，第五个元素指出`Widget`是否有更高优先级

```cpp
Widget w;
...
bool highPriority = features(w)[5];
...
  
processWidget(w, highPriority);       // 按照优先级处理w
```

上述代码没什么问题，但如果从显式改为`auto`，情况便急转而下

```cpp
auto highPriority = feature(w)[5];    // w具有高优先级吗？

processWidget(w, highPriority);       // 未定义行为
```

在使用了`auto`后，`highPriority`的类型就不再是`bool`，`std::vector<bool>`的`operator[]`返回值不是容器内元素的引用，而是`std::vector<bool>::reference`，它是嵌套在`std::vector<bool>`里的类

之所以弄出一个`std::vector<bool>::reference`，是因为`std::vector<int>`做过特化，用一种压缩形式表示持有的`bool`元素，每个`bool`元素用一个比特来表示

一般来说`vector<T>::operator[]`返回`T&`，但C++禁止比特的引用，因此对`std::vector<bool>::reference`做一个向`bool`的隐式转换

