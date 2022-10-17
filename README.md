#### 语言及工程
- [git使用方法](https://github.com/justscu/BL/blob/master/content/git_1.md)
- [c++工程常用工具](https://github.com/justscu/BL/blob/master/content/projectTool_1.md): gcc, cmake, configure
- [gcc头文件及lib文件](https://github.com/justscu/BL/blob/master/content/projectTool_3.md)
- [动态库中使用全局变量/静态变量和函数](https://github.com/justscu/BL/blob/master/content/projectTool_4.md)
- [环境变量](https://github.com/justscu/BL/blob/master/content/projectTool_3.md)
- [makefile](https://github.com/justscu/BL/blob/master/content/projectTool_2_makefile.md)
- [shell脚本写法](https://github.com/justscu/BL/blob/master/content/shell_1.md)
- [c++常见面试问题](https://github.com/justscu/BL/blob/master/content/cpp_1_basic.md)
- [c++ Primer](https://github.com/justscu/BL/blob/master/content/cpp_3_primer_5e.md)
- [c++ Benchmark](https://github.com/justscu/BL/blob/master/content/cpp_4_benchmark.md)
- [c++ MemoryOrder](https://github.com/justscu/BL/blob/master/content/cpp_5_memory_order.md)
- [SIMD](https://github.com/justscu/BL/blob/master/content/cpp_6_simd.md)
- [三方库](https://github.com/justscu/BL/blob/master/content/3rdlib_1.md)


#### 工具
- [htop/awk/vim](https://github.com/justscu/BL/blob/master/content/shell_3.md)
- [crontab](https://github.com/justscu/BL/blob/master/content/shell_2.md)


#### 调试
- [调试基础](https://github.com/justscu/BL/blob/master/content/debug_1_basic.md)
- [gdb](https://github.com/justscu/BL/blob/master/content/debug_2_gdb.md)
- [vargrind](https://github.com/justscu/BL/blob/master/content/debug_3_valgrind.md): 用来检查内存泄露(memcheck)、生成函数调用关系(callgrind)、cache命中率(cachegrind)、检查多线程程序竞争问题(helgrind)等
- [google-perftools](https://github.com/justscu/BL/blob/master/content/debug_5_perftools.md): 提供更为高效的内存分配函数tcmalloc，对CPU使用情况进行检查cpu-profile，对内存使用情况进行检查heap-profile，内存泄露进行检查heap-checker
- [stack](https://github.com/justscu/BL/blob/master/content/debug_4_stack.md): stack是向下生长的，介绍SIGSEGV信号及如何用gdb调试栈


#### 网络
- [tcp状态分析](https://github.com/justscu/BL/blob/master/content/network_1_tcpstate.md)
- [ftp](https://github.com/justscu/BL/blob/master/content/network_2_ftp.md)
- [google protocol buffer](https://github.com/justscu/BL/blob/master/content/network_3_protocolbuffer.md)
- [网络分析工具](https://github.com/justscu/BL/blob/master/content/network_4_ansysistool.md): tcpdump, netstat, lsof等
- [epoll](https://github.com/justscu/BL/blob/master/content/network_5_epoll.md)
- [网卡](https://github.com/justscu/BL/blob/master/content/network_6_interface_card.md)
- [组播](https://github.com/justscu/BL/blob/master/content/network_7_udp.md)

#### 数据结构 
- [二叉树](https://github.com/justscu/BL/blob/master/content/struct_1_二叉树.md)
- [一致性hash](https://github.com/justscu/BL/blob/master/content/struct_2_conhash.md)
- [内存池](https://github.com/justscu/BL/blob/master/content/struct_3_内存池.md)
- [跳表](https://github.com/justscu/BL/blob/master/content/struct_4_SkipList.md)
- [环形缓冲区](https://github.com/justscu/BL/blob/master/content/struct_5_环形缓冲区.md)


#### CSAPP
- [数据的表示](https://github.com/justscu/BL/blob/master/content/CSAPP-2-数据的表示.md): int/float/double如何在内存中表示
- [汇编基础](https://github.com/justscu/BL/blob/master/content/CSAPP-3-汇编.md): x86-64汇编、寄存器
- [优化程序性能](https://github.com/justscu/BL/blob/master/content/CSAPP-5-优化程序性能.md): 指令级并行
- [存储](https://github.com/justscu/BL/blob/master/content/CSAPP-6-存储.md): 各存储器延时、False-Sharing
- [链接](https://github.com/justscu/BL/blob/master/content/CSAPP-7-链接.md): 符号解析、重定位
- [异常控制流](https://github.com/justscu/BL/blob/master/content/CSAPP-8-异常控制流.md): 中断
- [虚拟内存](https://github.com/justscu/BL/blob/master/content/CSAPP-9-虚拟内存.md): 虚拟内存



#### 系统设计与优化
- [服务器设计](https://github.com/justscu/BL/blob/master/content/sdaa_1_服务器设计.md): 从系统性能、可用性、伸缩性、扩展性、安全性等方面考虑
- [高性能服务器设计](https://github.com/justscu/BL/blob/master/content/sdaa_2_高性能服务器设计.md): 预分配、数据拷贝、锁、线程切换
- [服务器优化](https://github.com/justscu/BL/blob/master/content/sdaa_3_服务器优化.md): cpu/mem/io等方面
- [服务发现](https://github.com/justscu/BL/blob/master/content/sdaa_4_服务发现.md): 分析smartStack服务注册/服务发现框架；Nerve/Snapse/HaProxy
- [负载均衡](https://github.com/justscu/BL/blob/master/content/sdaa_5_负载均衡.md): HaProxy/LVS/KeepAlived
- [同步与锁](https://github.com/justscu/BL/blob/master/content/sdaa_6_同步与锁.md)


#### 数据库
- [Linux平台操作MS SqlServer](https://github.com/justscu/BL/blob/master/content/database_1_sqlserver.md): linux平台下，使用unix-odbc操作sql server数据库
- [mongo db](https://github.com/justscu/BL/blob/master/content/database_2_mongodb.md)


#### golang
- [golang基础知识](https://github.com/justscu/BL/blob/master/content/golang_1_basic.md)
- [如何安装go开发环境](https://github.com/justscu/BL/blob/master/content/golang_2_install.md)
- [gopath及相关解释](https://github.com/justscu/BL/blob/master/content/golang_2_install.md)
- [go tool pprof工具](https://github.com/justscu/BL/blob/master/content/golang_4_pprof.md)
- [go方法总结](https://github.com/justscu/BL/blob/master/content/golang_3.md)
- [go知识点测试](https://github.com/justscu/BL/blob/master/content/golang_6_test.md)
- [go panic: out of memory](https://github.com/justscu/BL/blob/master/content/golang_5_outofmem.md)
