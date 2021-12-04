# 编写new和delete需要固守常规

---

从`operator new`开始，实现一致性`operator new`包括：返回正确的值，内存不足时必须调用new-handling函数，必须有对付零内存需求的准备，必须避免不慎掩盖正常形式的`new`

`operator new`的返回值很单纯，如果他有能力供应客户申请的需求（包括new-handling函数），那么返回一个指针指向那块内存，否则抛出一个`bad_alloc`异常

奇怪的是C++规定，即使客户要求0bytes，`operator new`也得返回一个合法指针，这种诡异的行为是为了简化语言其他部分

下面是个non-member operator new伪代码

```cpp
void* operator new(std::size_t size) throw(std::bad_alloc){
    using namespace std;
    if(size==0){
        size=1;         // 处理0bytes申请
    }
    while(true){
        尝试分配size bytes;
        if(分配成功){
            return (指向分配得到的内存的指针)
        }
        // 分配失败，找出目前的new-handling函数
        new_handler globalHandler=set_new_handler(0);
        set_new_handler(globalHandler);

        if(globalHandler) (*globalHandler)();
        else throw std::bad_alloc();
    }
}
```

这里将0bytes申请视为1byte申请，简单合法可行

这里将new-handling函数设置为`nullptr`后又恢复原样，是因为我们没有任何办法可以直接取得new- handling函数指针，所以必须调用set_new_handler函数将其找出来，拙劣但有效——至少对于单线程环境。如果实在多线程环境，或许需要某种机锁以便安全处置new-handling函数背后的数据结构

[rule49]()提到`operator new`内含一个无限循环，对应上述伪代码while(true)部分，推出该循环的唯一办法就是内存被成功分配或new-handling函数：让更多内存可用、安装另一个new-handler、卸除new-handler、抛出`bad_alloc`异常或其派生，或是直接承认失败return

---

`operator new`成员函数会被derived class继承，这会导致某些有趣的复杂度。

写出定制型内存管理器的一个最常见理由是针对某个特定`class`的对象分配行为提供最优化，而不是为了该`class`的任何derived class。例如针对class X设计的`operator new`，其行为很典型的只为大小刚好位sizeof(X)的对象而设计，然而一旦被继承下去，有可能base class的`operator new`被调用去分配derived class对象

```cpp
class Base{
public:
    static void* operator new(std::size_t size)throw(std::bad_alloc);
    ...
};

class Derived: public Base{
    ...         // 未声明operator new
};

Derived* p=new Derived;     // 此处调用Base::operator new
```

处理这种情况的最佳做法是：将内存申请量错误的调用行为改为采用标准`operator new`

```cpp
void* Base::operator new(std::size_t size)throw(std::bad_alloc){
    if(size!=sizeof(Base)){
        return ::operator new(size);
    }
    ...
}
```

这里关于size==0的判断，与sizeof(Base)的检测合并在一起，这里Base不再是一个独立式对象（作为一个base class），所以Base大小不可能为0.如果size==0，这份内存请求申请会交给::operator new，后者有责任以某种合理方式处理

---

如果打算控制`class`的“array内存分配行为”，需要实现`operator new`的array兄弟版本：`operator new[]`

你需要做的事就是分配一块未加工内存，因为你无法对array之内尚未存在的元素对象做任何事，你甚至不知道这个array将包含多少个对象，也不知道对象有多大，该分配多少内存

因此你不能在Base::operator new[]内假设array的每个元素对象大小是sizeof(Base)，也就意味着不能假设array的元素对象个数是(bytes申请量)/sizeof(Base)。此外传递给`operator new[]`的size_t参数，其值可能比“将被填上对象”的内存数量更大

---

`operator delete`情况更简单，C++保证“删除`nullptr`指针永远安全”，你必须兑现这项保证

```cpp
void operator delete(void* rawMemory)throw(){
    if(rawMemory==nullptr){
        return;             // 将被删除的是个nullptr，则什么都不用做
    }
    归还rawMemory所指向的内存;
}
```

这个函数的member版本需要多加一个动作检查删除数量。万一你的`class`专属`operator new`将大小有误的分配行为转交给`::operator new`执行，你也必须将大小有误的删除行为转交`::operator delete`执行

```cpp
class Base{
public:
    static void* operator new(std::size_t size)throw(std::bad_alloc);
    static void operator delete(void* rawMemory, std::size_t size)throw();
    ...
};

void Base::operator delete(void* rawMemory, std::size_t size)throw(){
    if(rawMemory==nullptr)  return;
    if(size!=sizeof(Base)){
        ::operator delete(rawMemory);
        return;
    }
    归还raw Memory指向的内存;
    return;
}
```

---

# 请记住

- operator new应该包含一个无限循环，在其中尝试分配内存。如果无法满足内存需求，就应该调用new-handler。他也应该有能力处理0bytes申请，处理“比正确大小更大的申请”
- operator delete在收到nullptr指针不做任何事，class专属版本还应该处理“比正确大小更大的错误申请”