I   C++基础知识
==
2 变量与基本类型
=====

如何选择数据类型
> (1)明确知道数据值不为负时，选择unsigned <br/>
> (2)在存放数据时才使用char/bool，在计算时不要使用char/bool。char的类型是未知的，可能为signed, 也可能为unsigned。若实在需要char类型参与计算，可以明确为signed char, 或unsigned char。<br/>
> (3)执行浮点数时，使用double，不要使用float。有些机器，double的速度比float还要快。<br/>


类型转换
> (1) 给signed变量赋值一个超出其表示范围的值时，结果是为定义的（可能继续工作/崩溃/垃圾数据）<br/>
> (2) `不要混用带符号的变量和无符号的变量`<br/>
> (3) 显式类型转换 `cast-name<type> (expression)`
> > (a) static_cast, 任何具有明确定义的类型转换，只要不包含底层const，都可以使用static_cast. <br/>
> > (b) const_cast, 去const，但只能改变运算对象的底层const. <br/>
> > (c) reinterpret_cast, <br/>
> > (d) dynamic_cast, <br/>

指针与const
>  （1）指向常量的指针(pointer to const)
```cpp
double d1 = 3.14, d2 = 2.24;
const double *pval = &d1; // const修饰*pval. low-level const
*pval = 3.2; // error，（1）不能通过指针修改所指对象的值
pval = &d2;  // ok，（2）修改指针所指向的地址
```
> （2）常量指针(const pointer)，在定义的时候必须被初始化，不能再指向其它地址
```cpp
double d1 = 3.14, d2 = 2.24;
double * const pval = &d1; // const修饰pval. top-level const
*pval = 3.2; // ok,
pval = &d2;  // error,
```

常量表达式
> （1）`常量表达式`是指值不会改变，且在`编译阶段`就须得到计算结果的表达式(非运行阶段)。
```cpp
const int32_t a = 5;
const int32_t b = a + 1; // a，b均为常量表达式
const int32_t c = current_time(); // c需要在运行中才能得到结果，非常量表达式
```
> （2）C++11允许将变量声明为`constexpr`类型，这样编译器在 编译阶段 就检查该值是否为常量表达式。
```cpp
constexpr int32_t a = 5;      // 5 是常量表达式
constexpr int32_t b = a + 1;  // a+1是常量表达式
constexpr int32_t c = size(); // 要求size()是一个constexpr函数时，才可以通过编译
```
> (3)在`constexpr`声明中如果定义了一个指针，则`constexpr`仅对指针有效，与指针所指的对象无关
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
typedef int32_t Num;
// base, Num都是double的同义词； Pointer 是double*的同义词
typedef Num base, *Pointer;

// typedef 与 const一起使用
char c = 'A';
typdef char *pstring;

// const 修饰cstr，说明cstr的值不能被改变. 同时cstr是一个指向char的指针
const pstring cstr = &c; 
// const 修饰*ps。ps是指针，不能通过ps来修改所指地址中的值，该地址中的值也是一个指针
const pstring *ps; 
```
> (2)使用关键字using
```cpp
using Num = int32_t; // Num是int32_t的同义词
Num c = 5;
```


auto 类型说明符
> (1) C++11引入auto，在编译阶段由编译器根据初始值来推导类型，因此，auto定义的变量必须有初始值。
```cpp
auto item = V1 + V2;  // 根据V1+V2的结果，来决定item的类型
auto i = 0, *p = &i;  // ok.
auto j = 0, pi = 3.14; // error. j 和pi的类型不同
```
> (2) 以引用对象的类型作为auto类型
```cpp
int32_t i = 5, &j = i;
auto a = j; // 等价于int32_t a = 5;
```
> (3)auto会去掉顶层const，保留底层const



decltype 类型指示符
> (1) decltype的作用是选择并返回操作数的数据类型。在编译阶段分析表达式并得到其类型，并不去计算表达式的值 <br/>
```cpp
const int32_t i = 0, &j = i, *p = &i;
decltype(i) x = 0; // 等价于 const int32_t x = 0;
decltype(j) y = x; // 等价于 const int32_t &y = x; decltype(j)是引用类型
decltype(j) z;     // 等价于 const int32_t &z; error, 引用必须初始化
decltype(*p) m;    // 等价于 const int32_t &m; error, 解引用
```
> (2) decltype使用的是表达式的话，需要根据表达式的结果来推测对应的类型 <br/>
> (3) decltype使用的表达式是解引用`decltype(*p)`，则decltype得到引用类型 <br/>
> (4) decltype使用的表达式加括号与不加括号意义不同，加括号就是一个表达式，就会变成引用 <br/>
```cpp
int32_t i = 0;
decltype(i)   x; // ok.
decltype((i)) y; // error. 两层括号，变成引用，等价于 int32_t &y;
```
> (5) 赋值是会产生引用的一类表达式，引用的类型就是左值的类型。
```cpp
int32_t i = 5, j = 10;
decltype(i)   a = i; // int32_t  a = i;
decltype(i=j) b = a; // int32_t &b = a; 表达式i=j的类型是 int32_t&
```

3 string, vector & array
=====

命名空间using
> 每个名字需要独立的using声明 <br/> 
> 头文件中不应该包含using声明。因为会被其它文件中引用，造成名字冲突


直接初始化与拷贝初始化
> 使用等号（=）来初始化一个变量，实际执行的是`拷贝初始化(copy initialization)`, 编译器把等号右侧的值拷贝到新创建的对象中去 <br/>
> 若不使用等号，则执行的是直接初始化(direct initialization)
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


数组与指针
> (1) 在很多用到数组名字的地方，编译器会自动地把名字替换为一个指向数组首元素的指针
```cpp
int32_t iarr[] = {0, 1, 2, 3, 4};
int32_t    *p1 = iarr; // 等价于 int32_t *p1 = &(iarr[0]);

