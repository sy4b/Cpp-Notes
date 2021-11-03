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