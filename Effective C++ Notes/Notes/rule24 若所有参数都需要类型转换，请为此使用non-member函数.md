# 若所有参数都需要类型转换，请为此使用non-member函数

令`class`支持隐式类型转换通常是个糟糕的注意。当然这条规则有例外，常见的例外是在建立数值类型时。

假设设计一个`class`用来表现有理数，允许整数“隐式转换”为有理数似乎颇为合理

---

假设像以下这样开始

```cpp
class Rational{
public:
    // 构造函数刻意部位explicit，允许int-to-Rational隐式转换
    Rational(int numerator=0, int denominator=1);
    int numerator() const;
    int denominator() const;
private:
    ...
};
```

这个`class`想要支持算术运算，但你不确定是否应该由member函数、non-member函数等等实现

先研究一下成员函数的写法

```cpp
class Rational{
public:
    ...
    const Rational operator*(const Rational& rhs)const;
};
```

这个设计使得你可以将两个有理数轻松的相乘

```cpp
Rational oneEight(1, 8);
Rational oneHalf(1, 2);
Rational result=oneHalf * oneEight;     // 很好
result = result * oneEight;             // 很好
```

但你还不满足，希望支持混合式运算，例如拿`Rational`和`int`相乘，然而你会发现，只有一半行得通

```cpp
result = oneHalf * 2;               // ok
result = 2 * oneHalf;               // invalid
```

这不是个好兆头，乘法应该满足交换律。用对应函数形式重写上述两个式子，问题便一目了然

```cpp
result = oneHalf.operator*(2);
result = 2.operator*(oneHalf);      // error
```

`2`并没有相应的`class`。第一个成功调用是因为隐式类型转换发生，`int`转换为`Rational`。如果构造函数`explicit`，那他也无法通过编译

---

通过让`operator*`成为非成员函数，就可以让编译器在每一个实参身上执行隐式类型转换

```cpp
class Rational{
    ...
};

const Rational operator*(const Rational& lhs, const Rational& rhs){
    return Rational(lhs.numerator()*rhs.numerator(), lhs.denominator()*rhs.denominator());
}
```

这时候还要问：是否应该设置为`friend`函数呢？对于本例不用，因为完全可以通过`public`接口完成任务。具体还要看情况

---

# 请记住

- u如果需要为某个函数的所有参数（包括`this`）进行类型转换，那么这个函数必须是个non-member
