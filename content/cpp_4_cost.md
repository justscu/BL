各操作耗时统计

3.4GHZ CPU.

func                  |    O0    |    O3    |    含义      |
----------------------|---------:|---------:|--------------|
add_func              |   3.30 ns|   0.38 ns| 自定义加法 
add_func_withmutex    |  21.34 ns|  13.72 ns| 自定义加法(with mutex) 
add_templates         |  15.10 ns|   0.28 ns| 递归加法 
add_va_args           |  11.08 ns|   2.43 ns| 宏定义加法 
array_push            |   7.98 ns|   3.54 ns| 数组: 直接赋值 
array_struct_cast     |   8.23 ns|   3.95 ns| 数组: 转换成struct后再赋值 
memcpy_random         | 121.53 ns| 115.71 ns| 随机memcpy_4 bytes 
memcpy_random         | 139.88 ns| 114.19 ns| 随机memcpy_8 bytes 
memcpy_random         | 135.39 ns| 117.55 ns| 随机memcpy_16 bytes 
memcpy_random         | 450.64 ns| 428.27 ns| 随机memcpy_1K 
memcpy_random         | 671.63 ns| 648.86 ns| 随机memcpy_2K 
memcpy_random         |   1.08 us|   1.05 us| 随机memcpy_4K 
memcpy_random         |   1.84 us|   1.80 us| 随机memcpy_8K 
memset_random         | 127.96 ns| 107.68 ns| 随机memset_4 bytes 
memset_random         | 128.91 ns| 110.89 ns| 随机memset_8 bytes 
memset_random         | 130.99 ns| 113.15 ns| 随机memset_16 bytes 
memset_random         | 321.65 ns| 302.09 ns| 随机memset_1K 
memset_random         | 469.11 ns| 441.43 ns| 随机memset_2K 
memset_random         | 727.39 ns| 705.96 ns| 随机memset_4K 
memset_random         |   1.25 us|   1.22 us| 随机memset_8K 
memset_random         |   2.29 us|   2.25 us| 随机memset_16K 
snprintf_cost         | 177.21 ns| 165.27 ns| snprintf耗时 
int64_add             |   2.13 ns|   0.25 ns| int64  加法 
double_add            |   2.38 ns|   1.07 ns| double 加法 
double_mul            |   3.14 ns|   2.14 ns| double 乘法 
double_div            |   5.67 ns|   4.73 ns| double 除法 
switch_case_5         |   3.60 ns|   2.45 ns| switch/case_5(分支越多越耗时) 
if_else_5             |   2.91 ns|   1.63 ns| if/else_5(分支越多越耗时)

