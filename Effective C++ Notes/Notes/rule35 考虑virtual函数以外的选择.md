# 考虑virtual函数以外的选择

假设正在写一个视频游戏软件，打算为游戏内的人物设计一个继承体系。你的游戏属于暴力砍杀类型，剧中人物被伤害或其他因素降低将康状态的情况并不罕见

因此你决定提供一个成员函数`healhValue`，返回一个整数，表示人物的健康程度。由于不同人物可能以不同方式计算健康指数，因此将其声明为`virtual`似乎是再明白不过的做法

```cpp
class GameCharacter{
public:
    virtual int healthValue()const; // derived class可重新定义它
    ...
};
```

这里`healhValue`不是pure virtual函数，暗示我们有个缺省算法

这的确是一个再明白不过的设计，但从某个角度来说反而成为了它的弱点。为了帮助跳过面向对象设计路上的常轨，我们考虑其他解法

---

## 藉由non-virtual interface手法实现template method模式（模板模式）

从一个有趣的思想流派开始，这个流派主张`virtual`函数应该几乎总是`priavte`。他们建议较好的设计是保留`healthValue`为`public`成员函数，但让它成为non-virtual函数，并调用一个private virtual函数（例如doHealthValue）进行实际工作

```cpp
class GameCharacter{
public:
    int healthValue()const{
        ...                             // 事前工作，祥下
        int retVal=doHealthValue();     // 做真正的工作
        ...                             // 事后工作，详下
        return retVal;
    }
    ...
private:
    virtual int doHealthValue()const{
        ...                             // 计算健康指数
    }
};
```

本rule为了方便，都直接在`class`定义内呈现成员函数本体，如rule30所言，他们暗自成为了`inline`，但实际上这里这是为了鹅方便阅读，不是一定要让他们inlining

这一基本设计，也就是“令客户通过public non-virtual成员函数间接调用private virtual函数”，成为non-virtual interface(NVI)手法。它是所谓Template Method(模板模式)设计模式的一个独特表现形式。

可以把这个non-virtual函数称为`virtual`函数的外覆器

NVI手法的一个有点隐身在上述代码注释“事前工作”“事后工作”之中。这意味着外覆器确保在一个`virtual`函数被调用之前设定好适当场景，并在调用结束之后清理场景

“事前工作”可以包括锁定互斥器、制造运转日志记录项、验证`class`约束条件、验证函数先决条件等等。事后工作包括解除互斥器锁定等等...

如果让客户直接调用`virtual`函数，就没办法做好这些事

NVI手法涉及在derived class内重新定义private virtual函数。这是允许的

在NVI手法下其实没有必要让`virtual`函数必须得是`private`，某些`class`继承体系要求derived class在`virtual`函数的实现内必须调用其base class的对应兄弟，为了让这样的调用合法，`virtual`函数必须是`protected`，不能是`private`

有时候`virtual`函数甚至一定得是`public`（例如具备多态性质的析构函数——rule7）如此一来就不能实施NVI手法了

---

## 藉由function pointers实现Strategy模式（策略模式）

另一个戏剧性的设计主张“人物健康指数的计算与人物类型无关”，这样的计算完全不需要“人物”这个成分。

例如我们可能会要求每个人物的构造函数接受一个指针，指向个健康计算函数，而我们可以通过调用该函数进行实际计算

```cpp
class GameCharacter;            // 前置声明
int defaultHealthCalc(const GameCharacter& gc); /// 计算健康指数的缺省算法
class GameCharacter{
public:
    typedef int (*HealthCalcFunc)(const GameCharacter&);
    explicit GameCharacter(HealthCalcFunc hcf=defaultHealthCalc): healthFunc(hcf){}
    int healthValue()const{return healthFunc(*this);}
    ...
private:
    HealthCalcFunc healthFunc;
}
```

这个做法是常见的Strategy设计模式的简单应用。它提供了某些有趣弹性：

- 同一人物类型之不同实体可以有不同的健康计算函数，例如

```cpp
class EvilBadGuy: public GameCharacter{
public:
    explicit EvilBadGuy(HealthCalcFunc hcf=defaultHealthCalc): GameCharacter(hcf){...}
    ...
};
int loseHealthQuickly(const GameCharacter&);    // 1
int loseHealthSlowly(const GameCharacter&);     // 2

EvilBadGuy ebg1(loseHealthQuickly);
EvilBadGuy ebg2(loseHealthSlowly);
```

- 某个人物的健康指数计算函数可在运行期变更

换句话数，健康计算函数不再是`GameCharacter`继承体系内的成员函数。这一事实意味着，这些计算函数并未特别访问被计算对象的内部成分

这也意味着，如果健康指数需要non-public信息计算，这个方式就有问题了

一般而言，唯一解决“需要non-member函数访问`class`的non-public成分”的办法是：弱化`class`的封装。例如设置友元，或者为其实现的一部分提供`public`函数

