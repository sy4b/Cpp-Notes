# Chapter 16 模板与泛型编程

模板是C++泛型编程的基础，一个模板就是一个创建类或函数的蓝图或者说公式

# 16.1 定义模板

## 16.1.1 函数模板

一个函数模板就是一个公式，可以用来生成针对特定类型的函数版本

```cpp
template <typename T>
int compare(const T &v1, const T &v2){
	if(v1<v2) return -1;
	if(v2<v1) return 1;
	return 0;
}
```

模板定义以关键字`template`开始，后跟一个**模板参数列表**，这是一个以逗号分隔的一个或多个**模板参数**的列表，用`<>`包围起来

模板参数表示在类或函数定义中用到的类型或值

### 实例化函数模板

当我们调用一个函数模板时，编译器通常用函数实参来为我们推断模板实参

```cpp
cout<<compare(1,0)<<endl; // T为int
```

编译器用推断出的模板参数来为我们**实例化**一个特定版本的函数。这些编译器生成的版本通常成为模板的实例

```cpp
// 实例化出 int compare(const int&, const int&)
cout<<compare(1,0)<<endl; // T为int
// 实例化出 int compare(const vector<int>&, const vector<int>&)
vector<int> vec1{1,2,3}, vec2{4,5,6};
cout<<compare(vec1,vec2)<<endl;
```

### 模板类型参数

模板类型参数前必须使用关键字`class`或`typename`

### 非类型模板参数

除了定义类型参数，还可以在模板中定义非类型参数。

非类型参数表示一个值而不是一个类型。通过特定的类型名来指定

当一个模板被实例化时，非类型参数被一个用户提供的或编译器推断的值代替，这些值必须是**常量表达式**

```cpp
template<unsigned N, unsigned M>
int compare(const char (&p1)[N], const char (&p2)[M]){
	return strcmp(p1,p2);
}
```

### inline和constexpr的函数模板

函数模板可以声明为`inline`或`constexpr`的

`inline`或`constexpr`说明符放在模板参数列表之后，返回类型之前

```cpp
template <typename T> inline T min(const T&, const T&);
```

### 编写类型无关的代码

编写泛型代码的两个重要原则：

- 模板中的函数参数是`const`的引用
- 函数体中的判断条件仅使用`<`比较运算

通过设置为`const`引用，可以保证函数可以用于不能拷贝的类型

### 模板编译

只有实例化出模板的一个特定版本时，编译器才会生成代码。这一特性影响了我们如何组织代码以及错误何时被检测到

为了生成一个实例化版本，编译器通常需要掌握函数模板或类模板成员函数的定义，因此模板的头文件既包括声明也包括定义

## 16.1.2 类模板

类模板用于生成类的蓝图。

与函数模板不同的是，编译器不能为类模板推断参数类型，必须在模板名的尖括号后增加额外信息——用来代替模板参数的模板实参列表

类似函数模板，类模板以关键字`template`开始，后跟模板参数列表

```cpp
template <typename T> class Blob{
private:
	std::shared_ptr<std::vector<T>> data;
	void check(size_type i, const std::string &msg) const;
public:
	typedef T value_type;
	typedef T typename std::vector<T>::size_type size_type;
	// 构造函数
	Blob();
	Blob(std::initializer_list<T> i1);
	// Blob中的元素数目
	size_type size()const{return data->size();}
	bool empty()const {return data->empty();}
	// 添加和删除元素
	void push_back(const T &t){data->push_back(t);}
	// 移动版本
	void push_back(T &&t){data->push_back(std::move(t));}
	void pop_back();
	// 元素访问
	T& back();
	T& operator[](size_type i);
};
```

### 实例化类模板

使用一个类模板时，必须提供额外信息，这些额外信息是**显式模板实参列表**，被绑定到模板参数

```cpp
Blob<string> names;
Blob<double> prices;
```

### 在模板作用域中引用模板类型

- 类模板的名字不是一个类型名

### 类模板的成员函数

