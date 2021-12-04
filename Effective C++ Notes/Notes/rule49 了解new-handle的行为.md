# 了解new-handle的行为

---

当`operator new`无法满足某一内存分配需求时，会抛出异常。某些老旧编译器会返回一个`null`指针，现在还是可以取得类似的行为（末尾讨论

当`operator new`抛出异常反映一个未满足的内存需求之前，会先调用一个客户指定的错误处理函数，称为new-handler（真正做的事情还更加复杂，见[rule51]()）

为了指定这个函数，客户需要调用`set_new_handler`，这是声明为`<new>`的一个标准程序库函数

```cpp
namespace std{
    typedef void (*new_handler)();
    new_handler set_new_handler(new_handler p) throw();
}
```

new_handler是一个`typedef`定义出一个指针指向函数，该函数不接受参数，不返回任何东西

set_new_handler接受一个new_handler，返回一个new_handler，末端`throw()`表示不抛出任何异常，是一份异常明细

set_new_handler的参数是一个指针，指向`operator new`无法分配足够内存时调用的函数；返回值指针指向被调用前正在执行的（马上将被替换的）new-handler函数

```cpp
void outOfMem(){
    std::cerr<<"Unable to satisfy request for memory\n";
    std::abort();
}

int main(){
    std::set_new_handler(outOfMem);
    int* pBigDataArray=new int[100000000L];
    ...
}
```

就本例而言，如果`operator new`无法为这么多个整数分配空间，outOfMem就会被调用，程序发出一个信息后abort

当`operator new`无法满足内存申请，他会不断调用new-handler函数，直到找到足够内存（引起反复调用代码显示于[rule51]()）。这描述出一个设计良好的new-handler函数必须完成以下事情：

- 让更多内存可以调用：实现该策略的做法之一：程序一开始就分配一大块内存，当new-handler第一次被调用，就将他们释放给程序使用
- 安装另一个new-handler：如果当前new-handler无法取得更多可用内存，或许他知道哪个另外的new-handler有能力分配内存，那么就可以安装另外那个new-handler替代自己（只需调用`set_new_handler`）。这么做，下次`operator new`调用new-handler时，调用的将是最新安装的那个（这个方案的变奏之一是：让new-handler修改自己的行为，下次被调用时就会做其他的事，做法之一是令new-handler修改“会影响new-handler行为”的static namespace 或global数据）
- 卸除new-handler：也就是将`nullptr`指针传给`set_new_handler`。没有安装任何new-handler时，`operator new`失败时会直接抛出异常
- 抛出`bad_alloc`的异常或派生自`bad_alloc`的异常，这样的异常不会被`operator new`捕获，会被传播到请求内存的位置
- 不返回：调用`abort`或`exit`

以上的选择让实现new-handler函数具有很大的弹性

---

有时候希望以不同方式处理内存分配失败的情况，视被分配物属于哪个`class`而定

```cpp
class X{
public:
    static void outOfMemory();
    ...
};

class Y{
public:
    static void outOfMemory();
    ...
};

X* p1=new X;    // 分配失败则调用X::outOfMemory
Y* p2=new Y;    // 分配失败则调用Y::outOfMemory
```

C++不支持`class`专属的new-handlers，也不需要。我们可以自己实现出这种行为，令每一个`class`提供自己的set_new_handler和`operator new`即可

---

现在打算处理Widget class内存分配失败情况

首先登陆“当`operator new`无法分配足够内存时”调用的函数

```cpp
class Widget{
public:
    static std::new_handler set_new_handler(std::new_handler p)throw();
    static void* operator new(std::size_t size)throw(std::bad_alloc);
private:
    static std::new_handler currentHandler;
};
```

`static`成员必须在`class`定义式之外被定义（除非他们是`const`整型

```cpp
std::new_handler Widget::currentHandler=0;
```

Widget内的`set_new_handler`函数会将获得的指针存储起来，返回在此调用之前存储的指针（同标准版

