# Table of Contents

| [基础](#基础) | | | | |
|:------:|:------:|:------:|:------:|:------:|
| [数据类型的选择与转换](#数据类型的选择与转换) | [const与constexpr](#const与constexpr) | [typedef与using](#typedef与using) | [auto](#auto) | [decltype](#decltype) |
| [array](#array) | [lvalue与rvalue](#lvalue与rvalue) | [可变参数](#可变参数) | [bind](#bind) | [lambda](#lambda) |
| [enum](#enum) | [tuple](#tuple) | [函数指针与函数对象](#函数指针与函数对象) | [运行时类型识别](#运行时类型识别) | [内存分配与对齐](#内存分配与对齐)
| [auto_ptr](#auto_ptr) | [unique_ptr](#unique_ptr) | [shared_ptr](#shared_ptr) | [weak_ptr](#weak_ptr)


| [标准库](#标准库) | | | | |
|:------:|:------:|:------:|:------:|:------:|
| [容器](#容器) | [iterator](#iterator) | [vector](#vector) | [顺序容器](#顺序容器) | [multimap与multiset](#multimap与multiset) |
| [map](#map) | [bitset](#bitset) | [swap](#swap) | [insert与emplace与erase](#insert与emplace与erase) | [erase](#erase) |
| [function类型](#function类型) | [random](#random) | [regex](#regex) |
| [常用算法](#常用算法) |


| [类的设计](#类的设计) | | |
|:------:|:------:|:------:|
| [构造函数](#构造函数) | [显示调用构造函数和析构函数](#显示调用构造函数和析构函数) | [类型转换运算符](#类型转换运算符) |





## 基础

### 数据类型的选择与转换

- 如何选择数据类型

可以使用`size_t` `ssize_t` `int32_t` `uint32_t`类型；指针使用`nullptr`代替`NULL`

明确知道数据值不为负时，选择`unsigned`. <br/>
`char`由"编译器"决定是`signed char`还是`unsigned char`，所以不要将char放入运算表达式中，否则容易出问题. <br/>
在存放数据时才使用char/bool，在计算时不要使用char/bool。char的类型是未知的，可能为signed, 也可能为unsigned。若实在需要char类型参与计算，可以明确为signed char, 或unsigned char. <br/>
浮点数运算，选用double而不是float。因为float精度通常不够，而且在某些机器上，double的速度可能比float速度更快. <br/>
对浮点数用乘法和除法，结果会因为精度而不同。如: <br/>

```cpp
int64_t v = 3724141724135945;
double d1 = ((double)v) * 0.0001;
double d2 = ((double)v) /  10000;
fprintf(stdout, "%.6f ", d1); // 372414172413.594543
fprintf(stdout, "%.6f ", d2); // 372414172413.594482
```

- 类型转换

给`signed`变量赋值一个超出其表示范围的值时，结果是`未定义`的（可能继续工作/崩溃/垃圾数据）. <br/>
不要混用`带符号的变量`和`无符号的变量`:
> (a) 给`unsigned`类型赋值一个超过其范围的数字时，会进行截断。只保留其范围内的部分. <br/>
> (b) `signed`和`unsigned`相加时，都转化为`unsigned`。signed会隐式转会为unsigned. <br/>
把整数赋值给浮点数时，小数部分为0。同时要注意是否超过浮点数类型的容量，精度是否会损失. <br/> 
转义："\x"后跟的1个或多个16进制数值；"\"后跟8进制数字；如"\115"="\x4d". <br/>
显式类型转换 `cast-name<type> (expression)` <br/>
> (a) static_cast, 任何具有明确定义的类型转换，只要不包含底层const，都可以使用static_cast. 在编译阶段就确定. <br/>
> (b) const_cast, 去const，但只能改变运算对象的底层const. (注: 强行去const，并修改一个const变量的值，是设计的缺陷，可能不会成功，依赖编译器的实现. const_cast更多的是函数调用时，对const参数做适配(被调用函数内部实际上不修改参数的值)). <br/>
> (c) reinterpret_cast, <br/>
> (d) dynamic_cast, 在运行阶段进行转化. <br/>


- 算术类型转换

`整形提升`，计算时，把小整数类型转扩展成较大的整数类型. <br/>
`无符号类型`
> (a) 若都为`无符号类型`，则扩张成较大类型. <br/>
> (b) 若`无符号类型`不小于`带符号类型`，则带符号的会先转成无符号(注意扩展的副作用). <br/>
> (c) 若`无符号类型`小于`带符号类型`，行为不定，依赖机器. <br/>


### const与constexpr

- const变量

默认情况下，const对象仅在文件内有效。当多个文件中出现了同名的const变量，等同于在不同的文件中分别定义了独立的变量. <br/>
为了避免这个问题，可以在声明或定义的时候，加`extern`
```cpp
extern const int32_t g_size = f(); // file.cpp中声明
extern const int32_t g_size; // file.h中定义
```

- 指针与const

建议使用nullptr，不要使用NULL。所有指针在使用之前，一定要初始化. 指向常量的指针(pointer to const)
```cpp
double d1 = 3.14, d2 = 2.24;
const double *pval = &d1; // const修饰*pval. low-level const
*pval = 3.2; // error，（1）不能通过指针修改所指对象的值
pval = &d2;  // ok，（2）修改指针所指向的地址
```

常量指针(const pointer)，在定义的时候必须被初始化，不能再指向其它地址
```cpp
double d1 = 3.14, d2 = 2.24;
double * const pval = &d1; // const修饰pval. top-level const
*pval = 3.2; // ok,
pval = &d2;  // error,
```

- constexpr(常量表达式)

`常量表达式`是指值不会改变，且在`编译阶段`就须得到计算结果的表达式(非运行阶段).
```cpp
const int32_t a = 5;
const int32_t b = a + 1; // a，b均为常量表达式
const int32_t c = current_time(); // c需要在运行中才能得到结果，非常量表达式
```

C++11允许将变量声明为`constexpr`类型，这样编译器在 `编译阶段` 就检查该值是否为常量表达式.
```cpp
constexpr int32_t a = 5;      // 5 是常量表达式
constexpr int32_t b = a + 1;  // a+1是常量表达式
constexpr int32_t c = size(); // 要求size()是一个constexpr函数时，才可以通过编译
```

在`constexpr`声明中如果定义了一个指针，则`constexpr`仅对指针有效，与指针所指的对象无关.
```cpp
int32_t i = 5;
// p1为指向常量的指针，不能通过p1来修改对象的值，但p1可以再指向其它地址
const int32_t *p1 = &i;
// p2为指向整数的常量指针，p2不能再指向其它地方。
// 同时，要求i的地址在编译阶段就要确定，即i是一个全局变量，或者静态局部变量
constexpr int32_t *p2 = &i;
```

- constexpr函数

```cpp
constexpr int32_t size(int32_t cnt) {
    return cnt * 4;
}

int32_t arr1[size(5)]; // ok

int32_t i = 5;
int32_t arr2[size(i)]; // error
```
constexpr的返回值不一定是常量表达式; constexpr函数会被编译器内联


### typedef与using

```cpp
typedef double Num;

typedef Num base, *Pointer; // base, Num都是double的同义词； Pointer 是double*的同义词

// typedef 与 const一起使用
char c = 'A';
typdef char *pstring;

// const 修饰cstr，说明cstr的值不能被改变. 同时cstr是一个指向char的指针
const pstring cstr = &c; // 等价于 char* const cstr = &c; 而不是等价于 const char* cstr;

// const 修饰*ps。ps是指针，不能通过ps来修改所指地址中的值，该地址中的值也是一个指针
const pstring *ps;  // 等价于 char* const *ps;
```

```cpp
using Num = int32_t; // Num是int32_t的同义词
Num c = 5;
```

```cpp
using ARR1=char[1024];
typedef char ARR2[1024];

std::cout << "output:" << sizeof(ARR1) << ", " << sizeof(ARR2) << std::endl; // output: 1024, 1024
// 二维数组
ARR1 arr1[32];
ARR2 arr2[32];
```


### auto

```cpp
// (1) 自动推断类型
{
    auto item = V1 + V2;   // 根据V1+V2的结果，来决定item的类型
}
// (2) auto推断的类型要一致
{
    auto i = 0, *p = &i;   // ok. same as int32_t i = 0, int32_t *p = &i;
    auto j = 0, pi = 3.14; // error. j 和pi的类型不同
}
// (3) auto会忽略引用和顶层的const; 但若带上'&'，则不会忽略;
{
    int32_t m = 5, &n = m;
    auto  k1 = n; // 等价于int32_t  k1 = 5; 去引用
    auto &k2 = n; // 等价于int32_t &k2 = n;

    const int32_t i = 100;
    auto  j = i; // same as int32_t j = i; 去const
    auto &k = i; // same as const int32_t &k = i;
}
// (4) auto会退化成指向数组的指针
{
    int32_t a[5];
    auto  j = a; // same as int32_t *p = a;

    auto &k = a; // same as int32_t (*p)[5] = a;
}

template<typename M, typename N>
void Multi(M t, N u) {
    auto r = t * u;
}

// (2) 返回值占位
template<typename T1, typename T2>
auto compose(T1 t1, T2 t2) -> decltype(t1+t2) {
    return t1 + t2;
}

```

C++11引入auto，有两个作用: `自动类型推断`和`返回值占位`. <br/>
在"编译阶段"由编译器根据初始值来推导类型，因此，auto定义的变量必须有初始值. <br/>
可以使用`valatile` `pointer(*)` `reference(&)` `rvalue reference(&&)`来修饰auto. <br/>
`auto`不能自动推导成CV-qualifiers(constant & volatile qualifiers), 除非被声明为引用类型. <br/>
如果初始化表达式是引用，则去除引用语义. <br/>
如果初始化表达式是const或volatie(或两者都有)，则去除const/volatile语义 (auto会去掉顶层const，保留底层const). <br/> 
如果auto关键字上带&，则不去除const语义. <br/>
函数的模版参数不能被声明为auto. <br/>
auto会被退化成指向数组的指针，除非被声明为引用. <br/>


### decltype

decltype的作用是选择并返回操作数的数据类型。在编译阶段分析表达式并得到其类型，并不去计算表达式的值.

```cpp
const int32_t i = 0, &j = i, *p = &i;
decltype(i) x = 0; // 等价于 const int32_t x = 0;
decltype(j) y = x; // 等价于 const int32_t &y = x; decltype(j)是引用类型
decltype(j) z;     // 等价于 const int32_t &z; error, 引用必须初始化
decltype(*p) m;    // 等价于 const int32_t &m; error, 解引用
```
decltype使用的是表达式的话，需要根据表达式的结果来推测对应的类型. <br/>
decltype和auto不同，不会忽略引用和顶层的const. <br/>
decltype使用的表达式是解引用`decltype(*p)`，则decltype得到引用类型. <br/>
decltype使用的表达式加括号与不加括号意义不同，加括号就是一个表达式，就会变成引用. <br/>
```cpp
int32_t i = 0;
decltype(i)   x; // ok.
decltype((i)) y; // error. 两层括号，变成引用，等价于 int32_t &y;
```

赋值是会产生引用的一类表达式，引用的类型就是左值的类型。
```cpp
int32_t i = 5, j = 10;
decltype(i)   a = i; // int32_t  a = i;
decltype(i=j) b = a; // int32_t &b = a; 表达式i=j的类型是 int32_t&
```


### array

```cpp
int32_t iarr[] = {0, 1, 2, 3, 4};
int32_t    *p1 = iarr; // 等价于 int32_t *p1 = &(iarr[0]);

auto     p2(iarr); // 等价与 int32_t *p2 = &(iarr[0]); auto p2(&(iarr[0]));
decltype(iarr) p3; // 等价于 int32_t p3[5];
```

在很多用到数组名字的地方，编译器会自动地把名字替换为一个指向数组首元素的指针. <br/>
数组名作为一个auto变量的初始值时，推断出来的类型是指针而非数组. <br/>
数组名作为decltype的表达式时，返回的类型是数组. 


- 复杂数组声明
```cpp
int32_t arr[10];

int32_t    *p[10]; // p的长度为10， 每个元素都存放指针（int32_t*）
int32_t    &r[10]; // error，不存在引用的数组
int32_t  (*p)[10]; // p为指针，指向一个数组，数组的长度为10，数组中每个元素类型为int32_t
int32_t  (&r)[10] = arr; // r是数组的引用
int32_t *(&r)[10] = p;
```
```cpp
int arr[2][3] = {{1,2,3}, {4,5,6}};
decltype(arr) p1; // 等价于 int(*)[2][3] p1;
auto p2  = arr; // 等价于 int(*)[3] p2 = &(arr[0]);
auto &p3 = arr; // int32(&)[2][3] p3 = arr;
```

- 使用标准库的begin() end() 来遍历数组
```cpp
int arr[100];
// int* pBeg = begin(arr), *pEnd = end(arr);
for (int* pBeg = begin(arr); pBeg != end(arr); ++pBeg) {
    std::cout << *pBeg;
}
```

- 返回数组指针的写法
```cpp
// 第一种写法
using arrT = int32_t[10]; // or typedef int32_t arrT[10]; 里面存放10个int32_t类型数据
arrT* func(int32_t args); // 返回一个指针，指向int32_t [10] 的数组

// 第二种
int32_t (*func(int32_t args)) [10] {

}

// 第三种
// C++11中的新写法，使用尾置返回类型
auto func(int32_t args) -> int(*)[10] {

}
```


### lvalue与rvalue

C++11中所有的值(表达式)必属于左值/右值之一; <br/>
当一个对象被当作`右值`的时候，用的是对象的值（内容）；当对象被用作`左值`的时候，用的是对象的身份（在内存中的位置）；<br/>
可以取地址的且有名字的就是`左值`; 不能取地址的、没有名字的就是`右值`; <br/>
C++11中的右值，又分为`纯右值`(prvalue, Pure Rvalue) 和 `将亡值`: <br/>
> `纯右值`指临时变量和不跟对象关联的字面量值；<br/>
> `将亡值`是C++11新增的跟右值引用相关的表达式，这样的表达式通常是将要被移动的对象（移为他用），比如返回右值引用T&&的函数返回值、std::move的返回值、转换为T&&的类型转换函数的返回值; <br/>
> 将亡值可以理解为通过“盗取”其它变量内存空间的方式获取到的值。在确保其它变量不再被使用、或即将被销毁时，通过“盗取”的方式可以避免内存空间的释放和分配，延长变量的生命期. <br/>

编码过程中，发现有些对象在赋值给其它对象后，会被立即销毁。这种情况下，移动而非拷贝对象，将获得性能的提升. <br/>
`右值引用`的一个重要特性是 只能绑定到一个将要销毁的对象。因此，可以自由的将一个右值的资源"移动"到另一个对象中. <br/>
`左值`有持久的状态, `右值`要么是字面常量，要么是在表达式求值的过程中创建的临时对象. `右值引用`只能绑定到临时对象(所引用的对象即将销毁、该对象没有其它用户). 右值引用的代码可以自由接管所引用的对象的资源. <br/>


### 可变参数

- 省略符形参

```cpp
#include <cstdarg>

int32_t add(int32_t cnt, ...) {
    int32_t ret = 0;

    // va_list的定义为 typdef  char *va_list; 用于持有可变参数
    va_list args;

    // 对args赋值，使其指向可变参数列表的第一个参数
    va_start(args, cnt);

    for (int32_t i = 0; i < cnt; ++i) {
        // 第2个参数指定其返回类型
        // 每次调用va_arg，会获取当前参数，并自动指向下一个可变参数
        ret += va_arg(args, int32_t);        
    }

    // 释放va_list变量
    va_end(args);

    return ret;
}

std::cout << add(5, 10, 10, 20, 30, 50) << std::endl;
```

编译器入栈的顺序为从右到左，所以上面入栈的顺序为: 50, 30, 20, 10, 10, 5. 所以`va_arg`在知道第一个参数的地址和类型后，就可以计算了


- initializer_list

```cpp
#include <initializer_list>

int32_t add(initializer_list<int32_t> lst) {
    int32_t ret = 0;
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        ret += *it;
    }

    return ret;
}

std::cout << add({10, 10, 20, 30, 50}) << std::endl;
```

`initializer_list`是标准库类型(类模版)，用于表示某种特定类型的值的数组. 要求所有的可变参数类型相同; `initializer_list`中的元素都是常量值，无法改变其元素的值


- 可变参数模版

可变参数的类型可以不同, `template <typename T, typename ... Args> void foo(const T &t, const Args &... rest);`
`Args`是模版参数包，`rest`是函数参数包. 

可变参数模版，通常是递归的，所以还需要定义一个`非可变参数的模版`


```cpp
template <typename T>
void pnt(const T &t) {
    std::cout << t << std::endl;
}

template <typename T, typename ... Args>
void pnt(const T &t, const Args &... rest) {
    std::cout << t << std::endl;
    pnt(rest...); // 递归调用
}

pnt(10, 20, 30, 50);
```


### bind

bind返回可调用的函数对象，头文件`#include <functional>`, std::bind有2种原型:

```cpp
template<class F, class... Args>
/*unspecified*/ bind(F&& f, Args&&... args);

template<class R, class F, class... Args>
/*unspecified*/ bind(F&& f, Args&&... args);
```

`auto newfunc = bind(&要调用的函数f, &对象, f的参数1， f的参数2, f的参数3, ..., f的最后一个参数);` <br/>
如果定义成这样: `auto newfunc = bind(&要调用的函数f, &对象, 123， "abc", std::placeholders::_2, std::placeholders::_1);`，则表示f的第一个参数固定填写为123，
f的第2个参数固定为"abc"，f的第3个参数为newfunc的第2个参数，f的第4个参数为newfunc的第1个参数.

`std::ref`返回引用， `std::cref`返回`const`引用

- (1) bind全局函数

```cpp
// f可以为静态函数或非静态函数
int32_t f(char a, uint32_t b, const char *c, double d) {
    std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;
    return b;
}

// bind返回可调用的函数对象, _1, _2分别为func1的第一个、第二个参数
std::function<int32_t(double, const char*)> func1 = bind(f, 'a', 15, std::placeholders::_2, std::placeholders::_1);
std::cout << func1(13.15, "13.15") << std::endl;



std::string str;
int32_t v = 5;

// lambda形式
for_each(str.begin(), str.end(), [&os, v](const std::string& s) {
    os << s << " " << v << std::endl; 
});

// 等价于
std::ostream & print(std::ostream &os, const std::string &s, int32_t v) {
    os << s << " " << v << std::endl;
}
for_each(str.begin(), str.end(), std::bind(print, ref(os), std::placeholders::_1, v));

```

- (2) bind类的成员非静态函数
```cpp
class Func {
public:
    int32_t f(char a, uint32_t b, const char *c, double d) {
        std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;
        return b;
    }
};

Func func;
// bind返回可调用的函数对象, _1, _2分别为func1的第一个、第二个参数
std::function<int32_t(double, const char*)> func1 = bind(&Func::f, &func, 'a', 13, std::placeholders::_2, std::placeholders::_1);
std::cout << func1(13.15, "13.15") << std::endl;

```

- (3) bind类的成员静态函数
```cpp
class Func {
public:
    static
    int32_t f(char a, uint32_t b, const char *c, double d) {
        std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;
        return b;
    }
};

Func func;
// bind返回可调用的函数对象, _1, _2分别为func1的第一个、第二个参数
std::function<int32_t(double, const char*)> func1 = bind(&Func::f, 'a', 13, std::placeholders::_2, std::placeholders::_1);
std::cout << func1(13.15, "13.15") << std::endl;

```

- (4) bind启动新线程

```cpp
#include <thread>

// bind成员函数
class CTest {
public:
    void test(int32_t arg1, const char *arg2, std::string &arg3) {
        // ...
    }
};

// 如何调用
CTest test;
std::string str;
std::thread th1(std::bind(&CTest::test, &test, 3, "abctest", std::ref(str)));
th1.detach();

// 或者
std::thread *th2  = new std::thread(std::bind(&CTest::test, &test, 3, "abctest", std::ref(str)));
th2->join();

```

### lambda

表达式原型: `[capture list] (parameter list) -> return type { function body; }`

lambda的`捕获列表`是一个lambda所在函数中定义的局部变量的列表（通常为空，表示不使用局部变量）. lambda`必须`使用尾置返回来指定返回类型，但可以忽略`参数列表`和`返回类型`.

`值捕获`，被捕获的变量的值，是lambda创建时的拷贝，因此，随后对值的修改，不会影响lambda内对应的值. `引用捕获`，当使用引用捕获时，要确保lambda执行期间，值是存在的. 可以从函数返回lambda.

```cpp
int32_t v1 = 42, v2 = 45;

auto func1 = [v1]() { return v1; };
v1 = 12;
std::cout << func1() << std::endl; // 42, 在lambda创建的时候，就保存了v1的值

auto func2 = [&v2]() { return v2; };
v2 = 12;
std::cout << func2() << std::endl; // 12
```

`隐式捕获`，让编译器自己去推断捕获那些变量.

```cpp
it32_t v1 = 42, v2 = 45;
auto f1 = [=]() { return v1; }; // 隐式捕获，采用值的方式来捕获
auto f2 = [&]() { return v2; }; // 隐式捕获，采用引用的方式来捕获
```

一个启动线程的例子

```cpp

int32_t a = 10;
std::vector<std::string> vec{"ab", "ba", "ccc"};

// 引用 vec.
std::thread th([a, &vec](){
    fprintf(stdout, "thread start. \n");

    fprintf(stdout, "%d \n", a);
    for (auto &it : vec) {
        fprintf(stdout, "%s \n", it.c_str());
    }

    fprintf(stdout, "thread end. \n");
});

```

### enum

```cpp
enum       Color1 { red, blue, yellow };
enum class Color2 { Red, Blue, Yellow }; // 限定作用域

Color1 a1 = red;         // ok
Color1 a2 = Color2::Red; // error
Color2 b1 = red;         // error
Color2 b2 = Color2::Red; // ok
```
```cpp
enum class eNum : int32_t { Num1 = 5, Num2 = 10, Num3 = 15, Num4 = 20, };

int32_t a = eNum::Num2; // compile error.
int32_t b = static_cast<int32_t>(eNum::Num2); // ok
```

枚举属于`字面值常量`类型，C++11加入`限定作用域的枚举类型`. <br/>
不限定作用域的声明: `enum Name { ... }` <br/>
限定作用域的声明: `enum class Name { ... };` or `enum struct { ... };`，多了`class` or `struct` <br/>
一个不限定作用域的枚举类型的对象(或枚举成员)，自动的转化为整型. <br/>
限定作用域的enum，不能“隐式”的转换为整形. <br/>


### tuple

tuple是元素个数不定的模版，`#include <tuple>`，原型为`tuple<T1, T2, ..., Tn>`，元素的类型可以不同，个数不定. 使用`make_tuple`生成对象.

```cpp
tuple<int32_t, const char*, double> tst(10, "aaa", 5.2);
tst = make_tuple(15, "test_15", 3.14); // 使用make_tuple生成对象
int32_t   first = get<0>(tst); // get<0>是第一个成员
const char* sec = get<1>(tst);
double    third = get<2>(tst);

// sz=3, 使用tuple_size<tupleType>::value返回元素个数
size_t sz = tuple_size<decltype(tst)>::value;

// tuple_element<i, tupleType>::type 返回第i个元素的类型
// p 为 const char*
tuple_element<2, decltype(tst)>::type p = get<1>(tst); 
```

两个`tuple`相等的条件是: 元素个数相等 且 每个元素内容也相等.


### 函数指针与函数对象

- 函数指针(全局函数或全局静态函数)

函数位于代码段中，是一段内存数据。`函数指针`是指向这块内存的首地址。
可以使用`using`或`typedef`来定义`函数指针`类型。

```cpp
typedef int32_t (*FuncPointer1)(char a, uint32_t b, const char *c, double d);
using FuncPointer2 = int32_t(*)(char a, uint32_t b, const char *c, double d);

int32_t f(char a, uint32_t b, const char *c, double d) {
    std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;
    return b;
}

void test() {
    FuncPointer1 fp1 = f;
    FuncPointer2 fp2 = f;
    
    std::cout << fp1('a', 1, "func", 15.12) << std::endl; // output: 1
    std::cout << fp2('a', 2, "func", 15.12) << std::endl; // output: 2
    
    std::cout << typeid(FuncPointer1).name() << std::endl; // output: PFicjPKcdE
    std::cout << typeid(FuncPointer2).name() << std::endl; // output: PFicjPKcdE
}
```

- 函数指针(类的普通成员函数)

```cpp
class Exam {
    // 声明类的成员函数指针, must use "Exam::*"
    using FUNC = int32_t(Exam::*)(int32_t, int32_t);

public:
    Exam(bool v) {
        if (v) {
            func_ = &Exam::add; // 获得该函数在内存中的实际地址
        }
        else {
            func_ = &Exam::sub; // 获得该函数在内存中的实际地址
        }
    }

    // 必须使用"->*"进行调用
    int32_t operator()(int32_t a, int32_t b) { return (this->*func_)(a, b); }

private:
    int32_t add(int32_t a, int32_t b) { return a + b; }
    int32_t sub(int32_t a, int32_t b) { return a - b; }

private:
    FUNC func_ = nullptr;
};


// 使用示例
Exam e1(true), e2(false);
std::cout << e1(15, 10) << std::endl;
std::cout << e2(15, 10) << std::endl;

```

- 函数指针(类的静态成员函数)

```cpp
class Exam {
    // 声明 普通函数指针
    using FUNC = int32_t(*)(int32_t, int32_t);

public:
    Exam(bool v) {
        if (v) {
            func_ = &Exam::add; // 获得该函数在内存中的实际地址
        }
        else {
            func_ = &Exam::sub; // 获得该函数在内存中的实际地址
        }
    }
    
    // 不能使用"->*"进行调用
    int32_t operator()(int32_t a, int32_t b) { return func_(a, b); }

private:
    static int32_t add(int32_t a, int32_t b) { return a + b; }
    static int32_t sub(int32_t a, int32_t b) { return a - b; }

private:
    FUNC func_ = nullptr;
};


// 使用示例
Exam e1(true), e2(false);
std::cout << e1(15, 10) << std::endl;
std::cout << e2(15, 10) << std::endl;
```


- 函数指针(类的virtual成员函数)

```cpp
class ExamA {
    // 声明类的成员函数指针, must use "Exam::*"
    using FUNC = int32_t(ExamA::*)(int32_t, int32_t);

public:
    virtual ~ExamA() { }
    ExamA(bool v) {
        if (v) {
            func_ = &ExamA::add; // 得到的是虚表的偏移位置，只有在运行时，才能得到准确的函数地址
        }
        else {
            func_ = &ExamA::sub; // 得到的是虚表的偏移位置，只有在运行时，才能得到准确的函数地址
        }
    }

    int32_t operator()(int32_t a, int32_t b) { return (this->*func_)(a, b); }

private:
    virtual int32_t add(int32_t a, int32_t b) { return a + b; }
    virtual int32_t sub(int32_t a, int32_t b) { return a - b; }

private:
    FUNC func_ = nullptr;
};

class ExamB : public ExamA {
    // 声明类的成员函数指针, must use "Exam::*"
    using FUNC = int32_t(ExamB::*)(int32_t, int32_t);

public:
    virtual ~ExamB() { }
    ExamB(bool v) : ExamA(v) {
        if (v) {
            func_ = &ExamB::add; // 得到的是虚表的偏移位置，只有在运行时，才能得到准确的函数地址
        }
        else {
            func_ = &ExamB::sub; // 得到的是虚表的偏移位置，只有在运行时，才能得到准确的函数地址
        }
    }

    int32_t operator()(int32_t a, int32_t b) { return (this->*func_)(a, b); }

private:
    virtual int32_t add(int32_t a, int32_t b) { return 2*(a + b); }
    virtual int32_t sub(int32_t a, int32_t b) { return 2*(a - b); }

private:
    FUNC func_ = nullptr;
};


// 测试用例
ExamA a1(true), a2(false);
std::cout << a1(15, 10) << std::endl; // 25
std::cout << a2(15, 10) << std::endl; // 5

ExamB b1(true), b2(false);
std::cout << b1(15, 10) << std::endl; // 50
std::cout << b2(15, 10) << std::endl; // 10

```

- 函数类型(std::function)

类模版`std::function`是一种通用、多态的函数封装。`std::function`的对象可直接调用的，相比`函数指针`，是类型安全的。

原型: `template<class R, class... Args> class function<R(Args...)>`, `R`为被调用函数的返回类型； `Args...`为被调用函数的形参。


```cpp
// 普通函数
int32_t CommFunc(int32_t a) { return a; }

// lambda表达式
auto LambFunc = [](int32_t a) { return a; };

// 仿函数
class Functor {
public:
    int32_t operator()(int32_t a) { return a; }
};

class CFunc {
public:
    // 非静态成员函数
    int32_t cfunc(int32_t a) { return a; }
    // 静态成员函数
    static int32_t sfunc(int32_t a) { return a; }
};


// 用例
std::function<int32_t(int32_t)> test1 = CommFunc;
std::cout << test1(15) << std::endl;

std::function<int32_t(int32_t)> test2 = LambFunc;
std::cout << test2(15) << std::endl;

Functor functor;
std::function<int32_t(int32_t)> test3 = functor;
std::cout << test3(15) << std::endl;

CFunc cfunc;
std::function<int32_t(int32_t)> test4 = std::bind(&CFunc::cfunc, &cfunc, std::placeholders::_1);
std::cout << test4(15) << std::endl;

std::function<int32_t(int32_t)> test5 = std::bind(&CFunc::sfunc, std::placeholders::_1);
std::cout << test5(15) << std::endl;

std::function<int32_t(int32_t)> test6 = &CFunc::sfunc;
std::cout << test6(15) << std::endl;

```


- 函数对象(重载operator())

如果一个类重载了函数调用运算符`operator()(args)`，则该类的对象被称作`函数对象`; 函数对象常作为范型算法的实参来使用

```cpp
class Pnt {
public:
    Pnt(std::ostream &o) { this->os = o; }
    // 重载 operator()
    void operator()(const std::string &s) const {
        os << s << std::endl;
    }
private:
    std::ostream &os; // 
};

std::vector<std::string> vec{"aaa", "bbb", "ccc"};
// 生成一个函数对象（未命名的），作为for_each的第3个参数
for_each(vec.begin(), vec.end(), Pnt(std::cout));
```

lambda也是函数对象

```cpp
std::vector<std::string> vec{"aaa", "bbb", "ccc"};
// 生成一个函数对象（未命名的），作为for_each的第3个参数
for_each(vec.begin(), vec.end(), [](const std::string &s){ std::cout << s << std::endl; });
```

标准库定义的函数对象。在C++标准库中，定义了一组表示算术运算符/关系运算符/逻辑运算符的类，每个类分别定义了调用运算符。这些函数对象可以作为范型算法的参数.

C++中的几种可调用对象: 函数/函数指针/lambda表达式/bind创建的对象/重载了函数调用运算符的类 <br/>


### 运行时类型识别

- `typeid`运算符

原型为`typeid(e)`，e是任意类型的表达式或类型的名字. typeid返回标准库`type_info`(或派生类)对象的引用;
如果e是指向“基类”的指针或引用，且该类含有virtual函数，则需要到“运行时”才会求得值；否则在编译的时候就可以求得.


- `type_info`类
头文件`#include <typeinfo>`，不同编译器实现的细节不同. <br/>
至少包含`t1 == t2` `t1 != t2` `t1.name()` `t1.before(t2)` 4个函数. <br/>


- `dynamic_cast` 运算符

`dynamic_cast`运算符，将基类的指针(或引用)，安全的转换成派生类的指针(或引用). `dynamic_cast`的转换依赖虚表，没有virtual函数的类是没有虚表的，在编译时会报错. 

```cpp
dynamic_cast<type* > e; // 指针
dynamic_cast<type& > e; // 引用
dynamic_cast<type&&> e; // e不能是左值
```

对于指针类型，若转换失败，返回nullptr; 对于引用类型，若转换失败，抛出`bad_cast`异常.

```cpp
class Base { 
public:
    virtual f(); // 没有该virtual函数时，dynamic_cast转换会通不过编译
};

class Derived: public Base {
};

Base    *bp = new Derived; // 基类指针，指向派生类对象
Derived *dp = dynamic_cast<Derived*>(bp); // compile success, and dp != nullptr.

Base    *bp2 = new Base;
Derived *dp2 = dynamic_cast<Derived*>(bp2); // compile success, but dp2 = nullptr.

//
void f(const Base &b) {
    const Derived &d = dynamic_cast<const Derived&>b; // 若转换失败，会抛出bad_cast异常
}
```

在什么情况下，应该使用`dynamic_cast`替代需虚函数.


### 内存分配与对齐

- 分配一块1024字节的内存，要求起始地址16字节对齐
```cpp
void *mem = malloc(1024+15);
void *ptr = (((char*)mem)+15) & (~((char*)0x0F)); // 16字节对齐, 大小为1024字节
free(mem);
mem = nullptr;

// 另一种方法
// #include <stdlib.h>
void *ptr = aligned_alloc(16, 1024);
```

- 变量64字节对齐

```cpp
alignas(64) unit64_t count = 0;
```


### auto_ptr

`std::auto_ptr`是C++98标准中引入的一个智能指针，用于自动管理动态分配的内存. 在C++11中已经被弃用，并在C++17中被完全移除.

推荐使用`std::unique_ptr`作为替代. 因为它提供了更好的安全性和更清晰的所有权语义.

```cpp
#include <memory>

class Resource {
public:
    Resource() { fprintf(stdout, "Resource() \n"); }
    ~Resource() { fprintf(stdout, "~Resource() \n"); }

    void inc() { ++value_; }
    void pnt() { fprintf(stdout, "%u \n", value_); }

private:
    uint32_t value_ = 0;
};

void func() {
    // 创建一个 std::auto_ptr 管理 Resource 对象
    std::auto_ptr<Resource> ptr1(new Resource);

    ptr1->inc(); // ok

    // 将所有权转移给另一个 std::auto_ptr 对象
    std::auto_ptr<Resource> ptr2(ptr1);

    ptr1->inc(); // error: coredump.

    ptr2->inc(); // ok

    ptr2->pnt(); // 2

    // 函数结束时，ptr2 会被销毁，自动释放 Resource 对象
}
```

### unique_ptr

`std::unique_ptr`保证其拥有的资源在同一时间内只能被一个智能指针所拥有.
它不支持拷贝操作，但支持`移动`操作，可将 std::unique_ptr 的所有权从一个对象转移到另一个对象.

当`std::unique_ptr`的实例被销毁时（例如，当它离开其作用域时），它会自动释放其所拥有的资源，
通常是通过调用delete（如果是数组，则调用 delete[]）.

`std::unique_ptr`通常比`std::shared_ptr`更高效，因为它不需要维护引用计数.

```cpp
void fn() {
    // 创建一个 std::unique_ptr 来管理一个 Resource 对象
    std::unique_ptr<Resource> ptr(new Resource());

    // 以下操作会编译错误，因为 std::unique_ptr 不支持拷贝构造
    // std::unique_ptr<Resource> copy = ptr;

    // 以下操作会编译错误，因为 std::unique_ptr 不支持拷贝赋值
    // std::unique_ptr<Resource> another;
    // another = ptr;

    // 移动操作是允许的，这会将所有权从 ptr 转移到 newPtr
    std::unique_ptr<Resource> newPtr = std::move(ptr);

    // 在这个点，ptr 为空，newPtr 拥有 Resource 对象的所有权
    // 当 newPtr 被销毁时，Resource 对象将被自动释放
}
```

### shared_ptr

多个`std::shared_ptr`实例可以共同拥有同一个对象, 内部使用引用计数机制来跟踪有多少个`std::shared_ptr`实例共享同一个资源.
当最后一个`std::shared_ptr`实例被销毁或者被重新赋值时，它所拥有的资源会被自动释放.
通常调用 delete（或 delete[]，如果指针指向的是数组）来完成的.

`std::shared_ptr`比`std::unique_ptr`有更高的运行时开销，因为它需要维护引用计数.

不需要共享所有权时，推荐使用`std::unique_ptr`;
需要共享所有权或不确定资源所有权归属时，`std::shared_ptr`是一个很好的选择.

`std::shared_ptr`的引用计数操作是线程安全的，可以在多线程环境中使用.

```cpp
void fun() {
    // 创建一个 std::shared_ptr 来管理一个 Resource 对象
    std::shared_ptr<Resource> ptr1(new Resource());

    // 此时 ptr1 和 ptr2 都拥有 Resource 对象的所有权
    // 当 main 函数结束时，ptr1 和 ptr2 都会被销毁，自动释放 Resource 对象


    ptr1->inc(); // ok

    // 创建另一个 std::shared_ptr 实例，共享同一个 Resource 对象
    // std::shared_ptr<Resource> ptr2 = ptr1;
    std::shared_ptr<Resource> ptr2(ptr1);
    // std::shared_ptr<Resource> ptr2 = std::move(ptr1); // error

    ptr1->inc(); // ok

    ptr2->inc(); // ok

    ptr2->pnt(); // 3
}
```

### weak_ptr
 
 
 
## 标准库

### 容器

名称(关联容器) | 类型 | 特点 |
:---:|------|------|
map               | 红黑树实现k-v关联数组 |有序容器，key须支持< |
set               | 关键字即值 |有序容器，key须支持< |
multimap          | 关键字可重复出现的map |有序容器，key须支持< |
multiset          | 关键字可重复出现的set |有序容器，key须支持< |
unordered_map     | 用hash实现的map |无序容器，key须支持==|
unordered_set     | 用hash实现的set |无序容器，key须支持==|
unordered_multimap| 用hash实现的map，关键字可重复出现 |无序容器，key须支持==|
unordered_multiset| 用hash实现的set，关键字可重复出现 |无序容器，key须支持==|


名称(顺序容器)|特征|访问|插入/删除|备注|
:---:|----|----|---------|----|
vector      |可变大小数组|支持随机访问/下标访问、速度快  | 支持随机插入/删除, 但速度慢 | 中间插入时，需要移动后面的元素  |
array       |固定大小数组|支持随机访问/下标访问，速度快  | 不能添加或删除元素          | 数组大小固定，不能改变大小      |
list        |双向链表    |只支持双向[顺序]访问           | 插入/删除速度快             |                                 |
forward_list|单向链表    |只支持单向[顺序]访问           | 插入/删除速度快             |                                 |
deque       |双端队列    |支持快速随机访问               | 在头尾插入/删除速度快       |                                 |


常用类型 | 含义 |
---------|------|
size_type             |unsigned类型，可以表示容器大小|
difference_type       |signed类型，可保存两个迭代器间距离|
iterator              |begin(), end()     |
const_iterator        |cbegin(), cend()   |
reverse_iterator      |rbegin(), rend()   |
const_reverse_iterator|crbegin(), crend() |
value_type            |元素类型             |
reference             |value_type&，如 list<int>::reference val = *ilist.begin(); |
const_reference       |const value_type&，如 list<int>::const_reference val = *ilist.begin(); |


### iterator

- iterator const_iterator

`iterator`可以用来比较。当it1 和 it2指向同一个元素时，才相等

```cpp
vector<int> v(50);
if (vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
	*it = 0; //赋值为0
}

if(vector<string>:: const_iterator it = v.begin(); it != v.end(); ++it){
	cout << *it << end; //不能通过该迭代器，改变元素的内容
}

// 该迭代器，只能够指向v.begin()，不能够指向其他地方，但可以通过该迭代器修改所指向的内容。
const vector<string>:: iterator it = v.begin(); 
```

- 运算

`修改容器的内在状态`或`移动容器内的元素`时，要特别注意。这些操作会使指向被移动的元素的迭代器失效，也可能会使`迭代器失效`(如erase操作)

```
*iter
iter->mem
++iter, --iter, iter++, iter--
iter1 == iter2 ，当2个迭代器指向同一个容器中的同一个元素时，才相等
vector & deque提供的额外运算: `iter+n ,  iter-n,  iter1 += iter2, > , < >=, <=`，和指针的操作一样，不同于链表的操作
```

迭代器可以支持`it++`, `it--`这类操作，也可以支持`iter+n` , `iter-n`

也支持`iter1 - iter2`，得到的是`vector<T>::difference_type`类型，或者`vector<T>::size_type`类型，这2种类型，定义的是`signed`，

`vector<T>::iterator mid = v.begin+ v.size()/2;` 但不能够使用 `(v.begin() + v.end())/2;`


### vector

一个`vector`类似于一个动态的一维数组。`vector`对象（以及其他标准库容器对象）可以在运行的时候，高效的添加元素。
可以给`vector`预先分配好预定个数的内存，但更有效的方法，是先初始化一个空的`vector`对象，然后再动态的增加元素。
一个二维数组的方法：`vector< vector<int> > v;`

```cpp
// 头文件
#include <vector>
using namespace std::vector;

// 常用函数
v.size();
v.empty();
v.push_back(t) // 在末尾添加元素 
v[n]  // 要注意下标操作是否会越界
v.begin()
v.end() // vector的“末端元素的下一个”，指向一个不存在的元素。
```

添加元素的正确做法是`v.push_back(t);`，不能对不存在的元素进行下标操作。对不存在的元素进行下标操作，会导致缓冲区溢出。


### 顺序容器

任何`push/insert/delete/resize/erase`都可能导致**迭代器失效**。当编写循环将元素插入到vector或deque中时，程序要确保每次循环后，迭代器都得到更新。

常见操作
```cpp
c.begin(), c.end()  // 返回迭代器
c.front(), c.back() // 返回元素

c.push_back(t)     // 在容器c的尾部添加元素t，返回void
c.push_front(t)    // 在容器c的前段，添加元素t，返回void
c.insert(p, t)     // 在 p所指向 的元素的前面，插入t，返回指向新元素的迭代器
c.insert(p, n, t)    // 在p所指向的元素的前面，插入n个t，返回void
c.insert(p, b, e)    // 在p所指向 的元素的前面，插入b到e之间的元素，返回void

c.erase(p)     // 删除p指向的元素，返回指向p的下一个元素的迭代器 。当p指向c.end()时，行为未定义
c.erase(b, e)  // 删除b, e间的所有元素，返回指向e的下一个元素的迭代器
c.clear()      // 等价于 c.erase(c.begin(), c.end()); 返回void
c.pop_back()   // 只支持 list& deque，返回void
c.pop_front()  // 只支持 list& deque，返回void

while(c.empty() == false) {
	process(c.front());  // 返回第一个元素
	c.pop_front();       // 删除第一个元素，返回void
}

c.size()      // 返回c中有的元素的个数， c::size_type
c.max_size()  // 返回c中可以容纳的元素个数， c::size_type
c.empty()     // 为空时，返回true.
c.resize(n), c.resize(n, t)

c[n], c.at(n); // 使用下标操作时，一定要注意下标是否会越界；越界会使程序崩溃，只支持vector & deque
```

- 赋值操作

assign、=、swap

```cpp
c1 = c2         //先删除c1的所有元素，再将c2的元素赋值到c1.  c1 & c2，必须要有相同的类型
c1.assign(v.begin(), v.end())  // 先删除c1的所有元素。c1和v的类型可以不同。
c1.swap(c2)  // 交换c1 & c2中的元素，c1 & c2的类型要相同
```


### multimap与multiset
multimap/multiset，允许一个K，对应多个实例。
在multimap/multiset中，相同的K及其实例，都是相邻地存放的。
multimap，不支持下标操作。

`erase(K)` 带K时，删除所有拥有该K的元素，并返回被删除的个数。`erase(p)` 带迭代器时，只删除迭代器指定的元素，并返回void。


- find

```cpp
multimap<string, string> test;
multimap<string, string>::size_type c = test.count("aaaa"); //返回K=“aaa”的值的个数
multimap<string, string>::iterator iter = test.find("aaaa");
for(multimap<string, string>::size_type ct = 0; ct != c; ++ct, ++iter){
	cout << iter->second << endl;
}

// 或

typedef  multimap<string, string>::iterator authors_it;
authors_it beg = authors.lower_bound("aaaa"); //若查找的元素存在，则返回一个迭代器，指向第一个实例
authors_it end = authors.upper_bound("aaaa"); //返回的位置为最后一个实例的下一个位置
while(beg != end){
	cout << beg->second << endl;
	++beg;
}
// 若没有找到相关的元素，lower_bound, upper_bound返回相同的迭代器：都指向同一个元素或指向multimap的超出末端位置。

// 或

typedef  multimap<string, string>::iterator authors_it;
pair<authors_it, authors_it> pos = authors.equal_range("aaaa"); 
//equal_range返回pair类型。pair.first指向第一个位置，pair.second指向最后一个元素的下一个位置
while(pos.first != pos.second){
	cout << pos.first->second << endl;
	++pos.first;
}
```


### map

```cpp
map<K, V>::key_type    // 键的类型
map<K, V>::mapped_type // pair的类型。它的first具有const map<K, V>::key_type类型，second具有map<K, V>::mapped_type类型
map<K, V>::value_type  // 键所关联的值的类型
```

- 查找

常用的方法有`find`、`count`，`[]`，下标操作，有个副作用：当下标的元素不存在时，会创建一个元素。
```cpp
map<string, int> comp_count;
comp_count["dzh"] = 1; // 若找到，替换该值；若没找到，为map创建一个新K-V值，并插入该值到map中
int i = comp_count["zdwx"]; // 若找到，返回该值；若没有找到，创建一个新的K-V值，插入到map中，并返回V。

// 无副作用的做法
if(0 != comp_count.count("foobar")) { //计算个数，对map，返回0或1
	int occurs = comp_count["foobar"];
}

//更高效的做法
map<string, int>::iterator it = comp_count.find("foobar");
if(it != comp_count.end()){
	int occurs = it->second;
}
```

- insert

若插入的K值存在，则原map中K-V值不变，返回pair对象。其类型为`pair<map<string, int>::iterator, bool>`。bool为false. <br/>
若插入的K值不存在，则插入新的K-V值到map中，返回pair对象。其类型为`pair<map<string, int>::iterator, bool>`。bool为true. <br/>
`comp_count.insert(map<string, int>::value_type("Anna", 1));`  `comp_count.insert(make_pair<string, int>("Anna", 1));`


- erase

`m.erase(K);` 删除键为K的元素，返回size_type类型，表示删除的个数 【顺序容器返回被删除的元素的下一个iterator】 <br/>
`m.erase(p);` 删除迭代器it指向的元素。p指向的元素必须存在，且不能为m.end(), 返回void。<br/>
`m.erase(b, e);` 删除 [b, e)间的元素。b指向的元素必须存在或m.end()

```cpp
map<string, reg_info>::iterator it = g_map_reg_info.begin();
//错误的删除方法
for(; it != g_map_reg_info.end();++it) {
    if(XXX){
        g_map_reg_info.erase(it); //把原来的it删掉了，后面再++it，会崩溃
    }
}
//正确的删除方法
for(; it != g_map_reg_info.end(); ) {
    if(XXX) {
        it = g_map_reg_info.erase(it); //正确的删除方法
    }else{
        ++it;
    }
}
```


### bitset

```cpp
#include <bitset>

template <std::size_t N> class bitset;
```

N为大小，定义时必须指定大小，且大小不能修改


### swap

`swap`操作交换两个相同类型容器的内容; swap交换两个容器内容的操作会很快（array类型除外）。元素本身未交换，只交换了两个容器的内部数据结构，不对任何元素进行拷贝。 所以进行swap操作后，之前的指向一个元素的迭代器，在swap后还是指向该元素，但是地址空间属于另一个容器。swap甚至会引起迭代器失效.

`array`的`swap`操作，会真正的去交换数组的元素，所以其交换时间由数组中元素的个数来决定。对array进行swap操作后，其指针/引用/迭代器所绑定的元素的地址均不变（但内容已经变化）


### insert与emplace与erase

标准库提供3个速度更快的插入操作: emplace_front, emplace, emplace_back

```cpp
class CVS{ ...};

vector<CVS> v;

// 以下3个操作，需要创建一个“临时对象”，然后在vector中分配内存，执行赋值操作
v.insert(v(...));
v.push_back(v(...));
v.push_front(v(...));

// 使用emplace相关的函数，会直接在vector分配的内存中调用相关构造函数，少一个赋值操作
v.emplace(...);
v.emplace_back(...);
v.emplace_front(...);
```

- erase / insert

向容器中“插入/删除”元素后，指向容器元素的指针/引用/迭代器可能会失效. 所以每次操作后，要确保能够重新正确的定位迭代器

```cpp
// 正确的insert方法
vector<int32_t> vec = {1,2,3,4,5,6,7,8,9,0};
auto it = vec.begin();
while (it != vec.end()) {
    if (*it == 5) { 
        ++it; // 在5之"后"插入42.
        it = vec.insert(it, 42); // insert返回指向新元素位置的指针
        ++it;
    }
    else {
        ++it;
    }
}
```
```cpp
vector<int32_t> vec = {1,2,3,4,5,6,7,8,9,0};
auto it = vec.begin();
// 正确的erase方法
// erase后，迭代器会失效，所以需要重新计算
while( it != vec.end() ) {
    if (*it % 2 == 0) {
        it = vec.erase(it);
    }
    else {
        ++it;
    }
}
```
`insert`在给定迭代器的位置【之前】插入元素，并返回指向新元素的迭代器. `erase`操作删除给定迭代器位置的元素，并会返回一个迭代器，该迭代器指向序列中的下一个元素. 在`insert/erase`类操作后，需要重新计算end()


### erase

```cpp
// map
std::map<char> m;
// return an iterator to the element that follows the last element removed
// (or map::end, if the last element was removed).
for (auto it = m.begin(); it != m.end();) {
    if (it->second == 'b' || it->second == 'c' || it->second == 'd') {
        it = m.erase(it);
        // m.erase(it++); // 用这个也正确
    } else { ++it; }
}

// vector
std::vector<char> v;
// An iterator pointing to the new location of the element that followed 
// the last element erased by the function call.
// This is the container end if the operation erased the last element 
// in the sequence.
for (auto it = v.begin(); it != v.end();) {
    if (*it == 'b' || *it == 'c' || *it == 'd') {
        it = v.erase(it);
        // v.erase(it++); // 不正确
    } else { ++it; }
}

// list
std::list<char> l;
// An iterator pointing to the element that followed the last element erased by the function call.
// This is the container end if the operation erased the last element in the sequence.
for (auto it = l.begin(); it != l.end();) {
    if (*it == 'b' || *it == 'c' || *it == 'd') {
        it = l.erase(it);
        // l.erase(it++); // 用这个也正确
    } else { ++it; }
}

// set
std::set<char> s;
// return an iterator to the element that follows the last element removed 
// (or set::end, if the last element was removed).
for (auto it = s.begin(); it != s.end();) {
    if (*it == 'a' || *it == 'b' || *it == 'c') {
        it = s.erase(it);
        // s.erase(it++); // 用这个也正确
    } else { ++it; }
}
```


### function类型

function是模版

```cpp
#include <functional>

int32_t add(int32_t a, int32_t b) { return a + b; }

class Sum {
public:
    int32_t operator() (int32_t a, int32_t b) const { return a + b; }
};

// 
function<int32_t(int32_t, int32_t)> f1 = add;
function<int32_t(int32_t, int32_t)> f2 = Sum(); // 函数对象
function<int32_t(int32_t, int32_t)> f3 = [](int32_t a, int32_t b) { return a + b; }; // lambda表达式

f1(1, 2); 
f2(1, 2);
f3(1, 3);

// 作为类型使用
std::map<std::string, function<int32_t(int32_t, int32_t)>> funcs;
funcs["add"] = add;
funcs["Sum"] = Sum();
funcs["lam"] = [](int32_t, int32_t) { return a + b; };
```


### random

`随机数引擎类`(random-number engines), 生成随机`unsigned`整数序列；

`随机数分布类`(random-number distribution)，使用引擎返回服从特定分布的随机数

```
Engine e(s); // 使用s作为种子
e.seed(s); // 重置种子
e.min(), e.max(); // 该引擎生成的最小值和最大值
```

```cpp
#include <random>

void get_random() {
    std::default_random_engine e(time(nullptr)); // 生成随机无符号数引擎
    // e.seed(time(nullptr));
    
    for (int32_t i = 0; i < 20; ++i) {
        fmt::print("{}\n", e());            
    }
}

void get_random() {
    std::default_random_engine e(time(nullptr));
    // or e.seed(time(nullptr));
    std::uniform_int_distribution<int32_t> u(-20, 20); // 生成[-20, 20]间的随机数

    fmt::print("{}\n", u(e)); // u的入参是随机数引擎，不是随机数    
}
```

其余随机数分布类

```cpp
uniform_int_distribution<int32_t> u(-100, 100);
uniform_real_distribution<double> u(-1.0, 1.0); // 均匀分布在[-1.0, 1.0]间的double
normal_distribution<> n(10, 2.5); // 正态分布，均值是4，标准差是2.5
bernolli_distibution b(p); // 伯努利分布，给定概率p生成true；p的默认值是0.5
```

### regex




## 类的设计

### 构造函数

拷贝构造函数(copy constructor) <br/>
移动构造函数(move constructor) <br/>
拷贝赋值运算符(copy-assignment operator) <br/>
移动赋值运算符(move-assignment operator) <br/>
析构函数(destructor) <br/>


- 拷贝初始化

拷贝初始化通常使用`拷贝构造函数`或`移动构造函数`来完成. 使用拷贝初始化的地方:
> (a) 使用"="定义变量 <br>
> (b) 将一个对象作为实参传递给一个非引用类型的形参 <br/>
> (c) 从函数（返回类型为非引用类型）返回一个对象 <br/>
> (d) 用花括号列表初始化一个数组中的元素，或一个聚合类中的成员 <br/>
```cpp
string dots(10, '.');  // 直接初始化
string s1(dots);       // 直接初始化
string s2 = dots;          // 拷贝初始化
string s3 = "aaaaa"        // 拷贝初始化
string s4 = string("aaa"); // 拷贝初始化
```
标准容器调用`insert/push`时，使用的是拷贝初始化；调用`emplace`时，使用的是直接初始化. 

`拷贝构造函数`可以有多个参数，但第一个参数必须是对`类自身的引用`，且额外参数要有默认值. 如果我们没有定义`拷贝构造函数`，编译器会为我们"合成"一个默认的拷贝构造函数


- 拷贝赋值运算符

如果类未定义自己的`拷贝赋值运算符`，编译器就会"合成"一个. 如果类含有`引用`或`const`类型的成员变量时，则编译器无法合成默认构造函数，也无法合成拷贝构造函数与赋值函数.

- default/delete
`default/delete`只能修饰具有合成版本的成员函数(默认构造函数、拷贝构造函数、移动构造函数、拷贝赋值运算符号、移动赋值运算符、析构函数). 使用`default`，表示让编译器为相应的成员函数，合成一个默认的版本

```cpp
class CTest {
public:
    CTest() = default;
    CTest(const CTest &) = default;
    CTest & operator= (const CTest &) = default;
    ~CTest() = default;   
};
```
```cpp
class NoDel {
public:
    NoDel() = delete;
    ~NoDel() = delete; // 析构函数, 或private
};

NoDel a; // error
NoDel * p = new NoDel; // OK
delete p; // error
```


- 移动构造函数 / 移动赋值运算符

```cpp
class T {
public:
    // 移动构造函数
    // 接管原来的内存，并将源对象的指针置为nullptr.
    T(T&& t) noexcept {
        this->p1 = t.p1, this->p2 = t.p2;
        t.p1 = nullptr, t.p2 = nullptr;
    }
    // 移动赋值函数
    T& operator= (T&& t) noexcept {
        if (this != &t) {
            this->p1 = t.p1, this->p2 = t.p2;
            t.p1 = nullptr, t.p2 = nullptr;
        }
        return *this;     
    }
private:
    int32_t *p1;
    int32_t *p2;
};
```
`移动构造函数`通常不分配内存，`noexcept`告诉编译器，本函数不抛异常. <br/>
编译器合成`移动构造函数`和`移动赋值函数`的条件:
> (a) 该类没有定义任何版本的拷贝控制成员（拷贝构造函数、移动构造函数、拷贝赋值函数、移动赋值函数、析构函数）. <br/>
> (b) 且它的所有非static型数据成员都能移动. <br/>
若一个类定义了自己的`移动构造函数`或`移动赋值函数`，那么必须自己定义其它的拷贝操作（拷贝构造函数、拷贝赋值函数、析构函数），否则这些拷贝操作默认是被定义为删除的.


- 编译器合成函数总结

若类中含`引用`或`const`类型的成员变量时，编译器无法合成`默认构造函数`/`拷贝构造函数`/`拷贝赋值函数` <br/>
若类定义了自己的`移动构造函数`或`移动赋值函数`，那么类必须自己定义其它拷贝操作（拷贝构造函数/拷贝赋值函数/析构函数），否则这些拷贝操作默认是被定义为删除的. <br/> 
合成`默认构造函数`条件: 若类没有定义任何构造函数，则编译器会合成一个默认构造函数. <br/>
合成`拷贝构造函数`条件: 若类没有定义任何拷贝构造函数，则编译器会合成一个默认的拷贝构造函数. <br/>
合成`拷贝赋值函数`条件: 若类没有定义任何拷贝赋值函数，则编译器会合成一个默认的拷贝赋值函数. <br/>
合成`移动构造函数`/`移动赋值函数`条件: 
> 该类没有定义任何版本的拷贝控制函数（拷贝构造函数/移动构造函数/拷贝赋值函数/移动赋值函数/析构函数）.<br/>
> 且它的所有非static型成员变量都是可移动的. <br/>
合成`析构函数`条件: 若类未定义任何析构函数，则编译器会合成一个默认的析构函数. <br/>
使用`=default`，表示使用编译器默认合成的版本，否则编译器不一定会合成默认版本. <br/>
 

### 显示调用构造函数和析构函数

```cpp
class Item {
public:
    // 带参数的构造函数
    Item(int32_t idx)  {
        idx_ = idx;
        std::cout << "Item(" << idx_ << ")" << std::endl;
    }
    ~Item() { std::cout << "~Item(" << idx_ << ")" << std::endl; }

private:
    int32_t idx_;
};

Item* test_cons(int32_t cnt) {
    Item *it = (Item*)std::malloc(sizeof(Item) * cnt);
    for (int32_t i = 0; i < cnt; ++i) {
        new (&(it[i])) Item(i); // 显示调用构造函数
    }

    return it;
}

void test_dest(int32_t cnt, Item *it) {
    for (int32_t i = 0; i < cnt; ++i) {
        it[i].~Item();
    }
}
```

### 类型转换运算符

负责将类类型转换为其它类型，原型: `operator type() const`, 不允许带参数

```cpp
class Chg {
public:
    Chg(double d) : v(d) {}
    // 类型转换运算符
    // 将类类型，转换为int32_t类型
    operator int32_t() const { return (int32_t)d; }
    // 将类类型，转换为double类型. 该函数与上面的函数，在隐式转换时，常出现二义性
    // operator double() const { return d; }
private:
    double v;
};

Chg c(3.14);
std::cout << c + 5 << std::endl;
```

类型转换多发生在隐式转换，可能存在`二义性`



## 模版与范型编程

### 常用算法

unaryPred, binaryPred，是一元和二元谓词，分别接收一个和两个参数，均来自输入序列，谓词的返回值可作为条件

unaryOp, binaryOp, 是可调用对象，分别使用输入序列的一个或两个实参调用

comp是二元谓词


| 比较算法(大小) | | | | |
|--|--|--|--|--|
| min(val1, val2) | min(val1, val2, comp) | min(init_list) | min(init_list, comp) | 返回最小值 | 
| max(val1, val2) | max(val1, val2, comp) | max(init_list) | max(init_list, comp) | 返回最大值 |
| minmax(val1, val2) | minmax(val1, val2, comp) | minmax(init_list) | minmax(init_list, comp) | 返回pair |
| min_element(beg, end) | min_element(beg, end, comp) | 
| max_element(beg, end) | max_element(beg, end, comp) |
| minmax_element(beg, end) | minmax_element(beg, end,comp) | | | 返回pair |
| lexicographical_compare(beg1, end1, beg2, end2) | lexicographical_compare(beg1, end1, beg2, end2, comp) | 比较两个序列的大小，返回true/false |


| 数值算法 | | |
|---|---|---|
| accumulate(beg, end, init) | accumulate(beg, end, init, binaryOp) | 求和 |
| inner_product(beg1, end1, beg2, init) | inner_product(beg1, end1, beg2, init, binOp1, binOp2) | 乘积再求和 |
| partial_sum(beg, end, dest) | partial_sum(beg, end, dest, binaryOp) | 求和, 新序列写入dest中 |
| adjacent_difference(beg, end, dest) | adjacent_difference(beg, end,dest, binaryOp) | 求差, 新序列写入dest中 |
| iota(beg, end, val) |


| 查找算法 |  |   |   |
|----------|--|---|---|
| find(beg, end, val) | find_if(beg, end, unaryPred) | find_if_not(beg, end, unaryPred)｜返回迭代器，指向第一个满足条件的元素 |
| count(beg, end, val) | count_if(beg, end, unaryPred) | | 返回出现次数 |
| all_of(beg, end, unaryPred) | any_of(beg, end, unaryPred) | none_of(beg, end, unaryPred) | 所有满足/任一满足/都不满足 |
| adjacent_find(beg, end) | adjacent_find(beg, end, binaryPred) | | 返回第一对相邻重复元素的迭代器 |
| search_n(beg, end, count, val) | search_n(beg, end, cout, val, binaryPred) | | 返回迭代器，从此位置开始有count个相等元素 |
| search(beg1, end1, beg2, end2) | search(beg1, end1, beg2, end2, binaryPred) | | 返回[beg2, end2)在[beg1, end1)中第一次出现的位置 |
| find_first_of(beg1, end1, beg2, end2) | find_first_of(beg1, end1, beg2, end2, binaryPred) | | 返回[beg2, end2)中任一元素在[beg1, end2)中第一次出现的位置|
| find_end(beg1, end1, beg2, end2) | find_end(beg1, end1, beg2, end2, binaryPred) | | |


| 二分查找 | 要求序列中元素有序 | |
|----|----|---|
| lower_bound(beg, end, val) | lower_bound(beg, end, val, comp) | 返回迭代器，指向第一个小于等于val的元素 | 
| upper_bound(beg, end, val) | upper_bound(beg, end, val, comp) | 返回迭代器，表示第一个大于val的元素 |
| equal_range(beg, end, val) | equal_range(beg, end, val, comp) | 返回pair |
| binary_search(beg, end, val) | binary_search(beg, end, val, comp) | 返回bool |
| partial_sort(beg, mid, end) | partial_sort(beg, mid, end, comp) | 部分排序 |
| parttal_sort_copy(beg, end, dstBeg, dstEnd) | partial_sort_copy(beg, end, dstBeg, dstEnd, comp) |
| nth_element(beg, nth, end) | nth_element(beg, nth, end, comp) |


| 重排算法 | | |
|---|---|---|
| remove(beg, end, val) | remove_if(beg, end, unaryPred) |
| remove_copy(beg, end, dest, val) | remove_copy_if(beg, end, dest, val, unaryPred) |
| unique(beg, end) | unique(beg, end, binaryPred) | 删除相邻重复元素 |
| unique_copy(beg, end, dest) | unique_copy_if(beg, end, dest, binaryPred) |
| rotate(beg, mid, end) | rotate_copy(beg, mid, end, dest) |
| reverse(beg, end) | reverse_copy(beg, end, dest) | 翻转 |


| 排序 | | |
|---|----|---|
| sort(beg, end) | sort(beg, end, comp) |
| stable_sort(beg, end) | stable_sort(beg, end, comp) |
| is_sorted(beg, end) | is_sorted(beg, end, comp) | 返回bool，是否排序 |
| is_sorted_until(beg, end) | is_sorted_until(beg, end, comp) | 返回迭代器，查找最长子序列 |

| 只读操作 | | |
|---|---|---|
| for_each(beg, end, unaryPred) | 遍历, unaryPred的返回值被忽略 |
| mismatch(beg, end, beg2) | mismatch(beg1, end2, beg2, binaryPred) | 比较两个序列中的元素是否匹配 |
| equal(beg1, end1, beg2) | equal(beg1, end1, beg2, binaryPred) | 比较两个序列是否相等 |