- 与其他类相同，既可以在类模板内部也可以在类模板外部定义成员函数，并且定义在类模板内的成员被隐式声明为内联函数
- 定义在类模板之外的成员函数必须以`template`开始，后接模板参数列表
- 在类外部定义一个成员时，必须说明成员属于哪个类。

```cpp
template <typename T>
void Blob<T>::check(size_type i, const std::string& msg)const{
	if(i>=data->size())
		throw std::out_of_range(msg);
}
```

```cpp
template <typename T>
Blob<T>::Blob():data(std::make)shared<std::vector<T>>()){}
```

- 如果一个成员函数没有被使用，则它不会被实例化

### 在类代码内简化模板类名的使用

使用一个类模板类型时必须提供模板实参，但在类模板自己的作用域内，可以直接用模板名而不提供实参

### 类模板和友元

如果一个类模板包含一个非模板友元，则友元被授权可以访问所有模板实例

如果友元自身是模板，类可以授权给所有友元模板实例，也可以只授权给特定实例

### 一对一友好关系

```cpp
template<typename> class BlobPtr;
template<typename> class Blob;
template<typename T>
	bool operator==(const Blob<T>&, const Blob<T>&);
template<typename T> class Blob{
	friend class BlobPtr<T>;
	friend bool operator==<T>(const Blob<T>&, const Blob<T>&);
};
```

### 通用和特定的模板友好关系

一个类也可以将另一个模板的每个实例都声明为自己的友元，或者限定特定的实例为友元

```cpp
template<typename T> class Pal; // 前置声明，将模板的一个特定实例声明为友元时需要用到
class C{ // C是一个普通的非模板类
	// 用C实例化的Pal是C的一个友元
	friend class Pal<C>;
  // Pal的所有实例都是C的友元
	template<typename T> friend class Pal2;
};

template<typename T> class C2{ // C2是一个类模板
	friend class Pal<T>; // 每个实例都把相同实例化的Pal声明为友元，声明必须在作用域内
	template<typename X> friend class Pal2; // Pal2的所有实例都是C2所有实例的友元
	friend class Pal3; // 非模板类，是C2所有实例的友元
};
```

### 令模板自己的参数类型成为友元

新标准可以将模板类型参数声明为友元

```cpp
template<typename Type> class Bar{
	friend Type; // 将访问权限授予用来实例化Bar的类型
	//...
};
```

以上也可以用内置类型来实例化`Bar`

### 模板类型别名

可以定义一个`typedef`来引用实例化的类

```cpp
typedef Blob<string> StrBlob;

template<typename T> using twin=pair<T,T>;
twin<string> authors; // authors是一个pair<string,string>
```

### 类模板的static成员

```cpp
template<typename T> class Foo{
public:
	static std::size_t count(){return ctr;}
	...
private:
	static std::size_t ctr;
	...
};
```

每个`Foo`的实例都有自己的`static`成员实例。给定任意类型X，都有一个`Foo<X>::ctr`和`Foo<X>::count()`。所有`Foo<X>`类型的对象共享相同的`ctr`对象和`count`函数

每个`static`数据成员必须有且仅有一个定义，但是类模板的每个实例都有一个独特的`static`对象，因此定义数据成员时也定义为模板

```cpp
template<typename T>
size_t Foo<T>::ctr=0;
```

## 16.1.3 模板参数

类似函数参数的名字，一个模板的参数也没有什么内在含义，通常将类型参数命名为`T`，但实际上可以用任何名字

```cpp
template<typename Foo> Foo calc(const Foo& A, const Foo& b){
	Foo tmp=a;
	...
	return tmp;
}
```

### 模板参数和作用域

模板参数遵循普通的作用域规则。一个模板参数名的可用范围是声明之后，至模板声明或定义结束之前。与任何其他名字一样，模板参数会隐藏外层作用域中声明的相同名字。

在模板内不能重用模板参数名

```cpp
typedef double A;
template<typename A, typename B> void f(A a, B b){
	A tmp=a; // tmp的类型为A，而不是double
	double B; // 错误，重声明模板参数B
}

template<typename V, typename V> // 错误，不能重用
```

