### 各基本操作耗时统计

3.4GHZ CPU. 每次操作花费时间

|func                  |    O0    |    O3    |     含义      |
|----------------------|---------:|---------:|--------------|
|compare_int32         |   1.96 ns|   0.66 ns| 直接比较==
|memcmp_int32          |   3.17 ns|   2.08 ns| memcmp比较
|compare_int64         |   2.39 ns|   0.60 ns| 直接比较==
|memcmp_int64          |   2.41 ns|   2.36 ns| memcmp比较
|memcmp                |   1.95 ns|   1.71 ns| memcmp比较(5 bytes)
|StrNCmp               |   1.35 ns|   0.73 ns| 按位异或(5 bytes)
|assign_int32          |   2.29 ns|   0.69 ns| (顺序) 直接赋值int32 
|memcpy_int32          |   3.20 ns|   1.13 ns| (顺序) memcpy_int32 
|assign_int64          |   2.68 ns|   1.16 ns| (顺序) 直接赋值int64 
|memcpy_int64          |   3.31 ns|   1.56 ns| (顺序) memcpy_int64 
|memcpy_1K             | 144.93 ns| 144.03 ns| (顺序)memcpy_1K 
|memcpy_4K             | 488.03 ns| 479.64 ns| (顺序)memcpy_4K 
|memset_1K             | 138.58 ns| 139.85 ns| (顺序)memset_1K 
|memset_4K             | 475.97 ns| 478.47 ns| (顺序)memset_4K 
|memcpy_random         | 121.53 ns| 115.71 ns| 随机memcpy_4 bytes 
|memcpy_random         | 139.88 ns| 114.19 ns| 随机memcpy_8 bytes 
|memcpy_random         | 450.64 ns| 428.27 ns| 随机memcpy_1K 
|memcpy_random         |   1.08 us|   1.05 us| 随机memcpy_4K 
|memset_random         | 127.96 ns| 107.68 ns| 随机memset_4 bytes 
|memset_random         | 128.91 ns| 110.89 ns| 随机memset_8 bytes 
|memset_random         | 321.65 ns| 302.09 ns| 随机memset_1K 
|memset_random         | 727.39 ns| 705.96 ns| 随机memset_4K 
|                     -|         -|         -|       - |
|add_func              |   3.30 ns|   0.38 ns| 自定义加法 
|add_func_withmutex    |  21.34 ns|  13.72 ns| 自定义加法(with mutex) 
|add_templates         |  15.10 ns|   0.28 ns| 递归加法 
|add_va_args           |  11.08 ns|   2.43 ns| 宏定义加法 
|array_push            |   7.98 ns|   3.54 ns| 数组: 直接赋值 
|array_struct_cast     |   8.23 ns|   3.95 ns| 数组: 转换成struct后再赋值 
|snprintf_cost         | 177.21 ns| 165.27 ns| snprintf耗时 
|int64_add             |   2.13 ns|   0.25 ns| int64 加法 
|int64_mul             |   1.91 ns|   0.79 ns| int64 乘法 
|int64_div             |   2.23 ns|   0.99 ns| int64 除法 
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

