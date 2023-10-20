
### 模版及特化

包含 `函数模版、类模版`； `类型模版、非类型模版`；
非类型模版参数必须在编译期就能确定结果，且非类型模版的参数不允许为浮点数、类对象、字符串.


```cpp
// (1) 类型模版
template<typename T1, typename T2> 
class Sam1 {
private:
    T1 m1;
    T2 m2;
};

// (2) 非类型模版
template <int32_t M, int32_t N>
int32_t cmp(const char (*str1)[M], const char(*str2)[N]) {
    ...
}
```

- 模版特化

```cpp

// 0, 基础版本
template<class T1, class T2>
class Data {
    T1 d1;
    T2 d2;
};

// 1, 全特化
template<>
class Data<int32_t, char> {
  int32_t d1;
  char    d2;
};

// 2, 偏特化第2个参数
template<class T1>
class Data<T1, char> {
    T1   d1;
    char d2;
};

// 3, 偏特化为引用类型
template<class T1, class T2>
class Data<T1&, T2&> {
    T1& d1;
    T2& d2;
};

// 4, 偏特化为指针类型
template<class T1, class T2>
class Data<T1*, T2*> {
    T1* d1;
    T2* d2;
};

int32_t m1 = 5;
char    m2 = '5';

Data<int32_t, int32_t> d0; // 0
Data<int32_t, char>    d1; // 1
Data<int64_t, char>    d2; // 2
Data<int32_t&, char&>  d3(m1, m2); // 3
Data<int32_t*, char*>  d4; // 4
    
```

```cpp
template <typename T>          int32_t compare(const T &l, const T& r);

// 模版重载
template <size_t N, size_t M>  int32_t compare(const char(&)[N], const char[&][M]);

// 函数模版特化
template<> int32_t compare<const char*>(const char* p1, const char* p2);
```

- 模版类型别名

```cpp
template<typename T>
using twin = std::pair<T, T>;

// template<typename T> typedef std::pair<T, T> twin; // error: a typedef cannot be a template
// 实例化
twin<std::string> var1;

//
template<typename T> using part = std::pair<T, int32_t>;
// template<typename T> typedef std::pair<T, int32_t> part; // error: a typedef cannot be a template
// 实例化
part<std::string> var2; // 等价于 std::pair<std::string, int32_t>
part<int64_t> var3;     // 等价于 std::pair<int64_t, int32_t>
```

- 模版类的static成员

每个模版类的实例，都拥有自己的`static成员变量` & `static成员函数`，而不是所有的模版类的实例共享. 

```cpp
template<typename T> class Foo {
pubilc:
    static T& value() { return val_; }
private:
    static T var_;
};

template<typename T>
T var_ = 0; // 定义并赋值
```



### 萃取

```cpp

class IntArray {
public:
    IntArray() {
        for (int32_t i = 0; i < 10; ++i) {
            arr_[i] = i + 1;
        }
    }

    int32_t get_sum(int32_t mul) {
        int32_t ret = 0;
        for (int32_t i = 0; i < 10; ++i) {
            ret += arr_[i];
        }
        return ret * mul;
    }

private:
    int32_t arr_[10];
};

class DoubleArray {
public:
    DoubleArray() {
        for (int32_t i = 0; i < 10; ++i) {
            arr_[i] = (i*2) + 2.7126;
        }
    }

    double get_sum(double mul) {
        double ret = 0;
        for (int32_t i = 0; i < 10; ++i) {
            ret += arr_[i];
        }
        return ret * mul;
    }

private:
    double arr_[10];
};


// 萃取，空的基础类
template<typename Ty>
class NumTraits {};

template<>
class NumTraits<IntArray> {
public:
    using return_type = int32_t; // 萃取 参数类型
    using arg_type    = int32_t; // 萃取 返回值类型
};

template<>
class NumTraits<DoubleArray> {
public:
    using return_type = double;
    using arg_type    = double;
};


template<typename Ty>
class Appl {
public:
    // 使用萃取的 参数类型 和 返回值 类型
    auto get_sum(Ty &obj, typename NumTraits<Ty>::arg_type mul) -> typename NumTraits<Ty>::return_type {
        return obj.get_sum(mul);
    }
};


//////////////
IntArray ia;
Appl<IntArray> obj1;
fprintf(stdout, "%d. \n", obj1.get_sum(ia, 2));

DoubleArray da;
Appl<DoubleArray> obj2;
fprintf(stdout, "%f. \n", obj2.get_sum(da, 2.3152));

```

默认情况下，C++假定通过"作用域运算符"访问的名字不是类型。若希望访问类型，则需要使用`typename`关键字来显示说明. 



### enable_if

```cpp

// 基础模版, 第一个参数必须为bool
template<bool _Test, typename _Ty = void>
struct enable_if {};

// 部分特化模版
// 即第一个参数为true时，使用该模版
// enable_if<true>，使用该模版(特化模版)，第2个参数使用基础模版的void.
// enable_if<false>，使用基础模版
// enable_if<true, char>, 使用该模版(特化模版)，第2个参数为char
// enable_if<false, char>, 使用基础模版，第2个参数为char

template<typename _Ty>
struct enable_if<true, _Ty> {
    using type = _Ty;
};

// 定义 enable_if_t 类型模版
template <bool _Test, typename _Ty = void>
using enable_if_t = typename enable_if<_Test, _Ty>::type;

```

```cpp

// b没有任何类型
// 编译报错, error: invalid type in declaration before ‘;’ token
enable_if_t<false, int32_t> a;

// b没有任何类型
// 编译报错, error: invalid type in declaration before ‘;’ token
enable_if_t<false> b;

// 等价于 int32_t c;
enable_if_t<true, int32_t> c;

// 等价于 void d; 
// 编译报错, error: invalid type in declaration before ‘;’ token
enable_if_t<true> d; 

```



// 模版需要两个参数
//   模版参数1，_Ty，类型
//   模版参数2，_Val，值
template<typename _Ty, _Ty _Val>
struct integral_constant {
    static constexpr _Ty value = _Val;
    using value_type = _Ty;               // 重定义内部类型
    using type       = integral_constant; // 重定义内部类型

    // 类型value_type重载
    constexpr operator value_type() const noexcept {
        return value;
    }
    // 对()进行重载
    constexpr value_type operator()() const noexcept {
        return value;
    }
};




递归

```cpp
void expand() {}

template<typename Ty, typename... Args>
void expand(Ty arg, Args... rest) {
    fprintf(stdout, "%d. \n", sizeof ...(rest)); // 参数个数
    //
    expand(rest...);
}
```


