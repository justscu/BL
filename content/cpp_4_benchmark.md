
### memory barrier / memory fence

为了保证内存访问的串行化，CPU提供了以下指令。其原理就是在访问内存时添加若干延时，保证“此指令以后的内存访问”发生在“此指令以前的内存访问”完成以后。即内存访问不会出现重叠(CPU乱序时).

```cpp
    inline void lfence() {
        __asm__ __volatile__("lfence" ::: "memory"); // load fence, 读串行
    }
    
    inline void sfence() {
        __asm__ __volatile__("sfence" ::: "memory"); // store fence, 写串行
    }
    
    inline void mfence() {
        __asm__ __volatile__("mfence" ::: "memory"); // load & store memory fence, 读写都串行
    }
```

对于多核心处理器，每个核心都有自己的缓存，而这些缓存并非都实时跟内存进行交互。
这样就会出现一个核心上的缓存数据，跟另外一个核心上的缓存数据不一致的问题。
该问题可能会导致程序异常。为了解决这个问题，操作系统提供内存屏障。内存屏障核心作用: <br/>
> (1) 阻止屏障两边的指令重排 <br/>
> (2) 强制把`写缓冲区/高速缓存`中的`脏数据`写回主存，让所有核心缓存中的相应数据失效。即保证所有核心跟内存中最新数据一致，保证数据有效性。<br/>

内存屏障分类 <br/>
> (1) lfence (load fence), 在`读指令前`插入读屏障，让高速缓存中的数据失效，重新从内存中加载数据 <br/>
> (2) sfence (store fence), 在`写指令后`插入写屏障，让写入高速缓存的最新数据都写回到主内存 <br/>
> (3) mfence，同时具有lfence和sfence能立 <br/>

Lock前缀 <br/>
Lock不是内存屏障，但提供了内存屏障类似功能。<br/>
> (1) 先对总线/缓存加锁，再执行后面的指令，再把高速缓存中的脏数据刷回主存，最后释放锁 <br/>
> (2) 通过Lock锁住总线的时候，其余CPU的读写操作都被阻塞，直到锁被释放。Lock锁住总线后的写操作，
会使其余cpu的相关cache line失效。<br/>

### 各基本操作耗时统计

3.4GHZ CPU. 绑核后，每次操作花费时间

|func                  |    O0    |    O3    |     含义      |
|----------------------|---------:|---------:|--------------|
|compare_int32         |   2.13 ns|   0.64 ns| 直接比较==
|memcmp_int32          |   3.14 ns|   2.08 ns| memcmp比较
|compare_int64         |   1.57 ns|   0.64 ns| 直接比较==
|memcmp_int64          |   2.42 ns|   2.36 ns| memcmp比较
|memcpy_random         | 121.53 ns| 115.71 ns| 随机memcpy_4 bytes 
|memcpy_random         | 139.88 ns| 114.19 ns| 随机memcpy_8 bytes 
|memcpy_random         | 135.39 ns| 117.55 ns| 随机memcpy_16 bytes 
|memcpy_random         | 450.64 ns| 428.27 ns| 随机memcpy_1K 
|memcpy_random         | 671.63 ns| 648.86 ns| 随机memcpy_2K 
|memcpy_random         |   1.08 us|   1.05 us| 随机memcpy_4K 
|memcpy_random         |   1.84 us|   1.80 us| 随机memcpy_8K 
|memset_random         | 127.96 ns| 107.68 ns| 随机memset_4 bytes 
|memset_random         | 128.91 ns| 110.89 ns| 随机memset_8 bytes 
|memset_random         | 130.99 ns| 113.15 ns| 随机memset_16 bytes 
|memset_random         | 321.65 ns| 302.09 ns| 随机memset_1K 
|memset_random         | 469.11 ns| 441.43 ns| 随机memset_2K 
|memset_random         | 727.39 ns| 705.96 ns| 随机memset_4K 
|memset_random         |   1.25 us|   1.22 us| 随机memset_8K 
|memset_random         |   2.29 us|   2.25 us| 随机memset_16K 
|                     -|         -|         -|       - |
|add_func              |   3.30 ns|   0.38 ns| 自定义加法 
|add_func_withmutex    |  21.34 ns|  13.72 ns| 自定义加法(with mutex) 
|add_templates         |  15.10 ns|   0.28 ns| 递归加法 
|add_va_args           |  11.08 ns|   2.43 ns| 宏定义加法 
|array_push            |   7.98 ns|   3.54 ns| 数组: 直接赋值 
|array_struct_cast     |   8.23 ns|   3.95 ns| 数组: 转换成struct后再赋值 
|snprintf_cost         | 177.21 ns| 165.27 ns| snprintf耗时 
|int64_add             |   2.13 ns|   0.25 ns| int64  加法 
|double_add            |   2.38 ns|   1.07 ns| double 加法 
|double_mul            |   3.14 ns|   2.14 ns| double 乘法 
|double_div            |   5.67 ns|   4.73 ns| double 除法 
|rdtsc_cost            |   6.34 ns|   6.47 ns| 一次rdtsc耗时
|switch_case_5         |   3.60 ns|   2.45 ns| switch/case_5(分支越多越耗时) 
|if_else_5             |   2.91 ns|   1.63 ns| if/else_5(分支越多越耗时)
|ntoh16                |   2.03 ns|   0.58 ns| ntoh16
|ntoh32                |   1.91 ns|   0.60 ns| ntoh32
|ntoh64                |   2.29 ns|   0.71 ns| ntoh64



### 解析各操作耗时统计

|type  |           function |      O0  |      O3  |     desc |
|------|--------------------|---------:|---------:|----------|
|shl1b | checksum_add       | 598.42 ns|  22.67 ns| 直接累加
|shl1b | checksum_sse       | 159.24 ns|  29.68 ns| SSE
|shl1b | checksum_sse_4loop | 118.15 ns|  23.79 ns| 4路SSE
|shl1b | splite             |  38.45 ns|  24.02 ns| 数据包切割


