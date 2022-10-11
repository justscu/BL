SIMD发展历程

|    name |  year |  message |
|---------|-------|----------|
| mmx     | 1996  | 57条
| sse     | 1999  | 70条
| sse3    | 2000  |
| sse4    | 2007  |
| AVX     | 2008  | Sandy Bridge架构, 256位向量
| AVX2    | 2011  | 256位向量
| AVX-512 | 2013  | 512位向量

不同代际的指令不要混用，每次状态切换会消耗50-80个指令周期，严重拖慢程序的运行速度.

GCC下，查看`#include <x86intrin.h>`头文件，可以看见使用宏来控制各种头文件。使用何种头文件，需要在makfile中用宏控制。


[intel指令](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm)





### 数据类型

| 数据类型  |   描述 |       大小 |
|:--------|--------:|-----------:|
| __m128  | 4个float型  | 4 * 32 bit
| __m128d | 2个double型 | 2 * 64 bit
| __m128i | 数个int型    | 128 bit
| __m256  | 8个float型  | 8 * 32 bit
| __m256d | 4个double型 | 4 * 64 bit
| __m256i | 数个int型    | 256 bit
| __m512  | 16个float型  | 16 * 32 bit
| __m512d | 8个double型 | 8 * 64 bit
| __m512i | 数个int型    | 512 bit

`__m128i，__m256i, __m512i`均由整数构成的向量; char/short/int32_t/int64_t/long等均属于整数类型.


```
typedef float __m128 __attribute__ ((__vector_size__ (16), __may_alias__));
typedef long long __m128i __attribute__ ((__vector_size__ (16), __may_alias__));
typedef double __m128d __attribute__ ((__vector_size__ (16), __may_alias__));

typedef float __m256 __attribute__ ((__vector_size__ (32), __may_alias__));
typedef long long __m256i __attribute__ ((__vector_size__ (32), __may_alias__));
typedef double __m256d __attribute__ ((__vector_size__ (32), __may_alias__));
```


### 函数

函数名称一般为`_mm<bit_width>_<name>_<data_type>`, </br>
`bit_width`表示返回向量的类型(长度), 如mm256表示256位, mm表示128位; </br>
`name`可以看函数的功能，包含两部分。第一部分是`具体功能`，如load, sub; 第二部分是`可选修饰符`。</br>
`data_type`表示需要计算的数据是什么类型 </br>

`可选修饰符`相关解释

| 可选修饰符  | 含义 |
|-----------|------|
| u     | 如loadu, 对内存未对齐的数据进行操作
| s     | 如subs,adds, 考虑饱和计算
| h     | 水平方向作计算
| hi/lo | 如mulhi, 高低位
| r     | 如setr, 逆序
| fm    | 如fmadd


`data_type`相关解释

| data_type | 含义 |
|-----------|------|
| ps | 里面都是float，即32bit一个数 |
| pd | 里面都是double，即64bit一个数 |
| ss | 数量型float
| sd | 数量型double
| epi8/epi16/epi32/epi64 | 里面的数都是整形, 一个整形8bit/16bit/32bit/64bit
| epu8/epu16/epu32/epu64 | 里面的数都是unsinged整形, 一个整形8bit/16bit/32bit/64bit
| si128/si256 | 未指定类型的128/256位向量