---

## 藉由tr1::function完成Strategy模式

一旦习惯了templates以及他们对隐式接口的使用（rule41），基于函数指针的做法看起来便过分苛刻死板了。为什么健康指数的计算必须是个函数，而不能是某种看起来像函数的东西？如果一定得函数，为什么不能是成员函数？为什么一定得返回`int`而不是任何可被转换为`int`的类型呢？

如果我们不再使用函数指针，而是改用一个`tr1::function`对象，这些约束就全都挥发不见了。如rule54所说，这样的对象可以保存任何可调用物（函数指针、函数对象或成员函数指针），只要其签名式兼容与需求端

```cpp
class GameCharacter;
int defaultHealthCalc(const GameCharacter& gc);
class GameCharacter{
public:
    // HealthCalcFunc可以是任何可调用物，接受任何兼容GameCharacter，返回任何兼容int的东西
    typedef std::tr1::function<int(const GameCharacter&)> HealthCalcFunc;
    explicit GameCharacter(HealthCalcFunc hcf=defaultHealthCalc): healthFunc(hcf){}
    int healthValue()const{return healthFunc(*this);}
private:
    HealthCalcFunc healthFunc;
};
```

```cpp
short calcHealth(const GameCharacter&); // 返回类型non-int

struct HealthCalculator{                // 为计算健康指数设计的函数对象
    int operator()(const GameCharater&)const{...}
};

class GameLevel{
public:
    float health(const GameCharacter&)const;    // 成员函数 non-int
    ...
};

class EvilBadGuy: public GameCharacter{
    ...     // 同上
};

class EyeCandyCharacter: public GameCharacter{
    ...     // 另一个人物类型，假设构造函数和EvilBadGuy相同
};

EvilBadGuy ebg1(calcHealth);    // 人物1 使用某个函数计算

EyeCandyCharacter ecc(HEalthCalculator())   // 人物2 使用某个函数对象计算

GameLevel currentLevel;
...
EvilBadGuy ebg2(std::tr1::bind(&GameLevel::health, currentLevel, _1));          // 人物3 使用成员函数计算，详下
```

为了计算ebg2的健康指数，应该使用`GameLevel`的成员函数`health`

`GameLevel::health`宣称自己接受一个参数（指向`GameCharacter`的reference），但实际上它接受两个参数，因为他也获得一个隐式参数`GameLvel`，也就是`this`指向的那个

然而`GameCharacter`的健康计算函数只接受一个参数，我们必须用某种方式将`GameLevel::health`转换

这里我们会想要使用`currentLevel`作为ebg2的健康计算函数所需的`GameLevel`对象，于是我们将`currentLevel`绑定为`GameLevel`对象，让他在每次`GameLevel::health`被调用计算ebg2的健康时被使用

那正是`tr1::bind`的作为：指出ebg2的健康计算函数应该总是以`currentLevel`作为`GameLevel`对象

---

## 古典的Strategy模式

传统的Strategy做法会讲健康计算函数做成一个分离继承体系中的`virtual`成员函数，设计结果看起来像这样

![](https://github.com/sy4b/Cpp-Notes/blob/main/Graph/截屏2021-11-30%20上午10.37.01.png)

图片的意思是：`GameCharacter`是某个继承体系的根类，`EvilBadGuy`和`EyeCandyCharacter`都是derived class。同样，`HealthCalcFunc`是某个继承体系的根类，`SlowHealthLoser`和`FastHealthLoser`都是derived class。每一个`GameCharacter`对象都含有一个指针，指向一个来自`HealthCalcFunc`继承体系的对象

```cpp
class GameCharacter;
class HealthCalcFunc{
public:
    ...
    virtual int calc(const GameCharacter& gc)const{...}
    ...
};
HealthCalcFunc defaultHealthCalc;
class GameCharacter{
public:
    explicit GameChatacter(HealthCalcFunc* phcf=&defaultHealthCalc: pHealthCalc(phcf)){}
    int healthValue()const{return pHealthCalc->calc(*this);}
    ...
private:
    HealthCalcFunc* pHealthCalc;
};
```

熟悉标准Strategy模式的人很容易辨识。并且提供一个将既有的健康算法纳入使用的可能性——只要为`HealthCalcFunc`继承体系添加一个derived class即可

---

## 摘要

- 使用non-virtual interface(NVI)手法，这是template method设计模式的一种特殊形式。以public non-virtual成员函数包裹较低访问性（`public`或`protected`）的`virtual`函数
- 将`virtual`函数替换为“函数指针成员变量”，这是Strategy设计模式的一种分解表现形式
- 以`tr1::function`成员变量替换`virtual`函数，因而允许使用任何可调用物搭配一个兼容与需求的签名式。这也是Strategy设计模式的某种形式
- 将继承体系内的`virtual`函数替换为另一个继承体系内的`virtual`函数，这是Strategy设计模式的传统实现手法

---
