除了一个奇妙的例外情况，auto型别推导就是模板型别推导

在模板型别推导和auto型别推导之间可以建立起一一映射，他们之间存在双向的算法变换

```cpp
// 函数模板
template<typename T>
void f(ParamType param);

f(expr); // 调用
```

变量采用auto声明时，auto就扮演了T的角色，变量的型别饰辞则扮演ParamType的角色

```cpp

```