| type  |              function |      O0  |      O3  |     desc |
|:-----:|-----------------------|---------:|---------:|----------|
|base   | std_vector_interat    |   7.33 ns|   0.47 ns| std::vector<int64_t> 遍历
|base   |  hashtable_find       |  22.36 ns|  13.00 ns|  HashTable 查找
|base   | std_unorderedmap_find | 120.87 ns|  23.78 ns| std::unorderedmap 查找
|base   |    std_map_find       | 257.85 ns|  83.72 ns| std::map<> 查找
|base   |    XQueue_insert      |  40.96 ns|  39.09 ns| XQueue 插入
|       |                       |          |          |
|       |    memcpy_MD          |  17.33 ns|  16.40 ns| (热内存)直接拷贝768字节,96字段
|       |    assign_index       |   8.57 ns|   6.54 ns| (热内存)赋值13个字段
|       |    assign_stock       |  17.58 ns|  11.81 ns| (热内存)赋值55个字段
|       |    assign_option      |  17.62 ns|   8.10 ns| (热内存)赋值39个字段
|       |                       |          |          |
|sh1    |    checksum_add       | 598.42 ns|  22.67 ns| 直接累加
|sh1    |    checksum_sse       | 159.24 ns|  29.68 ns| SSE
|sh1    |    checksum_sse_4loop | 118.15 ns|  23.79 ns| 4路SSE
|sh1    |    splite_fb          |  37.05 ns|  22.73 ns| FB切割数据包
|sh1    |    splite             |  23.06 ns|  24.58 ns| 直接切割数据包
|sh1    |    decode             | 169.35 ns|  43.73 ns| 直接解码(含memset)
|       |                       |          |          |
|sh     |    split              |  59.62 ns|  51.97 ns| 只切割不取数据
|sh     |    split_if           | 220.89 ns| 148.60 ns| 切割且取数据(if)
|sh     |    split_if_else      | 277.85 ns| 212.21 ns| 切割且取数据(if ... else ...)
|sh     |    split_fb           | 343.24 ns| 247.05 ns| 切割且取数据(fb)
|sh     |    checksum_add       |   2.81 us|  92.55 ns| 直接累加
|sh     |    checksum_sse       | 690.07 ns| 107.98 ns| SSE
|sh     |    checksum_sse_4loop | 407.50 ns| 101.55 ns| 4路SSE
|sh     |    parse_md           | 803.06 ns| 376.59 ns| 单条
|sh     |    parse_idx          | 140.86 ns|  54.75 ns| 单条
|sh     |    parse_opt          |          |        ns| 单条
|sh     |    parse_e            | 129.89 ns|  53.08 ns| 单条
|sh     |    parse_t            | 140.48 ns|  57.62 ns| 单条
|       |                       |          |          |
|sz     |    splite             |   9.95 ns|   8.17 ns|
|sz     |    splite_fb          |  29.51 ns|  16.70 ns|
|sz     |    checksum_add       | 250.69 ns|  14.62 ns|
|sz     |    checksum_sse       |  74.11 ns|  16.12 ns|
|sz     |    checksum_sse_4loop |  86.03 ns|  19.06 ns|
|sz     |    parse_md           | 221.28 ns|  94.26 ns|
|sz     |    parse_idx          |  61.65 ns|  32.43 ns|
|sz     |    parse_ent          |  30.71 ns|   9.67 ns|
|sz     |    parse_trd          |  36.46 ns|  13.01 ns|



| volatile | mfence | cacheline | O3 (ns) |
|:--------:|:------:|:---------:|--------:|
|        n |      n |         n |      22 |
|        y |      n |         n |      20 |
|        n |      y |         n |      53 |
|        n |      n |         y |      12 |
|        n |      y |         y |      45 |
|        y |      y |         y |      46 |

alignas(kCacheLineSize), 能降低延时; mfence，会显著增加延时.


### socket cost

测试环境：o3, Intel(R) Xeon(R) Gold 6256 CPU @ 3.60GHz, Solarflare Communications XtremeScale SFC9250. <br/>
IP头部长度为[20, 60]字节, TCP头部长度为[20, 60]字节, UDP头部长度固定8字节. MTU=1500.

结论：包的总长度尽量接近且不超过MTU时，效率最高. SF onload模式下，发小包的效率也挺高.

下图中的长度为UDP载荷的长度

|     type  |       function |     128 |     256 |     512 |    1024 |    1420 |    1500 |    2048 |    4096 |
|:---------:|----------------|--------:|--------:|--------:|--------:|--------:|--------:|--------:|--------:|
|    lo     | multicast send | 1.46 us | 1.47 us | 1.47 us | 1.49 us | 1.49 us | 1.49 us | 1.50 us | 1.55 us |
|    SF     | multicast send | 1.84 us | 1.86 us | 1.86 us | 1.87 us | 1.88 us | 2.87 us | 2.89 us | 3.75 us |
| SF_onload | multicast send |  435 ns |  504 ns |  461 ns |  871 ns | 1.19 us | 1.54 us | 1.98 us | 3.42 us |


### queue cost

测试环境: o2, 11th Gen Intel(R) Core(TM) i7-11700 @ 2.50GHz. <br/>
一写一读的速度明显快很多，实现上也更容易.

| type | queue size | Throughput(in, W/s) | Throughput(out, W/s) | Latency(ns) | tips |
|:----:|------------|--------------------:|---------------------:|------------:|------|
| cycle|  1024*1024 |      2 ns,   5 WW/s |       2 ns,   5 WW/s |        2 ns |
| SPSC |  1024*1024 |     11 ns, 9000 W/s |      11 ns, 9000 W/s |       74 ns |
| SPSC1|  1024*1024 |     58 ns, 1724 W/s |      58 ns, 1724 W/s |       84 ns |
| MPSC |  1024*1024 |    360 ns,  278 W/s |     120 ns,  833 W/s |      233 ns | 2P1C |
| MPMC |  1024*1024 |    660 ns,  152 W/s |     500 ns,  200 W/s |      251 ns | 4P2C |
