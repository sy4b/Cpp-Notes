# 明智而慎重使用多重继承

本rule了解多重继承的两种观点：有人认为多重继承好，有人认为不好

---

最先需要认清的一件事是：当MI进入设计框架，程序有可能从一个以上的base classes继承相同名称（函数、typedef等等），那会导致较多的歧义

```cpp
class BorrowableItem{       // 图书馆允许借某些东西
public:
    void checkOut();
    ...
};

class ElectronicGadget{
private:
    bool checkOut()const;
    ...
};

class MP3Player: public BorrowableItem, public ElectronicGadget{...};

MP3Player mp;
mp.checkOut();              // 歧义，调用哪个checkOut
```

此例之中对checkOut的调用是歧义的，即使两个函数中只有一个可取用（一个`public`一个`private`）。这里两个checkOut有相同的匹配程度

```cpp
mp.BroowableItem::checkOut();       // 必须指明调用哪一个base class的checkOut
```

多重继承的意思是继承一个以上的base class，但这些base class并不常在继承体系中有更高级的base class，那会导致要命的“钻石型多重继承”

假设有这样一个继承体系

```cpp
class File{...};
class InputFile: public File{...};
class OutputFile: public File{...};
class IOFile: public InputFile, public OutputFile{...};
```

任何时候，如有一个继承体系中某个base class和derived class之间有一条以上的路线（上例File-InputFile-IOFile, File-OutputFile-IOFile）必须考虑是否让base class每一份变量都被derived class复制。这里IOFile对象应该这有一份base class复制

C++缺省做法是进行复制，想要抑制的话需要令带有数据的`class`成为一个virtual base class，需要让所有直接继承的`class`采用`virtual`继承

```cpp
class File{...};
class InputFile: virtual public File{...};
class OutputFile:virtual  public File{...};
class IOFile: public InputFile, public OutputFile{...};
```

使用`virtual`继承使得classes对象体积变大，访问速度更慢。此外，`virtual`base class的初始化责任由继承体系的最底层derived class负责，较之non-virtual base的情况复杂

建议非必要不使用virtual base；若一定要使用，尽可能避免在其中防止数据，这样就不用担心初始化和赋值

---

来看一下用于塑模“人”的Interface class

```cpp
class IPerson{
public:
    virtual ~IPerson();
    virtual std::string name()const=0;
    virtual std::string birthDate()const=0;
};
```

抽象基类无法实体化，使用工厂函数

```cpp
std::tr1::shared_ptr<IPerson> makePerson(DatabaseID personIdentifier);

DatabaseID askUserForDatabaseID();

DatabaseID id(askUserForDatabaseID());  // 获得一个id
std::tr1::shared_ptr<IPerson> pp(makePerson(id));   
```

makePerson需要借由IPerson的派生类创建对象并返回一个指针指向他，假设我们自己设计一个CPerson，并且从既有数据库相关class PersonInfo取得

```cpp
class PersonInfo{
public:
    explicit PersonInfo(DatabaseID pid);
    virtual ~PersonInfo();
    virtual const char* theName()const;
    virtual const char* theBirthDate()const;
    ...
private:
    virtual const char* valueDelimOpen()const;
    virtual const char* valueDelimClose()const;
    ...
};
```

PersonInfo被设计用来协助以各种格式打印数据库字段，起始和终止以特殊字符串为界，缺省采用`[]`，例如"Ring"被格式化为[Ring]，而valueDelimOpen()和valueDelimClose()用来允许derived class自定义界限符

```cpp
const char* PersonInfo::valueDelimOpen()const{
    return "[";
}

const char* PersonInfo::valueDelimClose()const{
    return "]";
}

const char* PersonInfo::theName()const{
    // 保留缓冲区给返回值 static自动初始化为全部是0
    static char value[Max_Formatted_Field_Value_Length];
    // 写入起始符号
    std::strcpy(value, valueDelimOpen());
    // value内的字符串写入IPerson name成员变量
    ...
    // 写入结尾符号
    std::strcat(value, valueDelimClose());

    return value;
}
```

（这里暂时不管实现中带来的线程问题和超限问题）valueDelimClose和valueDelimOpen都是`virtual`函数，所以theName返回的结果与derived class有关

此时IPerson文档要求name和birthDate函数返回未经装饰的值，所以需要重新定义valueDelimClose和valueDelimOpen函数。

CPerson和PersonInfo的关系是is-implemented-in-terms-of，可以用复合或`private`继承实现，这里因为需要重定义两个`virtual`函数，所以必须`private`继承或者复合+`public`继承。这里采用`private`继承

CPerson同时也必须实现IPerson接口，所以需要`public`继承IPerson

```cpp
class IPerson{
public:
    virtual ~IPerson();
    virtual std::string name()const=0;
    virtual std::string birthDate()const=0;
};

class DatabaseID{...};

class PersonInfo{
public:
    explicit PersonInfo(DatabaseID pid);
    virtual ~PersonInfo();
    virtual const char* theName()const;
    virtual const char* theBirthDate()const;
    virtual const char* valueDelimOpen()const;
    virtual const char* valueDelimClose()const;
    ...
};

// MI
class CPerson: public IPerson, private PersonInfo{
public:
    explicit CPerson(DatabaseID pid): PersonInfo(pid){}
    virtual std::string name()const{
        return PersonInfo::theName();
    }
    virtual std::string birthDate()const{
        return PersonInfo::theBirthDate();
    }
private:
    // 重新定义继承而来的virtual函数
    const char* valueDelimOpen()const{return "";}
    const char* valueDelimClose()const{return "";}
}
```

这个例子指出多重继承也有其合理用途

但如果有一个单一继承方案并且大约等价于一个多重继承方案，几乎可以确定前者更受欢迎。真的要使用MI也不要害怕，但必须明智而慎重使用

---

# 请记住

- MI比单一继承复杂，可能导致新的歧义以及对`virtual`继承的需要
- `virtual`继承会增加各方面成本；如果virtual base class不带任何数据，将是最具有实用价值的情况
- MI的确有正当用途，包括：`public`继承某个Interface class和`private`继承某个协助实现的class的组合