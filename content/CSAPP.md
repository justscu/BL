
###                           CH2 数据的表示

#### 2.1 整数
需要关注扩展、截断、比较(unsigned & signed)、强制转化、运算溢出

结论：*不要使用无符号数*

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
int32_t longer(const char* s1, const char* s2) {
    // 两个unsigned的值相减，结果总>=0.
    return strlen(s1) - strlen(s2) > 0;
}
```
