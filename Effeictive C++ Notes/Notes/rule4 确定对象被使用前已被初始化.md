# 确定对象被使用前已被初始化

关于初始化对象这件事，C++表现的反覆无常

```cpp
int x;      // 某些语境下被初始化为0，其他语境却不这么保证

class Point{
    int x,y;
};
Point p;    // p的成员变量有时候被初始化为0，有时候却不会
```

读取未初始化的值会导致不明确的行为。

最佳的处理办法是，永远在使用对象之前将其初始化。

对于无任何成员的内置类型，必须手动完成

```cpp
int x=0;

const char* text="A C-style string";

double d;
std::cin>>d;        // 以读取输入流的方式完成初始化
```

对于内置类型之外的自定义类型，初始化责任落在构造函数身上。构造函数最好使用成员初始值列表替换赋值动作

赋值的版本首先调用默认构造函数设置成员初始值，然后立即赋予新值。

某些情况下即使成员是内置类型（初始化和赋值的成本相同），也必须使用初始值列表，例如`const`和`reference`，它们不支持赋值

C++有固定的成员初始化次序：`base class`更早于其`derived class`被初始化，`class`变量成员则总是以声明次序被初始化，即使在成员初始值列表中以不同次序出现。

---

需要注意在不同编译单元内定义`non-local static`对象的初始化次序

所谓`static`对象，寿命从被构造出来知道程序结束，因此stack和heap-based对象都被排除，包括global对象、定义在namespace作用域内的对象、在classes内、在函数内、以及在file作用域内被声明为`static`的对象。

函数内的`static`对象称为`local static`对象，其他对象称为`non-local static`对象

编译单元指的是产出单一目标文件的源码，基本上是单一源码文件和头文件

```cpp
class FileSystem{                   // 来自你的程序库
public:
    ...
    std::size_t numDisks()const;
    ...
};
extern FileSystem tfs;              // 给用户使用的对象
```

如果客户在使用FileSystem对象之前没有完成构造，就会出现问题。例如客户新建一个`class`用于处理文件系统内的目录

```cpp
class Directory{
public:
    Directory(params);
    ...
};

Directory::Directory(params){
    ...
    std::size_t disks=tfs.numDisks();   // 使用tfs对象
}

Directory tempDir(params);              // 对象
```

此时初始化顺序的重要性显现出来了，`tfs`必须现在`tempDir`之前初始化，都则构造函数就无法工作。但`tfs`和`tempDir`是不同的人在不同的源码文件之中建立起来的，是定义于不同编译单元的`non-local static`对象，如何确定初始化顺序？

C++对于这种情况的初始化顺序没有明确定义

需要用一个小设计消除这个问题：将每个`non-local static`对象搬到自己的专属函数内，该对象在此函数内被声明为`static`，这些函数返回一个`reference`指向他所包含的对象。用户调用这些函数而不直接指涉这些对象。这是Singleton模式的一个常见实现手法

```cpp
class FileSystem{...};          // 同前
FileSystem& tfs(){              // 该函数用于替换tfs对象
    static FileSystem fs;       // 定义并返回一个local static对象
    return fs;                  // 返回一个reference指向上述对象
}

class Directory{...};           // 同前
Directory::Direcotry(params){
    ...
    std::size_t disk=tfs().numDisks();
    ...
}

Directory& tempDir(){
    static Directory td;
    return td;
}

```
---

# 请记住

- 为内置类型对象进行手工初始化，因为C++不保证初始化它们
- 构造函数最好使用成员初始化列表，而不要在构造函数本体那赋值操作。列表的排列次序应当和成员声明的次序
- 为避免跨编译单元之间的初始化次序问题，请用local static对象替换non-local stati对象
