`SIMD`指令在本质上类似一个`向量处理器`，可对控制器上的一组数据（又称"数据向量"）同时分别执行相同的操作从而实现空间上的并行.
CPU通过"单指令多数据技术"和"单时钟周期并行处理多个浮点"来有效地提高浮点运算速度.

SIMD发展历程

|    name |  year |  header file |  gcc flag |  message |
|---------|-------|--------------|:----------|---------:
| mmx     | 1997  |   mmintrin.h |     -mmmx | 64位向量, 8个64位寄存器(MM0~MM7), 57条
| sse     | 1999  |  xmmintrin.h |     -msse | 128位向量, 8个128位SSE指令专用寄存器(XMM0~XMM7), 70条
| sse2    | 2000  |  emmintrin.h |    -msse2 |
| sse4    | 2007  |  nmmintrin.h |  -msse4.2 |
| AVX     | 2008  |  avxintrin.h |     -mavx | 256位向量, SandyBridge架构, 8个256位寄存器(YMM0~YMM7)
| AVX2    | 2011  | avx2intrin.h |    -mavx2 | 256位向量, Haswell架构, 16个256位寄存器(YMM0~YMM15), 2个新FMA单元及浮点FMA指令，离散数据加载指令"gather"，新的位移和广播指令 |
| AVX-512 | 2013.7|              |           | 512位向量, KnightsLanding/SkyLake架构, 16个512位寄存器(ZMM0~ZMM15) AVX512-F, AVX512-ER/CD, AVX512-DQ/BW/VL |

不同代际的指令不要混用，每次状态切换会消耗50-80个指令周期，严重拖慢程序的运行速度.

GCC下，查看`#include <x86intrin.h>`头文件，可以看见使用宏来控制各种头文件。使用何种头文件，需要在makfile中用宏控制。

查看本机支持哪些指令: `gcc -dM -E -march=native - < /dev/null | grep -E "SSE|AVX" | sort`
```
#define __AVX__ 1
#define __AVX2__ 1
#define __SSE__ 1
#define __SSE2__ 1
#define __SSE2_MATH__ 1
#define __SSE3__ 1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#define __SSE_MATH__ 1
#define __SSSE3__ 1
```

[intel指令](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm)


### 内存对齐

SIMD指令通常有内存对齐要求。

`void *aligned_alloc(size_t alignment, size_t size);`，`aligned_free(void *)` 申请/释放一块对齐的内存.

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

| 可选修饰符  | meaning |
|-----------|----------|
| u     | 如loadu, 对内存未对齐的数据进行操作
| s     | 如subs,adds, 考虑饱和计算
| h     | 水平方向作计算
| hi/lo | 如mulhi, 高低位
| r     | 如setr, 逆序
| fm    | 如fmadd


`data_type`相关解释, execution mode(s=scalar, p=packed).

| data_type | meaning |
|-----------|---------|
| ps | packed `double precision floating` point data
| ss | scalar `single precision floating` point data
| pd | 里面都是double，即64bit一个数 |
| sd | 数量型double
| epi8/epi16/epi32/epi64 | 里面的数都是整形, 一个整形8bit/16bit/32bit/64bit
| epu8/epu16/epu32/epu64 | 里面的数都是unsinged整形, 一个整形8bit/16bit/32bit/64bit
| si128/si256 | 未指定类型的128/256位向量


#### SSE相关指令分类

(1) `Load`，把数据加载到寄存器，多数需要内存对齐

函数名到数第2个字母, 's'表示scalar, 'p'表示packed
```
__m128 _mm_load_ss (float *p); # 加载一个float到寄存器的最低字节，寄存器其余置0. r0=*p, r1=r2=r3=0.
__m128 _mm_load_ps (float *p); # 加载4个float到寄存器，要求p内存16字节对齐. r0=p[0], r1=p[1], r2=p[2], r3=p[3].
__m128 _mm_load1_ps(float *p); # 将一个float的值，加载到4个寄存器，r0=r1=r2=r3=*p;
__m128 _mm_loadh_pi(__m128 a, __m64 *p);
__m128 _mm_loadl_pi(__m128 a, __m64 *p);
__m128 _mm_loadr_ps(float *p); # r表示逆序加载, r3=p[0], r2=p[1], r1=p[2], r0=p[3]
__m128 _mm_loadu_ps(float *p); # u表示不需要16字节对齐
```

