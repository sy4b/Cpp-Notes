# 理解decltype

对于给定的名字或表达式，`decltype`指出它的型别。

一般来说他给出的结果和程序员的预测相同，但偶尔也会有例外

---

从一般性情况讲起，和`auto`和模板的型别推导过程相反，`decltype`一般只会鹦鹉学舌，返回给定的名字或表达式的确切型别

```cpp
const int i=0;            // decltype(i)是const int

bool f(const Widget& w);  // decltype(w)是const Widget&，decltyle(f)是bool (const Widget&)

struct Point{
  int x, y;
};                        // decltype(Point::x)和decltype(Point::y)是int

Widget w;                 // decltype(w)是Widget

if(f(w)){...}             // decltype(f(w))是bool

template<typename T>
class vector{
public:
  ...
  T& operator[](std::size_t index);
  ...
};

vector<int> v;            // decltype(v)是vector<int>
...
if(v[0]==0){...}          // decltype(v[0])是int&
```

C++11中，`decltype`主要用于声明那些返回值依赖于形参型别的函数模板。

例如我们写一个函数，形参包括一个容器，支持`opertaor[]`和下标，并会在返回下标操作之前进行用户验证。函数的返回值型别与下标操作结果型别相同

一般来说，含有型别`T`的容器，其`operator[]`会返回`T&`。`std::queue`就属于这种情况，`std::vector`也几乎是这种情况，只有`std::vector<bool>`会返回一个全新对象（见Rule#6)

```cpp
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i)->decltype(c[i]){  // 能运作，但亟待改进
  authenticateUser();
  return c[i];
}
```

- 在函数名使用的`auto`和型别推导毫无关系，只为了说明使用C++11的返回值型别尾序语法，即该函数的返回值型别位于形参列表->之后。
- 尾序返回值的好处在于指定返回值型别时，可以使用函数形参

C++11允许对单表达式的lambda表达式返回值类型进行推导。

C++14将范围扩大到一切lambda和一切函数，包括多表达式的。这意味着可以去掉尾置返回值，只保留`auto`。在该声明形式中，`auto`指明编译器依据函数实现来实施函数返回值的型别推导

```cpp
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i){  // C++14 不甚正确
  authenticateUser();
  return c[i];                              // 返回值型别根据c[i]推导
}
```

但根据[Rule#2](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Note/Rule%232%20理解auto型别推导.md)，编译器会对指定返回值类型为`auto`的函数实现模板型别的推导，对于上一个例子，这样就会留下隐患

```cpp
std::queue<int> d;
...
authAndAccess(d, 5)=10; // 验证用户，返回d[5]，将其赋值为10 但无法通过编译
```

此处`d[5]`返回引用，但模板类型推导将引用性剥夺（见[Rule#1](https://github.com/sy4b/Cpp-Notes/blob/main/Effective%20C%2B%2B%20Note/Rule%231%20型别推导.md)），因此得到的是一个int右值，而将10赋值给一个右值是禁止的

如果想让这个函数按照期望运作，就需要对返回值实施`decltype`型别推导，指定返回值型别与c[i]完全一致，C++14中采用`decltype(auto)`饰辞，`auto`指明需要实施推导，推导过程采用`decltype`的规则

```cpp
template<typename Container, typename Index>
decltype(auto)  authAndAccess(Container& c, Index i){ // C++14，可以运作，但仍然亟待改进
  aythenticateUser();
  return c[i];
};
```

`decltype(auto)`并不限于函数返回值型别处使用，在变量声明时，也可以用其推导

```cpp
Widget w;

const Widget& cw=w;

auto myWidget1=cw;            // Widget

decltype(auto) myWidget2=cw;  // const Widget&
```

---

最后来看改进

```cpp
template<typename Container, typename Index>
decltype(auto)  authAndAccess(Container& c, Index i){ // C++14，可以运作，但仍然亟待改进
```

- 容器的传递方式是非常量的左值引用，也就意味着允许客户对容器进行修改
- 但也意味着无法向该函数传递右值容器，因为右值不能绑定到左值引用

一般来说不会向函数传递右值容器，因为作为一个临时对象，引用在被析构后处于空悬状态

```cpp
// error
std::deque<std::string> makeStringDeque();  // 工厂函数
// 制作makeStringDeuqe返回的deue的第五个元素的副本
auto s=authAndAccess(makeStringDeque(),5);
```
如果要支持上述的语法，就要修改`authAndAccess`的声明，以同时接受左值和右值，重载是一种办法，但万能引用此时可以大展身手。

```cpp
template<typename Container, typename Index>
decltype(auto) authAndAccess(ConTainer&& c, Index i);
```

同时需要更新实现，对万能引用需要应用std::forward

```cpp
// C++14最终版

template<typename Container, typename Index>
decltype(auto) authAndAccess(ConTainer&& c, Index i){
  authenticateUser();
  return std::forward<Container>(c)[i];
}

// C++11最终版
template<typename Container, typename Index>
auto authAndAccess(ConTainer&& c, Index i)->decltye(std::forward<Container>(c)[i]){
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```
