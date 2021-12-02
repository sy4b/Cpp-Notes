# 运用成员函数模板接受所有兼容类型

---

所谓智能指针是“行为像指针”的对象，并提供指针没有的技能，例如`std::auto_ptr`和`std::shared_ptr`可以在正确的时机自动删除heap-based资源

STL容器的迭代器几乎总是智能指针；无疑你不会奢望`++`将一个内置指针从链表的某个节点移到另一个节点，但可以在`list::iterator`身上办得到

内置指针支持隐式转换：derived class指针可以隐式转换为base class指针，指向non-const的指针可以转换为指向`const`的对象

```cpp
class Top{...};
class Middle: public Top{...};
class Bottom: public Middle{...};

Top* pt1=new Middle;        // Middle* - Top*
Top* pt2=new Bottom;        // Bottom* - Top*
const Top* pct2=pt1;        // Top* - const Top*
```

而想在用户自定智能指针中模拟上述转换有些麻烦，我们希望以下代码通过编译

```cpp
template<typename T>
class SmartPtr{
public:
    explicit SmartPtr(T* realPtr);  // 智能指针通常以内置指针完成初始化
    ...
};

SmartPtr<Top> pt1=SmartPtr<Middle> (new Middle);    // SmartPtr<Middle>转换为SmartPtr<Top>
SmartPtr<Top> pt1=SmartPtr<Bottom> (new Bottom);    // SmartPtr<Bottom>转换为SmartPtr<Top>
SmartPtr<const Top> pct2=pt1;    // SmartPtr<Top>转换为SmartPtr<const Top>
```

同一个`template`的不同具现体之间不存在固有关系，这里以带有base-derived关系的类来具现化一个`template`，具现体并不带有base-derived关系，编译器将其视为完全不同的类

---

为了获得我们希望的SmartPtr classes之间的转换能力，我们必须将他们明确的编写出来

首先必须编写智能指针的构造函数。关键在于我们永远无法写出我们需要的所有构造韩顺。例如继承体系有所扩充，那么就得继续添加...我们不希望需要频繁进行修改

就原理而言，这个例子中我们需要的构造函数没有止境，因为一个`template`可以被无限量具现化，生成无限量函数，因此我们需要为构造函数写一个构造模板

这样的构造模板称为member function templates，为`class`生成函数

```cpp
template<typename T>
class SmartPtr{
public:
    template<typename U>
    SmartPtr(const SmartPtr<U>& other);
    ...
};
```

对任何类型T和U，可以根据SmartPtr<U>生成一个SmartPtr<T>，两者是一个`template`的不同具现体，有时称这个copy构造函数为**泛化copy构造函数**

上述泛化copy构造函数没有声明为`explicit`，是为了与内置类型保持一致

此外，这个函数隐含：不能根据一个SmartPtr<Top>创建一个SmartPtr<Bottom> 这符合我们的想法

假设SmartPtr像`auto_ptr`一样提供get成员函数返回指针指针对象持有的**原始指针的副本**，那么我们可以在构造模板内实现代码中约束转换行为

```cpp
template<typename T>
class SmartPtr{
public:
    template<typename U>
    SmartPtr(const SmartPtr<U>& other): heldPtr(other.get()){...}
    T* get()const{return heldPtr;}
private:
    T* heldPtr;         // SmartPtr持有的内置指针
}
```

我们使用成员初始值列表来初始化`SmartPtr<T>`之内类型为`T*`的变量，并以`U*`类型的指针作为初始值，这个行为只有在“存在一个隐式转换可以讲一个`U*`指针转换为一个`T*`指针“时才行得通

成员函数模板的效果不限于构造函数，常扮演另一个角色是支持赋值操作，例如`shared_ptr`支持所有“来自兼容的内指针、`tr1::shared_ptr`、`auto_ptr`和`tr1::weak_ptr`”的构造行为以及（`tr1::weak_ptr`除外）的赋值操作

下面是TR1中冠以`tr1::shared_ptr`的一份摘录，其中强烈倾向声明`template`参数使用关键字`class`

```cpp
template<class T>
class shared_ptr{
public:
    template<class Y>
    explicit shared_ptr(Y* p);

    template<class Y>
    shared_ptr(share_ptr<Y> const& r);

    template<class Y>
    explicit shared_ptr(weak_ptr<Y> const& r);

    template<class Y>
    explicit shared_ptr(auto_ptr<Y>& r);

    template<class Y>
    shared_ptr& operator=(shared_ptr<Y> const& r);

    template<class Y>
    shared_ptr& operator=(auto_ptr<Y>& r);

    ...
}
```

以上除了泛化copy构造函数之外，所有构造函数都是`explicit`，意味着从某个`shared_ptr`类型隐式转换到另一个`shared_ptr`类型是允许的，但从某个内置类型或其他智能指针隐式转换则不被认可

此外`auto_ptr`没有被声明为`const`，因为他们复制时实际上被改动了

---

成员函数模板不改变语言基本规则。当我们声明一个泛化copy构造函数时，如果T和Y同类型，编译器会选择具现化模板还是自动生成copy构造函数呢？

此时编译器会为你暗自生成一个。如果你想要控制copy构造函数的方方面面，必须同时声明copy构造函数和泛化copy构造函数。相同的规则也适用于assignment操作

```cpp
template<typename class T>
class shared_ptr{
public:
    // copy构造
    shared_ptr(share_ptr const& r);
    // 泛化copy
    template<class Y>
    shared_ptr(shared_ptr<Y> const & r);

    // assignmet
    shared_ptr& operator=(shared_ptr const& r);
    // 泛化
    template<class Y>
    shared_ptr& operator=(shared_ptr<Y> const & r);

    ...
}
```

---

# 请记住

- 使用成员函数模板生成“可接受所有兼容类型”的函数
- 声明泛化copy函数后，还是需要声明正常的copy构造函数