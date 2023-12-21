### 各基本操作耗时统计

CPU: Intel(R) Xeon(R) Gold 6256 CPU @ 3.60GHz, 睿频到4.3GHz， 每次操作花费时间

|func                  |    O0    |    O2    |     含义      |
|----------------------|---------:|---------:|--------------|
|compare_int32         |   2.74 ns|   0.82 ns| 直接比较==
|memcmp_int32          |   4.24 ns|   3.60 ns| memcmp比较
|compare_int64         |   2.71 ns|   2.14 ns| 直接比较==
|memcmp_int64          |   4.35 ns|   4.69 ns| memcmp比较
|memcmp                |   2.57 ns|   1.71 ns| memcmp比较(5 bytes)
|StrNCmp               |   4.07 ns|   0.92 ns| 按位异或(5 bytes)
|                    - |        - |        - |
|assign_int32          |   3.29 ns|   1.90 ns| (顺序) 直接赋值int32 
|memcpy_int32          |   3.84 ns|   1.88 ns| (顺序) memcpy_int32 
|assign_int64          |   4.76 ns|   3.10 ns| (顺序) 直接赋值int64 
|memcpy_int64          |   5.56 ns|   3.54 ns| (顺序) memcpy_int64
|                    - |        - |        - |
|memcpy_1K             | 442.60 ns| 441.96 ns| (顺序)memcpy_1K 
|memcpy_4K             |   1.75 us|   1.75 us| (顺序)memcpy_4K 
|memset_1K             | 412.19 ns| 448.53 ns| (顺序)memset_1K 
|memset_4K             |   1.73 us|   1.65 us| (顺序)memset_4K 
|memcpy_random         | 510.60 ns| 509.42 ns| 随机memcpy_4 bytes 
|memcpy_random         | 504.93 ns| 503.39 ns| 随机memcpy_8 bytes 
|memcpy_random         | 796.43 ns| 797.19 ns| 随机memcpy_1K 
|memcpy_random         |   1.24 us|   1.24 us| 随机memcpy_4K 
|memset_random         | 415.32 ns| 415.15 ns| 随机memset_4 bytes 
|memset_random         | 415.58 ns| 414.22 ns| 随机memset_8 bytes 
|memset_random         | 627.57 ns| 630.41 ns| 随机memset_1K 
|memset_random         | 943.75 ns| 955.18 ns| 随机memset_4K 
|                    - |        - |        - |
|add_func              |   3.30 ns|   0.24 ns| 自定义加法 
|add_func_withmutex    |  21.34 ns|  12.60 ns| 自定义加法(with mutex) 
|add_templates         |  15.10 ns|   0.23 ns| 递归加法 
|add_va_args           |  11.08 ns|   4.67 ns| 宏定义加法
|                    - |        - |        - |
|array_push            |   7.98 ns|  12.68 ns| 数组: 直接赋值 
|array_struct_cast     |   8.23 ns|   2.82 ns| 数组: 转换成struct后再赋值
|snprintf_cost         | 198.99 ns| 197.64 ns| snprintf耗时
|                    - |        - |        - |
|int64_add             |   2.83 ns|   1.25 ns| int64 加法 
|int64_mul             |   2.56 ns|   1.42 ns| int64 乘法 
|int64_div             |   2.76 ns|   1.77 ns| int64 除法
|int64_remainder       |   1.61 ns|   0.93 ns| int64 取余
|int64_and             |   1.76 ns|   0.29 ns| int64 与
|double_add            |   3.08 ns|   1.88 ns| double 加法 
|double_mul            |   2.80 ns|   1.90 ns| double 乘法 
|double_div            |   5.14 ns|   4.28 ns| double 除法 
|rdtsc_cost            |   5.83 ns|   6.01 ns| 一次rdtsc耗时
|                    - |        - |        - |
|switch_case_5         |   3.26 ns|   2.27 ns| switch/case_5(分支越多越耗时) 
|if_else_5             |   2.64 ns|   1.25 ns| if/else_5(分支越多越耗时)
|                    - |        - |        - |
|ntoh16                |   1.64 ns|   0.47 ns| ntoh16
|ntoh32                |   2.12 ns|   0.93 ns| ntoh32
|ntoh64                |   2.85 ns|   1.47 ns| ntoh64



