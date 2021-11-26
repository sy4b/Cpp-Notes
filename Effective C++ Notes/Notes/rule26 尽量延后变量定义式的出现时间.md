# 尽量延后变量定义式的出现时间

只要你定义了一个变量，而其类型带有一个构造函数或析构函数，那么当程序控制流到达这个定义式时，你就得承受构造成本；当变量离开作用域时，你又得承受析构成本。即便你没有使用这个变量，你也需要付出这些代价。所以你应该尽可能避免这个情形

---

或许你会认为你不可能定义一个不使用的变量，但话不要说太早！

考虑以下函数，他计算通行密码的加密版本之后返回，前提是密码足够长。如果密码太短，函数会丢出一个异常，类型为`logic_error`（定义于C++标准程序库）

```cpp
std::string encryptRassword(const std::string& password){
    using namespace std;
    string ercrypted;
    if(password.length() < MinimumPasswordLength){
        throw logic_error("Password is too short");
    }
    ...
    return encryptyed;
}
```

`ercrypted`对象在此函数中并非完全未被使用，但如果有个异常被丢出，他就真的没有使用。

最好延后ercrypted的定义式直到真的需要他

```cpp
std::string encryptRassword(const std::string& password){
    using namespace std;
    if(password.length() < MinimumPasswordLength){
        throw logic_error("Password is too short");
    }
    string ercrypted;
    ...
    return encryptyed;
}
```

这段代码依旧有缺陷。`ercrypted`没有初始值，意味着调用default构造函数，然后赋值，rule4指出这样比直接在构造时制定初始值效率差。举个例子，假设省略的部分调用以下函数

```cpp
void encrypt(std::string& s);       // 在其中的适当地点对s加密
```

于是函数的实现如下，目前还不是最好的做法

```cpp
std::string encryptRassword(const std::string& password){
    using namespace std;
    if(password.length() < MinimumPasswordLength){
        throw logic_error("Password is too short");
    }
    string ercrypted;
    encrypted=password;
    encrypt(encrypt);
    return encryptyed;
}
```

更加好的做法是password座位encrypted的初始值，跳过毫无意义的default构造

```cpp
// 最佳做法
std::string encryptRassword(const std::string& password){
    using namespace std;
    if(password.length() < MinimumPasswordLength){
        throw logic_error("Password is too short");
    }
    std::string ercrypted(password);
    return encryptyed;
}
```

因此，该rule所谓的“尽可能延后”的真正含义，是指不仅仅要延后变量的定义知道必须使用的前一刻为止，更要尝试咽喉这份定义直到能给他初值实惨为止，这样可以避免构造和析构非必要对象，避免无意义的default构造行为

---

# 请记住

- 尽可能延后变量定义式的出现，这样做可以增加程序的清晰度并改善程序效率
