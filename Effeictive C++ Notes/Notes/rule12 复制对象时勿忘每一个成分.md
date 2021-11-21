# 复制对象时勿忘每一个成分

设计良好的面向对象系统会将对象的内部封装起来，只留下两个函数负责对象拷贝，即`copy`构造函数和`copy assignment`操作符，称为`copying`函数

编译器在必要时候会为我们的`class`创建`copying`函数，将被拷对象的所有成员变量都做份拷贝

---

声明自己的`copying`函数的话，如果实现代码中有必然出错的地方，编译器也不会告诉你

假设一个`class`用来表示顾客，`copying`函数使得外界对他们的调用会被记录下来

```cpp
void logCall(const std::string& funcName);// 制造一个log entry

class Customer{
public:
    ...
    Customer(const Customer& rhs);
    Customer& operator=(const Customer& rhs);
    ...
private:
    std::string name;
};

Customer::Customer(const Customer& rhs): name(rhs.name){
    logCall("Custmoer copy constructor");
}

Customer& Costomer::opertaor=(const Customer& rhs){
    logCall("Customer copy assignment operator");
    name=rhs.name;
    return *this;
}
```

以上的每一件事都很美好，直到另一个成员变量加入战局

```cpp
class Date{...};                // 日期
class Customer{
public:
    ...                         // 同前
private:
    std::string name;
    Date lastTransaction;
}
```

这时候已有的`copying`函数执行的是局部拷贝：它们的确复制了顾客的`name`但没有复制新添加的`lastTransaction`，。大多数编译器对此不做任何提醒

结论非常明显：如果为一个`class`添加一个成员变量，必须同时修改`copying`函数

---

一旦发生继承，可能导致最暗中肆虐的一个潜藏危机

```cpp
class PriorityCustomer: public Customer{
public:
    ...
    PriorityCustomer(const PriorityCustomer& rhs);
    PriorityCustomer& operator=(const PriorityCustomer& rhs);
    ...
private:
    int priority;
};

PriorityCustomer::PriorityCustomer(const PriorityCustomer& rhs): priority(rhs.priority){
    logCall("PriorityCustomer copy constructor");
}

PriorityCustomer& PriorityCustomer::operator=(const PriorityCustomer& rhs){
    logCall("PriorityCustomer copy assignment operator");
    priority=rhs.priority;
    return *this;
}
```

以上没有复制继承的成员变量，应该让derived class的`copying`函数调用相应的base class函数

```cpp
class PriorityCustomer: public Customer{
public:
    ...
    PriorityCustomer(const PriorityCustomer& rhs);
    PriorityCustomer& operator=(const PriorityCustomer& rhs);
    ...
private:
    int priority;
};

PriorityCustomer::PriorityCustomer(const PriorityCustomer& rhs): Customer(rhs), priority(rhs.priority){     // 调用copy构造函数
    logCall("PriorityCustomer copy constructor");
}

PriorityCustomer& PriorityCustomer::operator=(const PriorityCustomer& rhs){
    logCall("PriorityCustomer copy assignment operator");
    Customer::operator=(rhs);       // 对base class成分进行赋值动作
    priority=rhs.priority;
    return *this;
}
```

---

本条款所说的“复制每一个成分”至此已经很清晰。当编写一个`copying`函数，请确保
1. 复制所有local成员变量
2. 调用所有base classes内适当的copying函数

---

# 请记住

- `copying`函数应该确保复制对象内所有成员变量以及所有base class成分
- 不要尝试用某个`copying`函数实现另一个`copying`函数，应该将共同技能放入第三个函数中，并且由两个`copying`函数共同调用
