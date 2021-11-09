除了一个奇妙的例外情况，`auto`型别推导就是模板型别推导

在模板型别推导和`auto`型别推导之间可以建立起一一映射，他们之间存在双向的算法变换

```cpp
// 函数模板
template<typename T>
void f(ParamType param);

f(expr); // 调用
```

变量采用`auto`声明时，`auto`就扮演了`T`的角色，变量的型别饰辞则扮演`ParamType`的角色

```cpp
auto x=27;  // x的型别饰辞就是auto本身

const auto cx=x;  // cx的型别饰辞为const auto

const auto& rx=x; // rx的型别饰辞为const auto&
```

- 情况一：型别饰辞是指针或引用，但不是万能引用
- 情况二：型别饰辞是万能引用
- 情况三：型别饰辞既非指针也非引用
