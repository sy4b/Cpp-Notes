# 了解new和delete的合理替换时机

---

什么情况下会有人想要替换编译器提供的`operator new`或`operator delete`呢？

- 检测运用上的错误：各种编程错误可能导致数据写在分配内存区块之前或者之后(overruns, underruns)。我们可以自行定义一个operator new，超额分配内存，在额外的空间放置特定的标志位，operator delete检查上述标志位是否原封不动，以检测是否出现overruns underruns
- 强化效能：编译器自带的new和delete实现比较一般化，我们可以通过自己定制，让新版本在特定情况下表现更好
- 收集使用上的统计数据：在定制new和delete之前理应先收集你的软件如何使用动态内存。自行定义new和delete使我们可以收集到这些信息

---

以下实现检测overruns（存在一点小错误稍后纠正

```cpp
static const int signature=0xDEADBEEF;
using Byte=unsigned char;
void* operator new(std::size_t size)throw(std::bad_alloc){
    using namespace std;
    size_t realSize=size+2*sizeof(int);     // 超额分配

    void* pMem=malloc(realSize);
    if(!pMem){
        throw bad_alloc();
    }

    // 内存最前和最后写入签名
    *(static_cast<int*>(pMem))=signature;
    *(reinterpret_cast<int*>(static_cast<Byte*>(pMem)+realSize-sizeof(int)))=signature;

    return static_cast<Byte*>(pMem)+sizeof(int);
}
```

这里定制版的new没有和编译器版本的new一样，内含一个循环，不断调用某个new-handling函数。这里暂时忽略这个问题，考虑齐位问题

---

许多计算机体系结构要求特定类型必须放在特定的内存地址上，例如要求指针的地址必须是4的倍数或double的地址必须是8的倍数，如果没有遵守这个条件，可能会导致运行期硬件异常。有些体系要求比较宽松，在其为条件满足时便提高最佳效率

C++要求所有new返回的指针都有适当的对齐，`malloc`就工作在这样的要求下，所以令一个new返回一个得自`malloc`的指针是安全的，但上述new中我们返回一个`malloc`返回的指针，并且进行偏移，这样没人能保证它的安全！

很多时候自己定制new和delete是没必要的

---

本条款的主题是，了解何时可在“全局性”或“class专属”的基础上合理替换缺省的`new`和`delete`

- 为了检测运用错误
- 手机动态分配内存使用统计信息
- 增加分配和归还的速度
- 降低缺省内存管理器带来的额外空间开销
- 弥补缺省分配器中的非最佳齐位
- 将相关对象成簇集中
- 获得非传统的行为