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

