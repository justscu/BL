`SIMD`指令在本质上类似一个`向量处理器`，可对控制器上的一组数据（又称“数据向量”）同时分别执行相同的操作从而实现空间上的并行. CPU通过"单指令多数据技术"和"单时钟周期并行处理多个浮点"来有效地提高浮点运算速度.

SIMD发展历程

|    name |  year |  header file |  gcc flag |  message |
|---------|-------|--------------|:----------|---------:
| mmx     | 1997  |   mmintrin.h |     -mmmx | 64位向量, 8个64位寄存器(MM0~MM7), 57条
| sse     | 1999  |  xmmintrin.h |     -msse | 128位向量, 8个128位SSE指令专用寄存器(XMM0~XMM7), 70条
| sse2    | 2000  |  emmintrin.h |    -msse2 |
| sse4    | 2007  |  nmmintrin.h |  -msse4.2 |
| AVX     | 2008  |  avxintrin.h |     -mavx | 256位向量, SandyBridge架构, 8个256位寄存器(YMM0~YMM7)
| AVX2    | 2011  | avx2intrin.h |    -mavx2 | 256位向量, Haswell架构, 16个256位寄存器(YMM0~YMM15), 2个新FMA单元及浮点FMA指令，离散数据加载指令"gather"，新的位移和广播指令 |
| AVX-512 | 2013.7| 512位向量, KnightsLanding/SkyLake架构, AVX512-F, AVX512-ER/CD, AVX512-DQ/BW/VL |

不同代际的指令不要混用，每次状态切换会消耗50-80个指令周期，严重拖慢程序的运行速度.

GCC下，查看`#include <x86intrin.h>`头文件，可以看见使用宏来控制各种头文件。使用何种头文件，需要在makfile中用宏控制。


[intel指令](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm)


### 内存对齐

SIMD指令通常有内存对齐要求。

`void *aligned_alloc(size_t alignment, size_t size);`，申请一块对齐的内存.

`__declspec(align(64)) double input[4] = {1.0, 2.0, 3.0, 4.0};`, 内存64bytes对齐

相比128-bit SSE指令, 256-bit AVX指令的内存对齐要求更高。


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
| ps | pached double precision floating point data
| ss | scalar single precision floating point data
| pd | 里面都是double，即64bit一个数 |
| sd | 数量型double
| epi8/epi16/epi32/epi64 | 里面的数都是整形, 一个整形8bit/16bit/32bit/64bit
| epu8/epu16/epu32/epu64 | 里面的数都是unsinged整形, 一个整形8bit/16bit/32bit/64bit
| si128/si256 | 未指定类型的128/256位向量