### 模板声明

模板声明必须包含模板参数

```cpp
template<typename T> int compare(const T&, const T&);
template<typename T> class Blob;
```

声明中的模板参数的名字不必与定义中相同

### 使用类的类型成员

我们使用作用域运算符`::`来访问`static`成员和类型成员

对于模板，假设T是一个模板类型参数，当编译器遇到`T::mem`这样的代码时，他不知道`mem`是一个类型还是一个`static`成员，直到实例化才会知道，但是为了处理模板，必须知道名字是否表示类型

```cpp
T::size_type *p; // 可能是p变量或者size_type*p
```

默认情况下C++假设通过作用运算符访问的名字不是类型，因此如果我们希望使用一个模板类型参数的类型成员，就必须使用关键字`typename`显式指出

```cpp
template<typename T>
typename T::value_type top(const T& c){
	if(!c.empty())
		return c.back();
	else
		return typename T::value_type();
}
// T::value_type是类型名
```

- tips：希望通知编译器一个名字表示类型时，必须使用关键字`typename`，而不能使用`class`

### 默认模板实参

在新标准中，我们可以为函数和类模板提供默认实参

```cpp
template<typename T, typename F=less<T>>
int compare(const T& v1 const T& v2, F f=F()){
	if(f(v1,v2)) return -1;
	if(f(v2,v1)) return 1;
	return 0;
}
```

与函数默认实参一样，对于一个函数模板，只有当他右侧的所有参数都有默认实参时，他才可以有默认实参

### 模板默认实参与类模板

无论何时使用一个类模板，都必须在模板名之后接上尖括号，尖括号指出类必须从一个模板实例化而来，尤其是一个类模板为所有模板参数都提供类默认实参

```cpp
template<class T=int> class Numbers{...};
Numbers<> average_precision;
```

## 16.1.4 成员模板

一个类可以包含本身是模板的成员函数，这种成员被称为成员模板，成员模板不能是虚函数

### 普通类的成员模板

```cpp
class DebugDelete{
public:
	DebugDelete(std::ostream &s=std::cerr):os(s){}
	template<typename T> void operator()(T *p)const
		{os<<"deleting unique_pter"<<std::endl; delete p;}
private:
	std::ostream &os;
};

// 可以用这个类代替delete
double *p=new double;
DebugDelete d;
d(p); // DebugDelete::operator()(double*)

// 可以用作unique_ptr的删除器
unique_ptr<int, DebugDelete> p(new int, DebugDelete());
unique_ptr<int, DebugDelete> sp(new string, DebugDelete());
```

### 类模板的成员模板

对于类模板也可以定义成员模板，此时类和成员各自有自己的、独立的模板参数

```cpp
template<typename T> class Blob{
	template<typename It> Blob(It b, It e); // 迭代器类型It
	...
};
// 在类模板外定义一个成员模板时，必须同时为类模板和成员模板提供模板参数列表，类模板的参数列表在前
template<typename T>
template<typename It>
Blob<T>::Blob(It b, It e): data(std::make_shared<std::vector<T>>(b,e)){})
```

### 实例化与成员模板

为了实例化一个类模板的成员模板，必须同时提供类和函数模板的实参

编译器通常根据传递给成员模板的函数实参来推断它的模板实参

## 16.1.5 控制实例化

模板在被使用时才会进行实例化。这一特性意味着相同的实例可能出现在多个对象文件中，当两个或多个独立编译的源文件使用了相同的模板并提供了相同的模板参数时，每个文件中都会有该模板的一个实例

以上特性可能导致严重额外开销。新标准下可以通过**显式实例化**来避免开销

一个显式实例化有如下形式：

```cpp
extern template declaration // 实例化声明
templare declaration // 实例化定义
```

`declaration`是一个类或函数声明，其中所有模板参数已被替换为模板实参

```cpp
extern template class Blob<string> // 声明
template int compare(const int&, const int&); // 定义
```