```cpp
std::new_handler Widget::set_new_handler(std::new_handler p)throw(){
    std:::new_handler oldHandler=currentHandler;
    currentHandler=p;
    return oldHandler;
}
```

最后Widget的`operator new`完成以下事情

1. 调用set_new_handler，告知Widget的错误处理函数，这会将Widget的new-handler安装为global new-handler
2. 调用global operator new，执行实际的内存分配。如果分配失败，global operator new会调用Widget的new-handler。如果最终无法分配足够内存，会抛出一个`bad_alloc`异常。此时Widget的`operator new`必须恢复原本的global new-handler，然后再传播该异常。为了确保原本的new-handler总是能被重新安装，运用资源管理对象防止资源泄露
3. 如果global operator new能够分配足够一个Widget对象所用的内存，operator new会返回一个指针，指向分配所得。Widget析构函数会管理global new-handler，自动将Widget operator new被调用前的那个global new-handler恢复

```cpp
// 资源管理类
class NewHandlerHolder{
public:
    explicit NewHandlerHolder(std::new_handler nh): handler(nh){}
    ~NewHandlerHolder(){
        std::set_new_handler(handler);
    }
    // 阻止拷贝
    NewHandlerHolder(const NewHandlerHolder&)=delete;   
    NewHandlerHolder& operator=(const NewHandlerHolder&)=delete;
private:
    std::new_handler handler;
};
```

接下来就可以实现Widget版本的operator new

```cpp
void* Widget::operator new(std::size_t size)throw(std::bad_alloc){
    NewHandlerHolder h(std::set_new_handler(currentHandler));
    return ::operator new(size);
}
```

Widget的客户应该如下使用其new-handling

```cpp
void outOfMem();            // 在分配失败时被调用

Widget::set_new_handler(outOfMem);

Widget* pw1=new Widget;

std::string* ps=new std::string;

Widget::set_new_handler(nullptr);

Widget* pw2=new Widget;
```

实现这一方案的代码不因class而不同，因此在这里使用复合是个合理的构想。一个简单的做法是建立一个mixin风格的base class，允许derived class继承单一特定能力。本例中是“设定class专属的new_handler的能力”，然后将这个base class转换为`template`，如此一来每个derivedclass豆浆获得实体互异的class data复件

```cpp
template<typename T>
class NewHandlerSupport{
public:
    static std::new_handler set_new_handler(std::new_handler p)throw();
    static void* operator new(std::size_t size)throw(std::bad_alloc);
    ...
private:
    static std::new_handler currentHandler;
};

template<typename T>
std::new_handler NewHandlerSupport<T>::set_new_handler(std::new_handler p)throw(){
    std::new_handler oldHandler=currentHandler;
    currentHandler=p;
    return oldhandler;
}

template<typename T>
void* NewHandlerSupport<T>::operator new(std::size_t size)throw(bad_alloc){
    NewHandlerHolder h(std::set_new_handler(currentHandler));
    return ::operator new(size);
}

template<typename T>
std::new_handler NewHandlerSupport<T>::currentHandler=nullptr;  // 将每一个currentHandler初始化为空
```

有了这个类模板之后，为Widget添加set_new_handler支持能力就轻而易举：只要令Widget继承自NewHandlerSupport<Widget>即可

```cpp
class Widget: public NewHandlerSupport<Widget>{
    ...
};
```

---

另一种形式`operator new`在分配内存失败时返回`nullptr`，使用nothrow对象（定义与头文件<new>中）

```cpp
class Widget{...};

// 分配失败则抛出bad_alloc
Widget* pw1=new Widget;
if(pw1==nullptr){           // false
    ...
}

// 分配失败返回nullptr
Widget* pw2=new(std::nothrow)Widget;
if(pw2==nullptr){           // 可能为true
    ...
}

```

---

# 请记住

- set_new_handler函数允许客户指定一个函数，在内存分配无法获得满足时被调用
- nothrow new局限性很大，只适用于内存分配，后续构造函数调用可能抛出异常