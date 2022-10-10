SIMD: mmx(1996年, 57条) -> sse(1999年, 70条) -> sse3(2000年) -> sse4(2007年) -> AVX(2008年, Sandy Bridge架构, 256位向量) -> AVX2(2011年, 256位向量) -> AVX-512(2013年, 512位向量)

[intel指令](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm)

### 数据类型

|:数据类型  |   描述 |       大小 :|
|---------|--------|-----------|
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


### 函数

函数名称一般为`_mm<bit_width>_<name>_<data_type>`, bit_width表示返回向量的类型(长度); name可以看函数的功能; data_type表示需要计算的数据是什么类型.

data_type相关解释

| data_type | 含义 |
|-----------|------|
| ps | 里面都是float，即32bit一个数 |
| pd | 里面都是double，即64bit一个数 |
| epi8/epi16/epi32/epi64 | 里面的数都是整形, 一个整形8bit/16bit/32bit/64bit
| epu8/epu16/epu32/epu64 | 里面的数都是unsinged整形, 一个整形8bit/16bit/32bit/64bit
| si128/si256 | 不关心向量是啥类型 