(2) `Set`，把数据加载到寄存器，多数不需要内存对齐
```
__m128 _mm_set_ss(float a); # 加载一个float到寄存器的最低字节，寄存器其余置0. r0=a, r1=r2=r3=0.
__m128 _mm_set_ps(float a, float b, float c, float d); # 加载4个float到寄存器，r0=a, r1=b, r2=c, r3=d.
__m128 _mm_set1_ps(float a); # 将一个float的值，加载到4个寄存器，r0=r1=r2=r3=a;
__m128 _mm_setr_ps(float a, float b, float c, float d); # r表示逆序加载, r3=a, r2=b, r1=c, r0=d
__m128 _mm_setzero_ps(); # 清0操作, r0=r1=r2=r3=0

```

(3) `Store`，把寄存器中的值，保存到内存，可以当作Load的逆擦作
```
void _mm_store_ss (float *dest, __m128 r); # 保存一个float到dest[0], 即dest[0] = r0. dest[1] = dest2[2] = dest3[3] = 0.
void _mm_store_ps (float *dest, __m128 r); # 保存4个float. dest[0] = r0, dest[1] = r1, dest[2] = r2, dest[3] = r3.
void _mm_store1_ps(float *dest, __m128 r); # dest[0] = dest[1] = dest2[2] = dest3[3] = r0.
void _mm_storeh_pi(__m64 *dest, __m128 r);
void _mm_storel_pi(__m64 *dest, __m128 r);
void _mm_storer_ps(float *dest, __m128 r); # dest[3] = r0, dest[2] = r1, dest[1] = r2, dest[0] = r3
void _mm_storeu_ps(float *dest, __m128 r); # u表示不需要16字节对齐
void _mm_stream_ps(float *dest, __m128 r); # 只修改一个float的值，其余值不动.
```

(4) `算术运算`, 加法、减法、乘法、除法、开方、最大值、最小值、近似求倒数、求开方的倒数等
```
__m128 _mm_add_ss(__m128 a, __m128 b); # 倒数第2个字母: `s`表示 scalar, `p`表示packed. 最后一个字母`s`=single float. `d`=double float.
__m128 _mm_add_ps(__m128 a, __m128 b);
```

#### AVX相关指令分类
(1) `Set`，需要内存对齐
```
_mm256_setzero_ps/pd  # `p`表示packed, `s`表示singled float. `d`表示double
_mm256_setzero_si256  # 整形数据
_mm256_set1_ps/pd     # 用一个float填充所有
_mm256_set1_epi8/epi16/epi32/epi64 # 用一个整形填充所有 
_mm256_set_ps/pd
_mm256_set_epi8/epi16/epi32/epi64
_mm256_set_m128/m128d/m128i
_mm256_setr_ps/pd                   # 逆序加载
_mm256_setr_epi8/epi16/epi32/epi64  # 逆序加载
```

(2) `Load`，把数据从内存加载到寄存器
```
_mm256_load_ps/pd        # `p`表示packed, `s`表示singled float. `d`表示double
_mm256_load_si256        # 整形数据
_mm256_loadu_ps/pd       # 从未对齐内存中加载
_mm256_loadu_si256       # 从未对齐内存中加载
_mm256_maskload_ps/pd    # 根据掩码加载
_mm256_maskload_epi32/64 # 根据掩码加载

```

(3) `加减`
```
_mm256_add_ps/pd，         _mm256_sub_ps/pd          # `p`表示packed, `s`表示singled float. `d`表示double
_mm256_add_epi8/16/32/64， _mm256_sub_epi8/16/32/64
_mm256_adds_epi8/16,       _mm256_subs_epi8/16       # `s`考虑内存饱和度
_mm256_adds_epu8/16,       _mm256_subs_epu8/16
_mm256_hadd_ps/pd,         _mm256_hsub_ps/pd         # `h`水平方向上对向量做加减法
_mm256_hadd_epi16/32,      _mm256_hsub_epi16/32      # `h`水平方向上对向量做加减法
_mm256_hadds_epi16,        _mm256_hsubs_epi16
_mm256_addsub_ps/pd                                  # 偶数位减，奇数位加
```

(4) `乘除`
```
_mm256_mul_ps/pd
_mm256_mul_epi32/epu32
_mm256_mullo_epi16/32       # Multiply integers and store low halves
_mm256_mulhi_epi16/epu16    # Multiply integers and store high halves
_mm256_mulhrs_epi16         # Multiply 16-bit elements to form 32-bit elements
_mm256_div_ps/pd            # 两个float类型的向量做除法
```

(5) `融合乘除/加减`
```
_mm256_fmadd_ps/pd,    _mm256_fmsub_ps/pd    # result = a*b + c
_mm256_fnmadd_ps/pd,   _mm256_fnmsub_ps/pd   # result = -(a*b) + c
__m256 _mm256_fmaddsub_ps/pd(__m256 a, __m256 b, __m256 c) 
__m256 _mm256_fmsubadd_ps/pd(__m256 a, __m256 b, __m256 c) 
```

(6) `排列和洗牌`
