# Table of Contents

* [基础](#基础)
  * [数据类型的选择与转换](#数据类型的选择与转换)

* [标准库](#标准库)

* [类的设计](#类的设计)

* [高级主题](#高级主题)

  * [变量与基本类型](#变量与基本类型)
  * [string与vector与array](#string与vector与array)
  * [表达式](#表达式)
  * [函数](#函数)

  * [顺序容器](#顺序容器)
  * [范型算法](#范型算法)
  * [关联容器](#关联容器)

  * [拷贝控制](#拷贝控制)
  * [重载运算与类型转换](#重载运算与类型转换)
  * [模版与范型编程](#模版与范型编程)




## 基础

### 数据类型的选择与转换

- 如何选择数据类型

可以使用`size_t` `ssize_t` `int32_t` `uint32_t`类型；指针使用nullptr代替NULL

(1) 明确知道数据值不为负时，选择`unsigned`. <br/>
(2) `char`由"编译器"决定是`signed char`还是`unsigned char`，所以不要将char放入运算表达式中，否则容易出问题. <br/>
(3) 在存放数据时才使用char/bool，在计算时不要使用char/bool。char的类型是未知的，可能为signed, 也可能为unsigned。若实在需要char类型参与计算，可以明确为signed char, 或unsigned char. <br/>
(4) 浮点数运算，选用double而不是float。因为float精度通常不够，而且在某些机器上，double的速度可能比float速度更快. <br/>
(5) 对浮点数用乘法和除法，结果会因为精度而不同。如: <br/>

```cpp
int64_t v = 3724141724135945;
double d1 = ((double)v) * 0.0001;
double d2 = ((double)v) /  10000;
fprintf(stdout, "%.6f ", d1); // 372414172413.594543
fprintf(stdout, "%.6f ", d2); // 372414172413.594482
```

- 类型转换

(1) 给`signed`变量赋值一个超出其表示范围的值时，结果是`未定义`的（可能继续工作/崩溃/垃圾数据）. <br/>
(2) 不要混用`带符号的变量`和`无符号的变量`:
> (a) 给`unsigned`类型赋值一个超过其范围的数字时，会进行截断。只保留其范围内的部分. <br/>
> (b) `signed`和`unsigned`相加时，都转化为`unsigned`。signed会隐式转会为unsigned. <br/>
(3) 把整数赋值给浮点数时，小数部分为0。同时要注意是否超过浮点数类型的容量，精度是否会损失. <br/> 
(4) 转义："\x"后跟的1个或多个16进制数值；"\"后跟8进制数字；如"\115"="\x4d". <br/>
(5) 显式类型转换 `cast-name<type> (expression)`
> (a) static_cast, 任何具有明确定义的类型转换，只要不包含底层const，都可以使用static_cast. 在编译阶段就确定. <br/>
> (b) const_cast, 去const，但只能改变运算对象的底层const. (注: 强行去const，并修改一个const变量的值，是设计的缺陷，可能不会成功，依赖编译器的实现. const_cast更多的是函数调用时，对const参数做适配(被调用函数内部实际上不修改参数的值)). <br/>
> (c) reinterpret_cast, <br/>
> (d) dynamic_cast, 在运行阶段进行转化. <br/>


- 算数类型转换

(1) `整形提升`，计算时，把小整数类型转扩展成较大的整数类型. <br/>
(2) `无符号类型`
> (a) 若都为`无符号类型`，则扩张成较大类型. <br/>
> (b) 若`无符号类型`不小于`带符号类型`，则带符号的会先转成无符号(注意扩展的副作用). <br/>
> (c) 若`无符号类型`小于`带符号类型`，行为不定，依赖机器. <br/>








声明与定义
```cpp
extern int32_t i; // 声明

int32_t j, k = 5; // 定义
// 若给一个extern标记的变量赋予值，该extern将不起作用，变成定义而不是声明
extern int32_t l = 6; // 定义
```
> (1) 初始化是创建变量时，赋予一个初始值；赋值是将原来的值擦除，用一个新值来代替. <br/>
> (2) 用初始化列表去初始化变量时，若发生数据丢失，编译器会报错。如int a = {3.14}; <br/>


const变量
> (1) 默认情况下，const对象仅在文件内有效。当多个文件中出现了同名的const变量，等同于在不同的文件中分别定义了独立的变量. <br/>
> (2) 为了避免这个问题，可以在声明或定义的时候，加`extern`
```cpp
extern const int32_t g_size = f(); // file.cpp中声明
extern const int32_t g_size; // file.h中定义
```

指针与const
> (1) 建议使用nullptr，不要使用NULL。所有指针在使用之前，一定要初始化. <br/>
> (2) 指向常量的指针(pointer to const)
```cpp
double d1 = 3.14, d2 = 2.24;
const double *pval = &d1; // const修饰*pval. low-level const
*pval = 3.2; // error，（1）不能通过指针修改所指对象的值
pval = &d2;  // ok，（2）修改指针所指向的地址
```
> (3) 常量指针(const pointer)，在定义的时候必须被初始化，不能再指向其它地址
```cpp
double d1 = 3.14, d2 = 2.24;
double * const pval = &d1; // const修饰pval. top-level const
*pval = 3.2; // ok,
pval = &d2;  // error,
```


常量表达式
> (1) `常量表达式`是指值不会改变，且在`编译阶段`就须得到计算结果的表达式(非运行阶段).
```cpp
const int32_t a = 5;
const int32_t b = a + 1; // a，b均为常量表达式
const int32_t c = current_time(); // c需要在运行中才能得到结果，非常量表达式
```
> (2) C++11允许将变量声明为`constexpr`类型，这样编译器在 编译阶段 就检查该值是否为常量表达式.
```cpp
constexpr int32_t a = 5;      // 5 是常量表达式
constexpr int32_t b = a + 1;  // a+1是常量表达式
constexpr int32_t c = size(); // 要求size()是一个constexpr函数时，才可以通过编译
```
> (3) 在`constexpr`声明中如果定义了一个指针，则`constexpr`仅对指针有效，与指针所指的对象无关.
```cpp
int32_t i = 5;
// p1为指向常量的指针，不能通过p1来修改对象的值，但p1可以再指向其它地址
const int32_t *p1 = &i;
// p2为指向整数的常量指针，p2不能再指向其它地方。
// 同时，要求i的地址在编译阶段就要确定，即i是一个全局变量，或者静态局部变量
constexpr int32_t *p2 = &i;
```


类型别名 `typedef` & `using`
> (1) 使用关键字typedef
```cpp
typedef double Num;
// base, Num都是double的同义词； Pointer 是double*的同义词
typedef Num base, *Pointer;

// typedef 与 const一起使用
char c = 'A';
typdef char *pstring;

// const 修饰cstr，说明cstr的值不能被改变. 同时cstr是一个指向char的指针
const pstring cstr = &c; // 等价于 char* const cstr = &c; 而不是等价于 const char* cstr;

// const 修饰*ps。ps是指针，不能通过ps来修改所指地址中的值，该地址中的值也是一个指针
const pstring *ps;  // 等价于 char* const *ps;
```
> (2)使用关键字using
```cpp
using Num = int32_t; // Num是int32_t的同义词
Num c = 5;
```
> (3)数组声明
```cpp
using ARR1=char[1024];
typedef char ARR2[1024];

std::cout << "output:" << sizeof(ARR1) << ", " << sizeof(ARR2) << std::endl; // output: 1024, 1024
// 二维数组
ARR1 arr1[32];
ARR2 arr2[32];
```


自动类型推断`auto`

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
> (1) C++11引入auto，有两个作用: `自动类型推断`和`返回值占位`. <br/>
> (2) 在"编译阶段"由编译器根据初始值来推导类型，因此，auto定义的变量必须有初始值. <br/>
> (3) 可以使用`valatile` `pointer(*)` `reference(&)` `rvalue reference(&&)`来修饰auto. <br/>
> (4) `auto`不能自动推导成CV-qualifiers(constant & volatile qualifiers), 除非被声明为引用类型. <br/>
> (5) 如果初始化表达式是引用，则去除引用语义. <br/>
> (6) 如果初始化表达式是const或volatie(或两者都有)，则去除const/volatile语义 (auto会去掉顶层const，保留底层const). <br/> 
> (7) 如果auto关键字上带&，则不去除const语义. <br/>
> (8) 函数的模版参数不能被声明为auto. <br/>
> (9) auto会被退化成指向数组的指针，除非被声明为引用. <br/>


decltype 类型指示符
> (1) decltype的作用是选择并返回操作数的数据类型。在编译阶段分析表达式并得到其类型，并不去计算表达式的值. <br/>
```cpp
const int32_t i = 0, &j = i, *p = &i;
decltype(i) x = 0; // 等价于 const int32_t x = 0;
decltype(j) y = x; // 等价于 const int32_t &y = x; decltype(j)是引用类型
decltype(j) z;     // 等价于 const int32_t &z; error, 引用必须初始化
decltype(*p) m;    // 等价于 const int32_t &m; error, 解引用
```
> (2) decltype使用的是表达式的话，需要根据表达式的结果来推测对应的类型. <br/>
> (3) decltype和auto不同，不会忽略引用和顶层的const. <br/>
> (4) decltype使用的表达式是解引用`decltype(*p)`，则decltype得到引用类型. <br/>
> (5) decltype使用的表达式加括号与不加括号意义不同，加括号就是一个表达式，就会变成引用. <br/>
```cpp
int32_t i = 0;
decltype(i)   x; // ok.
decltype((i)) y; // error. 两层括号，变成引用，等价于 int32_t &y;
```
> (6) 赋值是会产生引用的一类表达式，引用的类型就是左值的类型。
```cpp
int32_t i = 5, j = 10;
decltype(i)   a = i; // int32_t  a = i;
decltype(i=j) b = a; // int32_t &b = a; 表达式i=j的类型是 int32_t&
```

## string与vector与array

命名空间using
> (1) 每个名字需要独立的`using`声明. <br/> 
> (2) 头文件中不应该包含using声明。因为会被其它文件中引用，造成名字冲突.


直接初始化与拷贝初始化
> (1) 使用等号（=）来初始化一个变量，实际执行的是`拷贝初始化(copy initialization)`, 编译器把等号右侧的值拷贝到新创建的对象中去. <br/>
> (2) 若不使用等号，则执行的是直接初始化(direct initialization).
```cpp
string s1 = "abc"; // 拷贝初始化
string s2("abc");  // 直接初始化
```


复杂数组声明
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


使用标准库的begin(), end() 来遍历数组
```cpp
int arr[100];
// int* pBeg = begin(arr), *pEnd = end(arr);
for (int* pBeg = begin(arr); pBeg != end(arr); ++pBeg) {
    std::cout << *pBeg;
}
```


数组与指针
```cpp
int32_t iarr[] = {0, 1, 2, 3, 4};
int32_t    *p1 = iarr; // 等价于 int32_t *p1 = &(iarr[0]);

auto     p2(iarr); // 等价与 int32_t *p2 = &(iarr[0]); auto p2(&(iarr[0]));
decltype(iarr) p3; // 等价于 int32_t p3[5];
```
> (1) 在很多用到数组名字的地方，编译器会自动地把名字替换为一个指向数组首元素的指针. <br/>
> (2) 数组名作为一个auto变量的初始值时，推断出来的类型是指针而非数组. <br/>
> (3) 数组名作为decltype的表达式时，返回的类型是数组. 



## 表达式

lvalue & rvalue
> (1) C++11中所有的值(表达式)必属于左值/右值之一; <br/>
> (2) 当一个对象被当作`右值`的时候，用的是对象的值（内容）；当对象被用作`左值`的时候，用的是对象的身份（在内存中的位置）；<br/>
> (3) 可以取地址的且有名字的就是`左值`; 不能取地址的、没有名字的就是`右值`; <br/>
> (4) C++11中的右值，又分为`纯右值`(prvalue, Pure Rvalue) 和 `将亡值`: <br/>
> > `纯右值`指临时变量和不跟对象关联的字面量值；<br/>
> > `将亡值`是C++11新增的跟右值引用相关的表达式，这样的表达式通常是将要被移动的对象（移为他用），比如返回右值引用T&&的函数返回值、std::move的返回值、转换为T&&的类型转换函数的返回值; <br/>
> > 将亡值可以理解为通过“盗取”其它变量内存空间的方式获取到的值。在确保其它变量不再被使用、或即将被销毁时，通过“盗取”的方式可以避免内存空间的释放和分配，延长变量的生命期. <br/>



## 函数

含有可变形参的函数
```cpp
// 使用initializer_list<T>
int32_t sum(initializer_list<int32_t> args) {
    int32_t ret = 0;
    for (auto it : args) {
        ret += *it;
    }
    return ret;
}

std::cout << sum({2, 5}) << std::endl;
std::cout << sum({2, 5, 8}) << std::endl;

// 使用省略符
int32_t sum(int32_t cnt, ...);
```
> (1) initializer_list形参
> > (a) initializer_list是标准库类型(类模版)，用于表示某种特定类型的值的数组. <br/>
> > (b) initializer_list中的元素都是常量值，无法改变其元素的值. <br/>
> > (c) initializer_list中所有元素的类型都相同. <br/>
>
> (2) 省略符形参


返回数组指针的写法
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

constexpr函数
```cpp
constexpr int32_t size(int32_t cnt) {
    return cnt * 4;
}

int32_t arr1[size(5)]; // ok

int32_t i = 5;
int32_t arr2[size(i)]; // error
```
> constexpr的返回值不一定是常量表达式 <br/>
> constexpr函数会被编译器内联

# 标准库

## 顺序容器

顺序容器类型

名称|特征|访问|插入/删除|备注|
:---:|----|----|---------|----|
vector      |可变大小数组|支持随机访问/下标访问、速度快  | 支持随机插入/删除, 但速度慢 | 中间插入时，需要移动后面的元素  |
array       |固定大小数组|支持随机访问/下标访问，速度快  | 不能添加或删除元素          | 数组大小固定，不能改变大小      |
list        |双向链表    |只支持双向[顺序]访问           | 插入/删除速度快             |                                 |
forward_list|单向链表    |只支持单向[顺序]访问           | 插入/删除速度快             |                                 |
deque       |双端队列    |支持快速随机访问               | 在头尾插入/删除速度快       |                                 |

array在定义的时候，必须指定大小，如 
```cpp
array<int32_t, 1024> a;
array<int32_t, 1024>::size_type size;
```

常用类型 | 含义 |
---------|------|
size_type             |无符号整型，可以表示容器大小|
value_type            |元素类型|
difference_type       |带符号整型，可保存两个迭代器间距离|
iterator              |begin(), end()     |
const_iterator        |cbegin(), cend()   |
reverse_iterator      |rbegin(), rend()   |
const_reverse_iterator|crbegin(), crend() |

swap
> swap操作交换两个相同类型容器的内容。 <br/>
> swap交换两个容器内容的操作会很快（array类型除外）。元素本身未交换，只交换了两个容器的内部数据结构，不对任何元素进行拷贝。
> 所以进行swap操作后，之前的指向一个元素的迭代器，在swap后还是指向该元素，但是地址空间属于另一个容器。swap甚至会引起迭代器失效<br/>
> array的swap操作，会真正的去交换数组的元素，所以其交换时间由数组中元素的个数来决定。对array进行swap操作后，其指针/引用/迭代器所绑定的元素的地址均不变（但内容已经变化）


emplace操作
> C++11提供3个速度更快的插入操作: emplace_front, emplace, emplace_back
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

插入/删除元素
> 向容器中“插入/删除”元素后，指向容器元素的指针/引用/迭代器可能会失效。
> 所以每次操作后，要确保能够重新正确的定位迭代器 <br/>
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
> insert在给定迭代器的位置【之前】插入元素，并返回指向新元素的迭代器. <br/>
> erase操作删除给定迭代器位置的元素，并会返回一个迭代器，该迭代器指向序列中的下一个元素. <br/>
> 在insert/erase类操作后，需要重新计算end(). <br/>


## 范型算法

范型算法参数
> (1) 对于接受2个容器的算法。若只接受一个单一迭代器来表示第二个序列的算法，都假定第二个序列至少跟第一个序列一样长. <br/>
```cpp
// 比较vec1 & vec2两个序列，默认要求vec2至少跟vec1一样长
equal(vec1.begin(), vec1.end(), vec2);
```
> (2) 范型算法并不去检查容器的目的地址是否有足够的空间.
```cpp
// 向vec中插入n个元素（值为value）. 但算法本身不会去检查vec的空间是否足够
fill_n(vec, n, value);
```

for_each
find_if
sort
stable_sort

lambda表达式
> 表达式原型: `[capture list] (parameter list) -> return type { function body; }` <br/>
> (1) lambda的捕获列表是一个lambda所在函数中定义的局部变量的列表（通常为空，表示不使用局部变量） <br/>
> (2) lambda【必须】使用尾置返回来指定返回类型，但可以忽略`参数列表`和`返回类型` <br/> 
> > (a) 值捕获，被捕获的变量的值，是lambda创建的时拷贝，因此，随后对值的修改，不会影响lambda内对应的值 <br/>
> > (b) 引用捕获，当使用引用捕获时，要确保lambda执行期间，值是存在的 <br/>
> > (c) 可以从函数返回lambda <br/>
```cpp
int32_t v1 = 42, v2 = 45;

auto func1 = [v1]() { return v1; };
v1 = 12;
std::cout << func1() << std::endl; // 42, 在lambda创建的时候，就保存了v1的值

auto func2 = [&v2]() { return v2; };
v2 = 12;
std::cout << func2() << std::endl; // 12
```
> > (d) 隐式捕获，让编译器自己去推断捕获那些变量
```cpp
it32_t v1 = 42, v2 = 45;
auto f1 = [=]() { return v1; }; // 隐式捕获，采用值的方式来捕获
auto f2 = [&]() { return v2; }; // 隐式捕获，采用引用的方式来捕获
```

bind函数适配器
> (1) `#include <functional>` 头文件. <br/>
> (2) 一般形式: `auto newCallable = bind(callable, arg_list);` , bind生成一个可调用的函数对象. <br/>
> (3) arg_list是callable的参数列表，当调用newCallable时，callable就会被调用. <br/>
> (4) std::placeholders::_1, std::placeholders::_2 ... , std::placeholders::_n，分别表示newCallable的调用参数. <br/>
```cpp
auto func1 = bind(f, a, b, _2, c, _1); // bind返回可调用的函数对象, _1, _2分别为func1的第一个、第二个参数
func1(25, 36);

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
> (5) ref返回引用， cref返回const引用 <br/>



## 关联容器

关联容器类型

名称 | 类型 | 特点 |
:---:|------|------|
map               | 红黑树实现k-v关联数组 |有序容器，key须支持< |
set               | 关键字即值 |有序容器，key须支持< |
multimap          | 关键字可重复出现的map |有序容器，key须支持< |
multiset          | 关键字可重复出现的set |有序容器，key须支持< |
unordered_map     | 用hash实现的map |无序容器，key须支持==|
unordered_set     | 用hash实现的set |无序容器，key须支持==|
unordered_multimap| 用hash实现的map，关键字可重复出现 |无序容器，key须支持==|
unordered_multiset| 用hash实现的set，关键字可重复出现 |无序容器，key须支持==|



# 设计类

## 拷贝控制

> 拷贝构造函数(copy constructor) <br/>
> 移动构造函数(move constructor) <br/>
> 拷贝赋值运算符(copy-assignment operator) <br/>
> 移动赋值运算符(move-assignment operator) <br/>
> 析构函数(destructor) <br/>


拷贝初始化
> (1) 拷贝初始化通常使用`拷贝构造函数`或`移动构造函数`来完成 <br/>
> (2) 使用拷贝初始化的地方:
> > (a) 使用"="定义变量 <br>
> > (b) 将一个对象作为实参传递给一个非引用类型的形参 <br/>
> > (c) 从函数（返回类型为非引用类型）返回一个对象 <br/>
> > (d) 用花括号列表初始化一个数组中的元素，或一个聚合类中的成员 <br/>
```cpp
string dots(10, '.');  // 直接初始化
string s1(dots);       // 直接初始化
string s2 = dots;          // 拷贝初始化
string s3 = "aaaaa"        // 拷贝初始化
string s4 = string("aaa"); // 拷贝初始化
```
> (3) 标准容器调用insert/push时，使用的是拷贝初始化；调用emplace时，使用的是直接初始化


拷贝构造函数
> (1) `拷贝构造函数`可以有多个参数，但第一个参数必须是对`类自身的引用`，且额外参数要有默认值 <br/> 
> (2) 如果我们没有定义`拷贝构造函数`，编译器会为我们"合成"一个默认的拷贝构造函数 <br/>


拷贝赋值运算符
> (1) 如果类未定义自己的`拷贝赋值运算符`，编译器就会"合成"一个 <br/>
> (2) 如果类含有`引用`或`const`类型的成员变量时，则编译器无法合成默认构造函数，也无法合成拷贝构造函数与赋值函数 <br/>

default/delete
> (1) default/delete 只能修饰具有合成版本的成员函数(默认构造函数、拷贝构造函数、移动构造函数、拷贝赋值运算符号、移动赋值运算符、析构函数) <br/>
> (2) 使用default，表示让编译器为相应的成员函数，合成一个默认的版本
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

显示调用构造函数和析构函数
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

右值引用
> (1) 编码过程中，发现有些对象在赋值给其它对象后，会被立即销毁。这种情况下，移动而非拷贝对象，将获得性能的提升. <br/>
> (2) `右值引用`的一个重要特性是 只能绑定到一个将要销毁的对象。因此，可以自由的将一个右值的资源"移动"到另一个对象中. <br/>
> (3) `左值`有持久的状态, `右值`要么是字面常量，要么是在表达式求值的过程中创建的临时对象. `右值引用`只能绑定到临时对象(所引用的对象即将销毁、该对象没有其它用户). 右值引用的代码可以自由接管所引用的对象的资源. <br/>



std::move


移动构造函数 / 移动赋值运算符
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
> (1) `移动构造函数`通常不分配内存，noexcept告诉编译器，本函数不抛异常. <br/>
> (2) 编译器合成`移动构造函数`和`移动赋值函数`的条件:
> > (a) 该类没有定义任何版本的拷贝控制成员（拷贝构造函数、移动构造函数、拷贝赋值函数、移动赋值函数、析构函数）. <br/>
> > (b) 且它的所有非static型数据成员都能移动. <br/>
>
> (3) 若一个类定义了自己的`移动构造函数`或`移动赋值函数`，那么必须自己定义其它的拷贝操作（拷贝构造函数、拷贝赋值函数、析构函数），否则这些拷贝操作默认是被定义为删除的. <br/>


编译器合成函数总结
> 若类中含义`引用`或`const`类型的成员变量时，编译器无法合成`默认构造函数`/`拷贝构造函数`/`拷贝赋值函数` <br/>
> 若类定义了自己的`移动构造函数`或`移动赋值函数`，那么类必须自己定义其它拷贝操作（拷贝构造函数/拷贝赋值函数/析构函数），否则这些拷贝操作默认是被定义为删除的. <br/> 
>
> (1) 合成`默认构造函数`条件: 若类没有定义任何构造函数，则编译器会合成一个默认构造函数. <br/>
>
> (2) 合成`拷贝构造函数`条件: 若类没有定义任何拷贝构造函数，则编译器会合成一个默认的拷贝构造函数. <br/>
> (3) 合成`拷贝赋值函数`条件: 若类没有定义任何拷贝赋值函数，则编译器会合成一个默认的拷贝赋值函数. <br/>
> 
> (4) 合成`移动构造函数`/`移动赋值函数`条件: 
> > (a) 该类没有定义任何版本的拷贝控制函数（拷贝构造函数/移动构造函数/拷贝赋值函数/移动赋值函数/析构函数）.<br/>
> > (b) 且它的所有非static型成员变量都是可移动的. <br/>
>
> (6) 合成`析构函数`条件: 若类未定义任何析构函数，则编译器会合成一个默认的析构函数. <br/>
> 
> (7) 使用`=default`，表示使用编译器默认合成的版本，否则编译器不一定会合成默认版本. <br/>
 

## 重载运算与类型转换

> `::`，`.*`，`.`， `?:` 4个不能被重载 <br/>
> `=`， `[]`， `()`， `->` 4个必须重载为成员函数 <br/>


函数对象 
> (1) 如果一个类重载了函数调用运算符`operator()(args)`，则该类的对象被称作`函数对象` <br/>
> (2) 函数对象常作为范型算法的实参来使用 <br/>
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
}

std::vector<std::string> vec{"aaa", "bbb", "ccc"};
// 生成一个函数对象（未命名的），作为for_each的第3个参数
for_each(vec.begin(), vec.end(), Pnt(std::cout));
```
> (3) lambda也是函数对象
```cpp
std::vector<std::string> vec{"aaa", "bbb", "ccc"};
// 生成一个函数对象（未命名的），作为for_each的第3个参数
for_each(vec.begin(), vec.end(), [](const std::string &s){ std::cout << s << std::endl; });
```
> (4) 标准库定义的函数对象。在C++标准库中，定义了一组表示算术运算符/关系运算符/逻辑运算符的类，每个类分别定义了调用运算符。这些函数对象可以作为范型算法的参数. <br/>
> (5) C++中的几种可调用对象: 函数/函数指针/lambda表达式/bind创建的对象/重载了函数调用运算符的类 <br/>


标准库function类型
> (1) `#include <functional>`，function是模版. <br/>
```cpp
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


类型转换运算符
> (1)  负责将类类型转换为其它类型，原型: `operator type() const`, 不允许带参数. <br/>
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
> (2) 类型转换多发生在隐式转换，可能存在`二义性`. <br/>



## 模版与范型编程

两种常见用法
> 类型参数模版、非类型参数模版
```cpp
// (1) 作为类型参数
template<typename T1, typename T2> 
class Sam1 {
private:
    T1 m1;
    T2 m2;
};

// (2) 作为非类型参数
template <int32_t M, int32_t N>
int32_t cmp(const char (*str1)[M], const char(*str2)[N]) {
    ...
}
```

模版类型别名
```cpp
template<typename T> using twin = std::pair<T, T>;
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


模版类的static成员
> 每个模版类的实例，都拥有自己的`static成员变量` & `static成员函数`，而不是所有的模版类的实例共享. <br/>
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


使用类的类型成员
```cpp
template<typename T> class Foo {
public:
    typename T::value_type get_value() { ... }
};
```
> 默认情况下，C++假定通过"作用域运算符"访问的名字不是类型。若希望访问类型，则需要使用`typename`关键字来显示说明. <br/>


模版特例化
```cpp
template <typename T>          int32_t compare(const T &l, const T& r);

// 模版重载
template <size_t N, size_t M>  int32_t compare(const char(&)[N], const char[&][M]);

// 函数模版特例化
template<> int32_t compare(const char* p1, const char* p2);
```


#  高级主题

tuple
> (1) tuple是元素个数不定的模版，`#include <tuple>`，原型为`tuple<T1, T2, ..., Tn>`，元素的类型可以不同，个数不定. <br/>
> (2) 使用`make_tuple`生成对象. <br/>
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
> (3) 两个tuple相等的条件是: 元素个数相等 且 每个元素内容也相等. <br/>


enum
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
> (1) 枚举属于`字面值常量`类型，C++11加入`限定作用域的枚举类型`. <br/>
> (2) 不限定作用域的声明: `enum Name { ... }` <br/>
> (3) 限定作用域的声明: `enum class Name { ... };` or `enum struct { ... };`，多了`class` or `struct` <br/>
> (4) 一个不限定作用域的枚举类型的对象(或枚举成员)，自动的转化为整型. <br/>
> (5) 限定作用域的enum，不能“隐式”的转换为整形. <br/>


运行时类型识别 - `typeid`运算符
> (1) 原型为`typeid(e)`，e是任意类型的表达式或类型的名字. typeid返回标准库`type_info`(或派生类)对象的引用. <br/>
> (2) 如果e是指向“基类”的指针或引用，且该类含有virtual函数，则需要到“运行时”才会求得值；否则在编译的时候就可以求得. <br/>


运行时类型识别 - `type_info`类
> (1) `#include <typeinfo>`头文件，不同编译器实现的细节不同. <br/>
> (2) 至少包含`t1 == t2` `t1 != t2` `t1.name()` `t1.before(t2)` 4个函数. <br/>


运行时类型识别 - `dynamic_cast`运算符
> (1) `dynamic_cast`运算符，将基类的指针(或引用)，安全的转换成派生类的指针(或引用). `dynamic_cast`的转换依赖虚表，没有virtual函数的类是没有虚表的，在编译时会报错. <br/>
```cpp
dynamic_cast<type* > e; // 指针
dynamic_cast<type& > e; // 引用
dynamic_cast<type&&> e; // e不能是左值
```
> (2) 对于指针类型，若转换失败，返回nullptr; 对于引用类型，若转换失败，抛出`bad_cast`异常. <br/>
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
> (3) 在什么情况下，应该使用`dynamic_cast`替代需虚函数. <br/>


