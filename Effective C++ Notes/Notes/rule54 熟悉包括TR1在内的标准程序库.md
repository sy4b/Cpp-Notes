# 熟悉包括TR1在内的标准程序库

TR1-Technical Report 1

回顾一下C++98列入的C++标准程序库有哪些主要成分

- STL，覆盖容器、迭代器、算法、函数对象】各种容器适配器和函数对象适配器
- Iostreams，覆盖用户自定缓冲功能，国际化IO，以及预先定义好的对象`cin cout cerr clog`
- 国际化支持，包括多区域能力(multiple active locales)，例如`wchar_t`和`wstring`等类型对促进Unicode有所帮助
- 数值处理：包括复数模板`complex`和纯数值模板`valarray`
- 异常阶层体系，包括base class exception及其derived class logic_error和runtime_error，以及更深继承的各个classes
-  C89标准程序库

---

TR1详细叙述了14个新组件，位于std名称空间的嵌套名称空间tr1内。本书包括

- 智能指针`tr1::shared_ptr`和`tr1::weak_ptr`。后者是为了解决环状`tr1::shared_ptr`的问题
- `tr1::function`，可以表示任何可调用物（函数或函数对象），只要其签名符合目标
    ```cpp
    void r(std::string func(int));  // 参数类型是一个函数，接受int类型返回string类型
    // 可以替换为
    void r(std::tr1::function<std::string(int)> func);
    ```
- `tr1::bind`，可以做STL绑定器`bind1st`和`bind2nd`能做的每一件事，鹅且更多。与前任的不同的是，他可以和`const`和non-const成员函数协同运作，可以和reference参数协调运作，不需要特殊协助就可以处理函数指针
- hash tables：`tr1::unordered_set``tr1::unordered_multiset``tr1::unordered_map``tr1::unordered_multimap`，这些容器内的元素无序
- 正则表达式
- tuples，元组，可持有任意个数的对象
- array，大小固定，不使用动态内存
- `tr1::mem_fn`
- `tr1::reference_wrapper`
- 随机数生成工具
- 数学特殊函数
- C99兼容扩充

以下组件由template编程技术构成
- type traits，提供类型的编译期信息
- `tr1::result_of`，推导函数调用的返回类型