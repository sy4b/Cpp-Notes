# 确定你的public继承塑造出is-a关系

以C++进行面向对象编程，最重要的一个规则是：public ineritance意味着is-a的关系，将这个规则牢牢记住！

---

如果你令Derived以`public`形式继承Base，你便是告诉编译器以及你的代码读者，每一个类型为Derived的对象同时也是个类型为Base的对象，反之不成立。

Base可以派上用场的地方，Derived同样可以。Base更一般化，Derived更加特殊化

C++对于`public`继承严格奉行上述见解，考虑以下例子

```cpp
class Person{...};
class Student: public Person{...};
```

根据生活经验我们知道，每个学生都是人，但并非每个人都是学生。这便是这个继承体系的主张。人的概念比学生更一般化，学生是人的一种特殊形式

承上所述，在C++领域中，任何函数如果期望获得一个类型为Person的实参，也都愿意接受一个Student的对象

```cpp
void eat(const Person& p);      // 任何人都会吃
void study(const Student& s);   // 只有学生才会到校学习
Person p;
Student s;
eat(p);
eat(s);
study(s);
study(p);                       // error
```

这个论点只对`public`继承才成立

---

`public`继承和is-a的等价关系听起来颇为简单，但有时候你的直觉会误导你

举个例子，企鹅是一种鸟，这是事实，鸟可以飞，这也是事实，如果我们用C++描述这层关系

```cpp
class Bird{
public:
    virtual void fly(); // 鸟可以飞
    ...
};
class Penguin: public Bird{
    ...
};

这里就遇上了乱流。事实上有数种鸟不会飞，包括企鹅，下面的继承关系塑造出较佳的真实性

```cpp
class Bird{
    ...                 // 没有声明fly函数
};

class FlyingBird: public Bird{
public:
    virtual void fly();
    ...
};

class Penguin: public Bird{
    ...                 // 没有声明fly函数
};
```

这样的继承体系比之前的更能反映真正的意思。可即便如此，我们还是没有处理好这些鸟事。

另一种处理的思想是为企鹅重新定义fly函数，让它产生一个运行期错误

```cpp
void error(const std::string& msg);
class Penguin: public Bird{
public:
    virtual void fly(){
        error("Attempt to make a penguin fly!");
    }
    ...
}
```

但是这里表达的意思和你所想的不同，这里不是说“企鹅不会飞”，而是说“企鹅会飞，但尝试那么做是一种错误”

从错误被侦测的时间点来看，上述做法实在运行期检测出来。而实际上我们可以在编译期就检测。那就是，不为Penguin定义fly函数

```cpp
class Bird{
    ...                         // 没有声明fly函数
};
class Penguin: public Bird{
    ...                         // 没有声明fly函数
};

Penguin p;
p.fly();                        // error
```

这种做法比较好

---

现在再考虑，可以让Square类`public`继承Rectangle类吗

看起来很合理，每个正方形都是一种矩形，反之不一定

```cpp
class Rectangle{
public:
    virtual void setHeight(int newHeight);
    virtual void setWidth(int newWidth);
    virtual int height()const;
    virtual int width()const;
    ...
};

void makeBigger(Rectangle& r){
    int oldHeight=r.height();
    r.setWidth(r.width()+10);
    assert(r.height()==oldHeight);      // 判断r的高度是否未曾改变
}
```

显然上述代码`assert`的结果永远为`true`

现在考虑这段代码，使用`public`继承，允许正方形被视为一种矩形

```cpp
class Square: public Rectangle{...};
Square s;
...
assert(s.width()==s.height());          // 这对所有正方形一定为true
makeBigger(s);                          // 为s增加面积
assert(s.width()==s.height());          // 对所有正方形应该仍然true
```

根据正方形的定义，第二个`assert`的结果也应该永远为`true`

但现在我们遇上了问题，如何调节下面各个`assert`判断式？

- 调用makeBigger之前，s的高度和宽度相同
- 在makeBigger内，s宽度改变高度不变
- makeBigger返回之后，s的高度再度和其宽度相同

？怎么样？

---

欢迎来到`public`继承的精彩世界，这里你在其他领域学习到的直接，恐怕无法像预期一般帮助你。

本例的根本困难在于，某些可以施行与矩形上的事情（例如宽度可以独立于其高度被外界修改）却不可以施行在正方形上。所以我们用`public`继承塑膜他们的关系并不正确

is-a并不是唯一存在于`class`之间的关系，另外两个常见的关系是has-a和is-implemented-in-terms-of（根据某物实现出）,将在rule38 39讨论

---

# 请记住

- `public`继承意味着is-a关系。适用于base class身上的每一件事情都必须适用于derived class身上，因为每一个derived class对象也都是一个base class对象