auto     p2(iarr); // 等价与 int32_t *p2 = &(iarr[0]); auto p2(&(iarr[0]));
decltype(iarr) p3; // 等价于 int32_t p3[10];
```
> (2) 数组名作为一个auto变量的初始值时，推断出来的类型是指针而非数组 <br/>
> (3) 数组名作为decltype的表达式时，返回的类型是数组 



4 表达式
=====

lvalue & rvalue
> (1) C++11中所有的值(表达式)必属于左值/右值之一; <br/>
> (2) 当一个对象被当作`右值`的时候，用的是对象的值（内容）；当对象被用作`左值`的时候，用的是对象的身份（在内存中的位置）；<br/>
> (3) 可以取地址的且有名字的就是`左值`; 不能取地址的、没有名字的就是`右值`; <br/>
> (4) C++11中的右值，又分为`纯右值`(prvalue, Pure Rvalue) 和 `将亡值`: <br/>
> > `纯右值`指临时变量和不跟对象关联的字面量值；<br/>
> > `将亡值`是C++11新增的跟右值引用相关的表达式，这样的表达式通常是将要被移动的对象（移为他用），比如返回右值引用T&&的函数返回值、std::move的返回值、转换为T&&的类型转换函数的返回值; <br/>
> > 将亡值可以理解为通过“盗取”其它变量内存空间的方式获取到的值。在确保其它变量不再被使用、或即将被销毁时，通过“盗取”的方式可以避免内存空间的释放和分配，延长变量的生命期. <br/>


类型转换
> 算数类型转换
> > (1) `整形提升`，计算时，把小整数类型转扩展成较大的整数类型; <br/>
> > (2) `无符号类型`
> > > (a) 若都为`无符号类型`，则扩张成较大类型；<br/>
> > > (b) 若`无符号类型`不小于`带符号类型`，则带符号的会先转成无符号(注意扩展的副作用)；<br/>
> > > (c) 若`无符号类型`小于`带符号类型`，行为不定，依赖机器；<br/>

6 函数
=====

含有可变形参的函数
> (1) initializer_list形参
```cpp
// sum
int32_t sum(initializer_list<int32_t> args) {
    int32_t ret = 0;
    for (auto it : args) {
        ret += *it;
    }
    return ret;
}

std::cout << sum({2, 5}) << std::endl;
std::cout << sum({2, 5, 8}) << std::endl;
```
> > (a) initializer_list是标准库类型(类模版)，用于表示某种特定类型的值的数组. 
> > (b) initializer_list中的元素都是常量值，无法改变其元素的值 <br/>
> > (c) initializer_list中所有元素的类型都相同 <br/>

> (2) 省略符形参
```cpp
int32_t sum(int32_t cnt, ...);
```


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

II  C++标准库
==

9 顺序容器
=====

顺序容器类型

名称|特征|访问|插入/删除|备注|
:---:|----|----|---------|----|
vector      |可变大小数组|支持随机访问/下标访问、速度快  | 支持随机插入/删除, 但速度慢 | 中间插入时，需要移动后面的元素  |


array       |固定大小数组|支持随机访问/下标访问，速度快  | 不能添加或删除元素          | 数组大小固定，不能改变大小      |
list        |双向链表    |只支持双向[顺序]访问           | 插入/删除速度快             |                                 |
forward_list|单向链表    |只支持单向[顺序]访问           | 插入/删除速度快             |                                 |
deque       |双端队列    |支持快速随机访问               | 在头尾插入/删除速度快       |                                 |



III 设计类
==



IV  高级主题
==