### 解析各操作耗时统计

| type  |              function |      O0  |      O3  |     desc |
|:-----:|-----------------------|---------:|---------:|----------|
|base   | std_vector_interat    |   7.33 ns|   0.47 ns| std::vector<int64_t> 遍历
|base   |  hashtable_find       |  22.36 ns|  13.00 ns|  HashTable 查找
|base   | std_unorderedmap_find | 120.87 ns|  23.78 ns| std::unorderedmap 查找
|base   |    std_map_find       | 257.85 ns|  83.72 ns| std::map<> 查找
|base   |    XQueue_insert      |  40.96 ns|  39.09 ns| XQueue 插入
|      -|                       |          |          |
|       |    memcpy_MD          |  17.33 ns|  16.40 ns| (热内存)直接拷贝768字节,96字段
|       |    assign_index       |   8.57 ns|   6.54 ns| (热内存)赋值13个字段
|       |    assign_stock       |  17.58 ns|  11.81 ns| (热内存)赋值55个字段
|       |    assign_option      |  17.62 ns|   8.10 ns| (热内存)赋值39个字段
|      -|                       |          |          |
|sh1    |    checksum_add       | 598.42 ns|  22.67 ns| 直接累加
|sh1    |    checksum_sse       | 159.24 ns|  29.68 ns| SSE
|sh1    |    checksum_sse_4loop | 118.15 ns|  23.79 ns| 4路SSE
|sh1    |    splite_fb          |  37.05 ns|  22.73 ns| FB切割数据包
|sh1    |    splite             |  23.06 ns|  24.58 ns| 直接切割数据包
|sh1    |    decode             | 169.35 ns|  43.73 ns| 直接解码(含memset)
|      -|                       |          |          |
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
|      -|                       |          |          |
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

测试环境：o2, CPU: Intel(R) Xeon(R) Gold 6256 CPU @ 3.60GHz, 睿频到4.3GHz. Solarflare Communications XtremeScale SFC9250. <br/>
IP头部长度为[20, 60]字节, TCP头部长度为[20, 60]字节, UDP头部长度固定8字节. MTU=1500.

结论：包的总长度尽量接近且不超过MTU时，效率最高. SF onload模式下，发小包的效率也挺高.

下图中的长度为UDP载荷的长度

|     type  |       function |     128 |     256 |     512 |    1024 |    1420 |    1500 |    2048 |    4096 |
|:---------:|----------------|--------:|--------:|--------:|--------:|--------:|--------:|--------:|--------:|
|    lo     | multicast send | 1.46 us | 1.47 us | 1.47 us | 1.49 us | 1.49 us | 1.49 us | 1.50 us | 1.55 us |
|    SF     | multicast send | 1.78 us | 1.79 us | 1.80 us | 1.82 us | 1.83 us | 2.83 us | 2.84 us | 3.79 us |
| SF_onload | multicast send |  525 ns |  728 ns |  464 ns |  657 ns | 1.10 us | 1.42 us | 1.69 us | 3.25 us |


### queue cost

测试环境: o2, CPU: Intel(R) Xeon(R) Gold 6256 CPU @ 3.60GHz, 睿频到4.3GHz. <br/>
一写一读的速度明显快很多，实现上也更容易.

| type | queue size | Throughput(in, W/s) | Throughput(out, W/s) | Latency(ns) | tips |
|:----:|------------|--------------------:|---------------------:|------------:|------|
| cycle|  1024*1024 |      2 ns,   5 WW/s |       2 ns,   5 WW/s |        2 ns |
| SPSC |  1024*1024 |      3 ns, 3.3 WW/s |       3 ns, 3.3 WW/s |      163 ns |
| SPSC1|  1024*1024 |     85 ns, 1176 W/s |      85 ns, 1176 W/s |      181 ns |
| MPSC |  1024*1024 |    593 ns,  168 W/s |     197 ns,  507 W/s |      ??? ns | 3P1C |
| MPMC |  1024*1024 |    880 ns,  113 W/s |     667 ns,  149 W/s |      ??? ns | 4P2C |