编译器遇到`extern`模板声明时，不会在本文件中生成实例化代码。将一个实例化声明为`extern`就表示在程序其他位置有该实例化的一个非`extern`声明

对于一个给定的实例化版本，可能有多个`extern`声明，但必须只有一个定义

### 实例化定义会实例化所有成员

一个类模板的实例化定义会实例化该模板的所有成员，包括内联的成员函数

## 16.1.6 效率与灵活性

# 16.2 模板实参推断

从函数实参来确定模板实参的过程被称为**模板实参推断**

## 16.2.1 类型转换与模板类型参数

与非模板函数一样，在调用时，传递给函数模板的实参被用来初始化函数的形参

如果一个函数形参的类型使用了模板参数类型，那么它采用特殊的初始化规则。只有很有限的几种类型转换会自动应用于这些实参。

编译器通常不是对实参进行类型转换，而是形成新的模板实例

与往常一样，顶层const无论是在形参还是实参中，都会被忽略。

在其他类型转换中，能在调用中应用于函数模板的包括如下两项

- `const`转换：可以将一个非`const`对象的引用（或指针）传递给一个`const`的引用（或指针）形参
- 数组或函数指针转换：如果函数形参不是引用类型，则可以对数组或函数类型的实参应用正常的指针转换。例如一个数组实参可以转换为一个指向首元素的指针。

其他类型转换，包括算数转换、派生类向基类的转换以及用户自定义的转换都不能应用于函数模板

```cpp
template<typename T> T fobj(T, T);
template<typename T> T fref(const T&, const T&);
string s1("a value");
const string s2("another value");

fobj(s1, s2); // const被忽略
fref(s1, s2); // s1转化为const是允许的

int a[10], b[42];
fobj(a, b); // fobj(int*, int*)
fref(a, b); // 错误 数组类型不匹配
```

### 使用相同模板参数类型的函数形参

一个模板类型参数可以用作多个函数形参的类型。由于只允许有限的类型转换，因此传递给函数模板形参的实参必须具有相同的类型

```cpp
long lng;
template<typename T> bool compare(T, T);
compare(lng, 1024); // error 1024是int
```

如果希望允许对函数实参进行正常的类型转换，可以将函数模板定义为两个类型参数，但必须兼容

```cpp
long lng;
template<typename T, typename V> bool compare(T, V);
compare(lng, 1024); //correct
```

### 正常类型转换应用于普通函数实参

函数模板可以应用普通类型定义的参数（即不涉及模板类型的类型），这种函数实参不进行特殊处理，他们正常转换为对应形参的类型

```cpp
template<typename T> ostream& print(ostream& os, const T& obj){
	return os<<obj;
}
```

## 16.2.2 函数模板显式实参

某些情况下，编译器无法推断出模板实参的类型。其他一些情况下也希望允许用户控制模板实例化。当函数返回类型与参数列表中任何类型都不相同时，这两种情况最常出现

### 指定显式模板实参

```cpp
template<typename T1, typename T2, typename T3>
T1 sum(T2, T3);
// T1的类型无法被推断
```

提供显式模板实参的方式与定义类模板实例的方式相同。显式模板实参在尖括号给出，位于函数名之后，实参列表之前

```cpp
auto val3=sum<long long>(i, lng); // long long sum(int, long)
```

显式模板实参按从左到右的顺序与对应模板参数匹配；只有尾部参数的显式模板实参才可以忽略，且前提是他们可以从函数参数中推断出来

```cpp
template<typename T1, typename T2, typename T3>
T3 alternative_sum(T2, T1);
auto val2=alternative_sum<long long, int , long>(i, lng)
```

### 正常类型转换应用于显式指定的实参

对于用普通类型定义的函数参数，允许进行正常的类型转换

对于模板类型参数已经显式指定的函数实参，同样也允许进行正常的类型转换

```cpp
long lng;
template<typename T> bool compare(T, T);
long lng;
compare(lng, 1024); // error
compare<long> (lng, 1024); // correct, compare(long, long)
compare<int> (lng, 1024); // correct, compare(int, int)
```

