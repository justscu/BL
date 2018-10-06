
###                           CH2 数据的表示

#### 2.1 整数
需要关注扩展、截断、比较(unsigned & signed)、强制转化、运算溢出

结论：**不要使用无符号数**

```cpp
bool b1 = (a > b); b2 = (a-b > 0);
// 当a和b为signed时，返回的值一致；
// 但都为unsigned时，返回的值可能不一致。存在潜在的bug.
```

`有符号整数`(unsigned)，在C语言中用补码表示（符号位不变；负整数其余位按位取反，再加1；正整数原码补码相同）。
扩展时，只扩展符号位，如(int16_t扩展为int32_t)。
截断时，原来的正数可能会变成负数；负数也可能变成正数。
比较时，先把signed变成unsigned，再比较。
强制转化时，只是改变了数据的含义（bit值并不变）。

`无符号整数`(signed)，在C语言中用补码表示(就是其本身)。
扩展时，在高位加0。
截断时，直接去掉高位。
比较时，signed会先变成unsigned，再比较。
强制转化时，只是改变了数据的含义（bit值并不变）。
size_t的类型为uint32_t or uint64_t。

```cpp
int16_t a;
uint32_t b;
// a的变化过程: int16_t -> int32_t -> uint32_t
if (a < b) {
}
```

```cpp
float sum(float a[], uint32_t len) {
    float ret = 0.0;
    // error. i会隐式转化为unsigned，若len=0，则len-1是一个较大的数，会数组越界
    for (int32_t i = 0; i < len-1; ++i) {
        ret += a[i];
    }
}
```

```cpp
size_t strlen(const char* src);
bool longer(const char* s1, const char* s2) {
    // 两个unsigned的值相减，结果总>=0.
    return (strlen(s1) - strlen(s2)) > 0;
}
```

#### 2.2 强制转换示例
```cpp
// (1) char -> int32_t
char*    p1; // 先扩展成int32_t
int32_t* p2;
*p2 = *p1;

movzbl (%rax), %eax
movsbl    %al, (%rdx) // 有符号扩展

// (2) char -> uint32_t
char*     p1; // 先扩展成int32_t
uint32_t* p2;
*p2 = *p1;

movzbl (%rax), %eax
movsbl %al, (%rdx) // 有符号扩展

// (3) uchar -> int32_t
uchar* p1; // 先扩展成uint32_t
int32_t* p2;
*p2 = *p1;

movzbl (%rax), %eax
movzbl %al, (%rdx)

// (4) uchar -> uint32_t
uchar* p1;  // 先扩展成uint32_t
uint32_t* p2;
*p2 = *p1;

movzbl (%rax), %eax
movzbl %al, (%rdx)

// (5) int32_t -> char
int32_t* p1;
char* p2;
*p2 = *p1;

movl (%rax), %eax
movb %al, (%rdx)

// (6) int32_t -> uchar
int32_t* p1;
uchar*   p2;
*p2 = *p1;

movl (%rax), %eax
movb %al, (%rdx)
```

#### 2.3 浮点数
内存表示

  类型|占用字节|符号(bit)|指数(bit)|尾数(bit)|精度(折算成10进制)|指数范围(折算成10进制)|
-----:|-------:|--------:|--------:|--------:|-----------------:|---------------------:|
float |      4 |       1 |       8 |      23 |    2^23, 6-7位   |          [-127, 128] |
double|      8 |       1 |      11 |      52 |    2^52, 15-16位 |        [-1023, 1024] |

浮点数与整数间转换，可能会有溢出/精度损失（要搞懂精度损失的原因，以及溢出的原因）.
> int32_t 转换为float，会有精度损失，但不会有溢出 <br/>
> int32_t 转换为double，不会有精度损失，也不会溢出 <br/>
> float 转换为int32_t, 可能会溢出 <br/>


