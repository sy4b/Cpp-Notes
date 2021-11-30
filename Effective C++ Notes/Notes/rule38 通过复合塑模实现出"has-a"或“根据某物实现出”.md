# rule38 通过复合塑模实现出"has-a"或“根据某物实现出”

复合是类型之间的一种关系，当某种类型的对象内含它种类型的对象，便是这种关系。

复合(composition)这个术语有许多同义词，包括layering(分层) containment(内含) aggregation(聚合) embedding(内嵌)

---

```cpp
class Address{...};
class PhoneNumber{...};
class Person{
public:
    ...
private:
    std::string name;
    Address address;
    PhoneNumber voiceNumber;
    PhoneNumber faxNumber;
};
```

本例之中Person对象由string Address PhoneNumber构成。

rule32指出，`public`继承带有is-a的意义。

复合也有他的意义：意味着has-a或is-implenmented-in-terms-of(根据某物实现出)。那是因为你正打算在你的软件中处理两个不同的领域。

程序中的对象其实相当于你塑造的世界中的某些食物，这样的对象属于应用域；其他对象则是实现细节上的人工制品，像是缓冲区、互斥器、查找数等等。这些对象相当于你软件的实现域，这些对象相当于你的软件的实现域

复合发生在应用域内的对象之间，表现出has-a的关系；发生于实现域内则表现出is-implemented-in-terms-of的关系

上述Person class示范has-a关系：Person有一个名称，一个地址，以及语音和传真两笔电话号码。你不会说“人是一个名称”而是说“人有一个名称”

比较麻烦的是区别is-a和is-implemented-in-terms-of这两种对象关系。假设你需要一个`template`，希望制造出一组classes用来表现不重复对象组成的集合。由于复用是件好事，你的第一个直觉就是采用STL提供的`set`

不幸的是`set`的实现往往招致“每个元素耗用三个指针”的额外开销，因为`set`通常以平衡查找树实现，使他们在查找、插入、移除元素时保证对数时间效率。如果你的程序空间比速度重要呢？这时候得自己写一个`template`

你决定在底层用链表实现，而STL有一个`list`，于是你决定复用它。你决定让你的Set template继承`std::list`

```cpp
template<typename T>
class Set:: public std::list<T>{...};
```

上述代码存在完全错误的东西，`public`继承下，`list`可以包含重复元素，而`Set`不可以。这两个`class`不该是is-a的关系

正确的做法是，Set对象可以根据一个`list`对象实现出来

```cpp
template<typename T>
class Set{
public:
    bool member(const T& item)const;
    void insert(const T& item);
    void remove(const T& item);
    std::size_t size()const;
private:
    std::list<T> rep;       // 用来表述Set的数据
};
```

Set成员函数可以大量依赖`list`及STL提供的机能来完成

```cpp
template<typename T>
bool Set<T>::member(const T& item)const{
    return std::find(rep.begin(), rep.end(), item)!=rep.end();
}

template<typename T>
void Set<T>::insert(const T& item){
    if(!member(item)){
        rep.push_back(item);
    }
}

template<typename T>
void Set<T>::remove(const T& item){
    typename std::list<T>::iterator it=std::find(rep.begin(),rep.end(), item);
    if(it!=rep.end()){
        rep.erase(it);
    }
}

template<typename T>
std::size_t Set<T>::size()const{
    return rep.size();
}
```

这些函数如此简单，适合成为inlining候选人，还需要考虑rule30

---

# 请记住

- 复合的意义和`public`继承完全不同
- 在应用域，复合意味着has-a
- 在实现域，复合意味着is-implemented-in-terms-of