## 16.2.3 尾置返回类型与类型转换

希望用户确定返回类型时，用显式模板实参表示模板函数的返回类型有效，但可能会带来麻烦

例如编写一个函数接受表示序列的一对迭代器，返回序列中的一个元素的引用

```cpp
template<typename It>
??? &fcn(It beg, It end){
	// 处理序列
	return *beg;
}
```

此函数应该返回`*beg`，且可以用`decltype(*beg)`来获取类型，在编译器遇到参数列表之前，`beg`都不存在。因此必须使用尾置返回类型

```cpp
template<typename It>
auto fcn(It beg, It end)->decltype(*beg){
	// 处理序列
		return *beg;
}
```

解引用运算符将返回一个左值，因此`decltype`推断出的类型为`beg`表示的元素类型的引用

### 进行类型转换的标准库模板类

有时候无法直接获得所需要的类型，比如类似`fcn`的函数，但返回一个元素的值而非引用

为了获得元素类型，可以使用标准库的类型转换模板。这些模板定义在头文件`type_traits`

可以使用`remove_reference`模板来获得元素类型，它有一个模板类型参数和一个名为`type`的`public`类型成员。

例如我们实例化`remove_reference<int&>`，那么`type`成员将是`int`

```cpp
template<typename It>
auto fcn2(It beg, It end)->typename remove_reference<*beg>::type
{
	//..
	return *beg; // 返回序列中一个元素的拷贝
}
```

![Untitled](Chapter%2016%20%E6%A8%A1%E6%9D%BF%E4%B8%8E%E6%B3%9B%E5%9E%8B%E7%BC%96%E7%A8%8B%20f35afd85975c4de8a06bb79447ed4fd5/Untitled.png)

## 16.2.4 函数指针和实参推断

使用一个函数模板初始化一个函数指针或者为一个函数指针赋值时，编译器使用指针的类型来推断模板实参

```cpp
template<typename T> int compare(const T&, const T&);
// pf1指向实例int compare(const int&, const int&)
int(*pf1)(const int&, const int&)=compare;
```

再说

## 16.2.5 模板实参推断和引用

为了理解如何从函数调用进行类型推断，考虑下面例子：

```cpp
template<typename T> void f(T& p);
```

其中p是一个模板类型参数T的引用，需要记住两点：

- 编译器会应用正常的绑定规则
- `const`是底层的，不是顶层的

### 从左值引用函数参数推断类型

当一个函数参数是模板类型参数的一个左值引用时（形如`T&`），绑定规则告诉我们只能传递给他一个左值（如一个变量或一个返回引用类型的表达式）。实参可以是`const`，也可以不是，如果实参是`const`的，`T`将被推断为`const`类型

```cpp
template<typename T> void f1(T&);
f1(i); // i是一个int,模板参数是int
f1(ci); // ci是一个const int,模板参数是const int
f1(5); // 错误，必须是左值
```

### 从右值引用函数参数推断类型

```cpp
template<typename T> void f3(T&&);
f3(42); // 实参是一个int类型的右值；模板参数是int
```

### 引用折叠和右值引用参数

假设`i`是一个`int`对象，我们可能认为像`f3(i)`这样的调用不合法，毕竟i是一个左值，通常不能将一个右值引用绑定到一个左值上

但是C++在正常绑定规则之外定义了两个例外允许这种绑定。

- 将一个左值传递给函数的右值引用参数，且此右值引用指向模板类型参数时，编译器推断模板类型参数为实参的左值引用类型。因此调用`f3(i)`时，编译器推断T的类型为`int&`
- 通常不能直接定义一个引用的引用，但是通过类型别名或者模板参数类型间接定义是可以的：如果我们间接创建一个引用的引用，则这些引用形成了折叠。
    
    通常引用会折叠成一个普通的左值引用。新标准中，折叠规则扩展到右值引用。只有在一种特殊情况下引用会折叠晟右值引用：右值引用的右值引用。即
    

```cpp
// X& &, X& &&, X&& & 都会折叠为X&
// X&& &&折叠成X&&
```

