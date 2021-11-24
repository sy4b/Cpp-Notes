# 必须返回对象时，别妄想返回其reference

一旦程序员领悟了pass-by-value的效率之高，往往想要根除pass-by-value。这时候很可能犯下一个致命错误：传递一些reference指向不存在的对象

---

考虑一个表现有理数的`class`，内含一个函数用来计算两个有理数的乘积

```cpp
class Rational{
public:
    Rational(int numerator=0, int denominatior=1);  // rule24说明为什么这个构造函数不声明为explicit
    ...
private:
    int n, d;
    friend const Rational operator*(const Rational& lhs, const Rational& rhs);
};
```

以上的函数by value返回了一个对象。我们需要考虑这个对象的构造和析构成本。如果改成传递reference，就不需要付出代价

但是记住，reference只是一个名称，代表某个既定对象。任何时候看到一个reference，都应该立刻问：另一个名称是什么？

以上的函数中，我们不可能期望一个Rational对象在调用operator*之前就存在，也就是说

```cpp
Rational a(1,2);
Rational b(3,5);
Rational c=a*b;
```

这样的操作并不合理，如果要求返回一个reference指向这个数值，必须自己创建那个Rational对象

---

函数创建新对象的途径有两个：在stack空间或者heap空间创建

如果定义一个local变量，就是在stack空间创建对象。按照这个策略

```cpp
const Rational& operator*(const Rational& lhs, const Rational& rhs){
    Rational result(lhs.n*lhs.n, lhs.d*lhs.d);  // 糟糕的代码！
    return result;
}
```

这种做法不仅调用了构造函数（我们的目的就是避免调用构造函数），而且返回一个reference指向local对象，而函数退出的时候，这个歌对象就被销毁。

---

于是我们考虑在heap内构造一个对象，并返回reference指向他

```cpp
const Rational& operator*(const Rational& lhs, const Rational& rhs){
    Rational* result=new Rational(lhs.n*lhs.n, lhs.d*lhs.d);
    return *result;
}
```

这个做法还是需要构造函数，并且，谁来执行`delete`呢？

即便谨慎的`delete`，还是难以在这样的情况下阻止内存泄漏

```cpp
Rational w, x, y, z;
w=x*y*z;                // operator*(operator*(x,y),z)
```

这里绝对会导致内存泄漏

---

这时候或许会想到这样的办法：让`operator*`返回的reference指向一个定义于函数内部的`static` Rational对象

```cpp
const Rational& operator*(const Rational& lhs, const Rational& rhs){            // 烂代码
    static Rational result;
    result=...;
    return result;
}
```

就像所有使用`static`对象的设计，这一个函数立刻造成多线程安全性的问题。此外还有更加深层次的瑕疵：

```cpp
bool operator==(const Rational& lhs, const Rational& rhs);
Rational a, b, c, d;
...
if((a*b)==(c*d)){
    ...
}else{
    ...
}
```

猜猜怎么着？`(a*b)==(c*d)`将总是为`true`，不论

一旦将代码重新写为等价的函数形式，就很容易知道出了什么意外

```cpp
if(operator==(operator*(a,b),operator*(c,d)))
```

每一个`operator*`都返回内部定义的`static`对象

---

一个“必须返回新对象的”函数的正确写法：那就让它返回一个新对象呗

```cpp
inline const Rational operator*(const Rational& lhs, const Rational& rhs){
    return Rational(lhs.n*rhs.n, lhs.d*rhs.d);
}
```

当然，必须承受返回值的构造和析构成本，然而长远来看这只是为了正确行为付出的一个小小代价

---

# 请记住

- 绝不要返回指针或引用指向一个local stack对象或者heap-allocated对象
