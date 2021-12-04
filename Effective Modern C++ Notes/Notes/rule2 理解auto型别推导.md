# 理解auto型别推导

---

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

以上三种情况都适用模板型别推导的规则，包括数组和函数在非引用型别饰辞下退化为指针的情况

但有一处不同。若要声明一个`int`并初始化为27，有以下四种语法

```cpp
// C++98
int x1=27;
int x2(27);
// C++11
int x3{27};
int x4={27};
```

采用`auto`声明变量相比采用固定型别声明变量更有优势，此时！

```cpp
auto x1=27; // 型别是int，值是27
auto x2(27); // 同上
auto x4={27}; // 型别是std::initializer_list<int>，值是{27}
auto x3{27}; // 同上
```

这是`auto`的一条特殊的型别推导规则所致，当用于`auto`声明变量的初始化表达式是大括号括起时，推导的型别就属于`std::intializer_list`

如果型别推导失败，例如大括号里类型不一，则编译失败。

对于大括号初始化表达式的处理方式，是`auto`型别推导和模板型别的唯一区别。对于`auto`，推导为`std::initializer_list`的一个实例；但是对应的模板推导失败

```cpp
auto x={11,23,9}; // std::initializer_list<int>

template<typename T>
void f(T param);

f({11,23,9}); // error
```

若指定如下，则可以推导出

```cpp
template<typename T>
void f(std::initializer_list<T> initList);

f({11,23,9}); // T为int，initList为std::initializer_list<int> 
```

---

C++14允许使用`auto`指出函数返回值需要推导，`lambda`表达式也会在形参声明中用到`auto`，但这些`auto`用法是在使用模板型别推导而非`auto`型别推导

所以以下无法通过编译

```cpp
auto createInitList()
  return {1,2,3}; // 错误，无法完成型别推导
 }
 
std::vector<int> v;
...

auto resetV=[&v](const auto& newValue){v=newValue;}; // lambda
...

resetV({1,2,3});  // 错误，无法完成型别推导
```

---

# 要点速记

- 一般情况下，`auto`和模板型别推导一样的，但`auto`会假定大括号括起的初始化表达式代表一个`std::initializer_list`，模板则不会
- 函数返回值或`lambda`中形参使用`auto`，意思是使用模板型别推导
