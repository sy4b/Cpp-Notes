# 认识模板元编程

TMP(template metaprogramming)是编写template-based C++程序并执行于编译期的过程

TMP使得某些事情变容易，甚至从不可能到可能；TMP可以将工作从运行期转移到编译期，可以找到一些原本在运行期才能找到的错误；使用TMP的C++程序可能在各个方面都更高效：较小可执行文件、较短运行期、较少内存需要等等；

但TMP使得编译时间变长

---

考虑实现advance的伪代码

```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d){
    if(.../*iter为random access迭代器*/){
        iter+=d;
    }else{
        if(d>=0){
            while(d--){
                ++iter;
            }else{
                while(d++){
                    --iter;
                }
            }
        }
    }
}
```

以下使用`typeid`实现，所有工作都在运行期完成

```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d){
    if(typeid(typename std::iterator_traits<IterT>::iterator_category)==typeid(std::random_access_iterator_tag)){
        iter+=d;
    }else{
        if(d>=0){
            while(d--){
                ++iter;
            }else{
                while(d++){
                    --iter;
                }
            }
        }
    }
}
```

这个解法比traits解法低，因为类型测试发生在运行期而不是编译期；运行期测试代码会连接与可执行文件中

taraits解法本身就是一种TMP，这个例子可以彰显TMP如何比通常的C++程序高效

---

除了效率问题，上述typeid-based解法可能存在编译期问题

例如调用以下代码

```cpp
std::list<int>::iterator iter;
advance(iter, 10);

// 针对以上调用，advance生成以下具现化
void advance(std::list<int>::iterator& iter, int d){
    if(typeid(typename std::iterator_traits<std::list<int>::iterator>::iterator_category)==typeid(std::random_access_iterator_tag)){
        iter+=d;        // 问题所在
    }else{
        if(d>=0){
            while(d--){
                ++iter;
            }else{
                while(d++){
                    --iter;
                }
            }
        }
    }
}
```

问题出现在`+=`那一行，我们知道对于`list<int>::iterator`这种bidirectional迭代器，他不支持`+=`操作，并且实际中也不会执行这段代码。但编译器必须确保所有源码有效，即使是不会被执行起来的代码

而traits解法针对不同类型拆分成不同函数，对于所有操作都可以施行

---

TMP是一个“图灵完全机器”，足以计算任何事物。使用TMP可以声明变量、执行循环、编写调用函数...但是表现形式可能与正常的C++对应物有所不同，例如我们提到的if-else结构，在TMP中藉由templates和特例化、重载函数完成

我们看看循环在TMP中如何实现。TMP没有真正的循环组件，所有循环效果由递归完成

以编译期计算阶乘为例

```cpp
template<unsigned n>
struct Factorial{
    enum{value = n * Factorial<n-1>::value};
};

// 相当于递归终止条件
// 模板特例化
template<>
struct Factorial<0>{
    enum{value=1};
};

// 可以这样使用Fatorial
int main(){
    std::cout<<Factorial<5>::value;     // 计算5！
    ...
}
```

---

TMP能够达成怎样的目标？

- 确保量度单位正确。例如把一个质量赋给一个速度是错误的，但将一个距离除以时间，结果付给一个速度则正确。TMP可以在编译期程序中所有度量单位的结合正确，可以进行早期错误侦测
- 优化矩阵运算。例如rule44倒入的矩阵类，进行operator*连乘时，正常的函数调用会创建很多暂时性对象存储返回对象。而使用TMP可以避免，减少内存消耗，提高执行速度而无需改变客户端用法
- 可以生成客户定制的设计模式实现品，例如策略模式，观察者模式等设计模式，运用所谓policy-based design的TMP技术，可以产生一些模板用来表述独立的设计选项，然后任意结合他们，使得模式实现品带着客户定制的行为

---

# 请记住

- TMP将工作从运行期移动到编译期，可以实现早期错误侦查和实现更高执行效率
- TMP可以被用来生成“基于政策选择组合”的客户定制代码，避免生产某些特殊类型不适合的代码