- tips：引用折叠只能应用于间接创建的引用的引用

将以上两个规则结合起来，意味着我们可以对一个左值调用f3

```cpp
f3(i); // 模板参数为T为int& T&&折叠为int&
```

### 编写接受右值引用参数的模板函数

模板参数可以推断为一个引用类型，这一特性对模板内的代码可能有令人惊讶的影响

```cpp
template<typename T> void f3(T&& val){
	T t=val; // 拷贝还是绑定一个引用？
	t=fcn(t); // 赋值只改变t还是既改变t又改变val？
	if(val==t){...} // 如果T是引用类型，则一直为true
}
```

实际中右值引用常用于两种情况：模板转发其实参或模板被重载

目前应该注意的是，使用右值引用的函数模板通常使用重载

```cpp
template<typename T> void f(T&&); // 绑定到非const右值
template<typename T> void f(const T&); // 绑定到左值和const右值
```

## 16.2.6 理解std::move

虽然不能直接将一个右值引用绑定到一个左值上，但可以用`move`获得一个绑定到左值上的右值引用。

`move`是一个函数模板，本质上可以接受任何类型的实参

```cpp
template<typename T>
typename remove_reference<T>::type&& move(T&& t){
	return static_cast<typename remove_reference<T>::type&&>(t);
}
```

首先`move`的函数参数是一个指向模板类型参数的右值引用，通过引用折叠，此参数可以与任何类型的实参匹配。可以给左值，也可以给右值

```cpp
string s1("hi"), s2;
s2=std::move(string("bye!")); // 正确，从右值移动数据
s2=std::move(s1); // 正确，但在赋值之后s1的值不确定
```

### std::move是如何工作？

在`std::move(string("bye!"))`中

- 推断出`T`的类型是`string`
- `remove_reference`使用`string`实例化
- `typename remove_reference::type`是`string`
- `move`的返回类型是`string&&`
- `move`的函数参数`t`的类型是`string&&`

在`std::move(s1)`中：

- `T`的类型是`string&`
- `remove_reference`使用`string&`进行实例化
- `typename remove_reference::type`是`string`
- `move`的返回类型是`string&&`
- `move`的函数参数`t`实例化为`string& &&`，折叠为`string&`

因此这个调用实例化`string&& move(string &t)`

正是我们需要的将一个右值绑定到一个左值，此时`cast`将`t`转换为`string&&`

## 16.2.7 转发

某些函数需要将其一个或多个实参连同类型不变地转发给其他函数，此时我们需要保持被转发实参的所有性质，包括实参类型是否是`const`以及实参是左值还是右值

```cpp
template<typename F, typename T1, typename T2>
void flip1(F f, T1 t1, T2 t2){
	f(t2,t2);
}

void f(int v1, int& v2){
	cout<<v1<<" "<<++v2<<endl;
}

f(42,i); // f改变了实参i
flip1(f,j,42); // 通过flip1调用f不会改变j
```

### 定义能保持类型信息的函数参数

通过将函数参数定义为一个指向模板类型参数的右值引用，可以保持其对应实参的所有属性

```cpp
template<typename F, typename T1, typename T2>
void flip2(F f, T1 &&t1, T2 &&t2){
	f(t2,t1);
}
```

f

```cpp
void g(int &&i, int& j){
	cout<<i<<" "<<j<<endl;
}
```

如果试图通过`flip2`调用`g`，则参数`t2`被传递给g的右值引用参数，而`t2`是一个左值

### 在调用中使用std::forward保持类型信息

可以使用名为forward的新标准库设施来传递flip2的参数，他能保持原始实参的类型

`forward`定义在`utility`中，必须通过显式模板实参来调用。`forward`返回该显式实参类型的右值引用

```cpp
template<typename F, typename T1, typename T2>
void flip(F f, T1&& t1, T2&& t2){
	f(std::forward<T2>(t2), std::forward<T1>(t1));
}
```

- tips：对`std::forward`不使用`using`声明是一个好主意

# 16.3 重载与模板
