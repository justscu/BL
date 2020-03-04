I   C++基础知识
==================

如何选择数据类型
> (1)明确知道数据值不为负时，选择unsigned <br/>
> (2)在存放数据时才使用char/bool，在计算时不要使用char/bool。char的类型是未知的，可能为signed, 也可能为unsigned。若实在需要char类型参与计算，可以明确为signed char, 或unsigned char。<br/>
> (3)执行浮点数时，使用double，不要使用float。有些机器，double的速度比float还要快。<br/>

类型转换
> (1) 给signed变量赋值一个超出其表示范围的值时，结果是为定义的（可能继续工作/崩溃/垃圾数据）<br/>
> (2)`不要混用带符号的变量和无符号的变量`<br/>

常量表达式
>（1）`常量表达式`是指值不会改变，且在`编译阶段`就须得到计算结果的表达式(非运行阶段)。
```cpp
const int32_t a = 5;
const int32_t b = a + 1; // a，b均为常量表达式
const int32_t c = current_time(); // c需要在运行中才能得到结果，非常量表达式
```
>（2）C++11允许将变量声明为`constexpr`类型，这样编译器在 编译阶段 就检查该值是否为常量表达式。
```cpp
constexpr int32_t a = 5;      // 5 是常量表达式
constexpr int32_t b = a + 1;  // a+1是常量表达式
constexpr int32_t c = size(); // 要求size()是一个constexpr函数时，才可以通过编译
```
```cpp
int32_t i = 5, j = 10;

const int32_t *p1 = &i;
int32_t const *p2 = &i; // p1, p2等价，指向一个常量（或变量）的地址
*p1 = 10; // error， 不能通过该指针对变量赋值
p1 = &j;  // ok, 但可以指向新的地址

int32_t * const p3 = &i; // p3 指向i的地址，不能再指向其它地方了，但可以 *p3 = 1 来修改值
*p3 = 1; // ok
p3 = &j  // error
```
(3)在`constexpr`声明中如果定义了一个指针，则`constexpr`仅对指针有效，与指针所指的对象无关
```cpp
int32_t i = 5;
// p1为指向常量的指针，不能通过p1来修改对象的值，但p1可以再指向其它地址
const int32_t *p1 = &i;
// p2为指向整数的常量指针，p2不能再指向其它地方。
// 同时，要求i的地址在编译阶段就要确定，即i是一个全局变量，或者静态局部变量
constexpr int32_t *p2 = &i;
```


II  C++标准库
==================


III 设计类
==================



IV  高级主题
==================
