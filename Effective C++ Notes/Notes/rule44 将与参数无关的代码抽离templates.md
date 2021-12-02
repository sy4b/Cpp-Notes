# 将与参数无关的代码抽离templates

templates是节约时间和避免重复代码的有效手段，可以写一个函数模板或类模板，让编译器做具现化的事情

但是如果使用不当，`template`可能会导致代码膨胀：其二进制带着重复的代码、数据，或两者都有。结果来看就是源码看起来合身而整齐，但目标码却不是这么回事。你需要知道如何避免这样的二进制浮夸

你的主要工具有个气势恢宏的名称：共性与变性分析，其概念十分平民化。纵使你从未写过一个`template`，你也始终做着这样的分析

当你编写某个函数，而你明白其中某些部分的实现码实质相同，你会单纯的重复这些码吗？当然不，你会抽出两个函数的共同部分，放入第三个函数之中，然后令原先两个函数调用这个新函数。同样道理，如果一个类的某些部分和另一个类重合，你会选择将共同部分搬到一个新类，使用继承或复合，让原先的classes取用共同特性，原classes的互异部分保留在原位置

编写tempaltes也是相同的分析，避免重复。这其中有个窍门。在non-template代码中，你可以明确的看到重复部分。而在`template`代码中，重复部分是隐晦的，必须用心感受

假设现在为一个固定尺寸的方阵编写一个模板，该矩阵支持逆运算

```cpp
template<typename T, std::size_t n>
class SquareMatrix{
public:
    ...
    void invert();
};

SquareMatrix<double, 10> sm1;
sm1.invert();

SquareMatrix<int, 5> sm2;
sm2.invert();
```

这两个函数并非完全相同：除了常量5和10，其他部分完全相同，这是引出代码膨胀的一个典型例子

我们可以本能的为他们建立一个带数值参数的函数，然后用5和10来调用这个带参数的函数

```cpp
template<typename T>
class SquareMatrixBase{
protected:
    ...
    void invert(std::size_t martixSize);
    ...
};
template<typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase<T>{
private:
    using SquareMatrixBase<T>::invert;
public:
    ...
    void invert(){
        this->invert(n);    // 避免模板化基类内的函数名称被掩盖
    }
};
```

带参数的invert位于base class，这个模板类只对元素类型参数化，不对矩阵尺寸参数化

目前一切都很好，但还有棘手的问题没解决。SquareMatrixBase<T>::invert如何知道操作哪些数据？只有derived class才知道。那么derived class如何联络base class做逆运算？一个可能的做法是为SquareMatrixBase<T>::invert添加一个参数，maybe是个指针，指向一块放置矩阵数据的内存初始点。但这种方法不太好，因为SquareMatrixBase<T>::invert可能不是唯一一个可以分离出来的函数，其他函数也添加这个部分造成代码重复

另一个办法是令SquareMatrixBase贮存一个指针，指向矩阵数值所在的内存

```cpp
template<typename T>
class SquareMatrixBase{
protected:
    SquareMatrixBase(std::size_t n, T* pMem):size(n), pData(pMem){}
    void setDataPtr(T* ptr){pData=ptr}
    ...
private:
    std::size_t size;
    T* pData;               // 指针，指向矩阵内容
};
```

这样允许derived class决定内存分配方式

```cpp
template<typename T, std::size_t n>
class SquareMatrix: private SquateMatrixBase<T>{
public:
    SquareMatrix(): SquareMatrixBase<T>(n, data){}
    ...
private:
    T data[n*n];
};
```

这种类型的对象不需要动态分配内存，但对象自身可能非常大。另一个做法是将矩阵数据放在heap中

```cpp
template<typename T, std::size_t n>
class SquareMatrix: private SquateMatrixBase<T>{
public:
    SquareMatrix(): SquareMatrixBase<T>(n, 0), pData(new T[n*n]){this->setDataPtr(pData.get());}
    ...
private:
    boost::scoped_array<T> pData;
};
```

此时不论数据存在何处，用膨胀的角度考虑，现在所有SquareMatrix成员函数可以单纯的`inline`方式调用base class 版本，后者由持有相同类型的元素的矩阵共享，并且不同大小的SquareMatrix对象有不同的类型，所以即使对象使用相同的成员函数，也无法相互传递，例如SquareMatrixBase<double, 5>和SquareMatrixBase<double, 10>都调用SquareMatrix<double>成员函数

着很棒，但必须付出代价。绑定着矩阵尺寸的invert版本，有可能生成比共享版本更加的代码。例如在尺寸专属版本中，尺寸是个编译器常量，可以藉由常量的广传达到最优化，包括把他们折进被生成指令中成为直接操作数。这在“与尺寸无关”版本中无法办到

从另一个角度看，不同大小的矩阵只有单一版本的invert，可以减少执行文件大小，降低程序的working set（对于一个在“虚内存”环境下执行的进程而言，其所使用的那一组内存页）,强化指令高速缓存区内的引用集中化，这些使得程序执行更快。

那么哪一种更合适？具体还得看你的平台的行为以及代表性数据的行为

另一个评价关心的主题是对象大小

---

这个条款只讨论非类型模板参数带来的膨胀，其实类型参数也会导致膨胀。例如许多平台上`int`和`long`有相同的二进制表述，所以vector<int>和vector<long>的成员函数很有可能完全相同。

类似情况，大多数平台上，所有指针类型都有相同的二进制表述，因此凡templates持有指针者（例如list<int*> list<const int*>），往往应该对每一个成员函数使用唯一一份底层实现

这意味着，如果你实现某些成员函数而他们操作强型指针，你应该令他们调用一个无类型指针（void*）的函数，后者完成实际工作

---

# 请记住

- templates生成多个classes和多个函数，任何templates代码都不该与某个造成膨胀的templates参数产生相依关系
- 非类型模板参数造成的代码膨胀可以以函数参数或`class`成员变量替换`template`参数
- 类型参数造成的代码膨胀可以让带有完全相同二进制表述的具现类型共享实现码