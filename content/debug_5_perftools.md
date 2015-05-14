## google perftools

以[google-perftools-1.10](https://github.com/justscu/BL/tree/master/src/google-perftools-1.10)和libunwind-0.99.tar.gz为例。
perftools共包含TCMALLOC、HEAP PROFILER、HEAP CHECKER、CPU PROFILER等4个工具。

#### 编译libunwind
若是x86_64系统，需要先安装[libunwind](http://www.nongnu.org/libunwind/)。
libunwind项目的主要目的是提供一个可移植的高效的API接口，来跟踪程序的调用链(call-chain)。

假设安装在"/home/ll/u01/libunwind-0.99"下，
```sh
./configure --prefix=/home/ll/u01/libunwind-0.99
make
make install
```
可以看到在"/home/ll/u01/libunwind-0.99"下，多了"include/、lib/、share/"3个目录。

#### 编译google-perftools
假设要安装在"/home/ll/commlib/u01/"下
```sh
./configure CXXFLAGS="-I/home/ll/u01/libunwind-0.99/include -fpermissive -g" 
LDFLAGS="-L/home/ll/u01/libunwind-0.99/lib" 
LIBS="-lunwind" 
--prefix=/home/ll/u01/google-perftools-1.10 --host=x86_64

make
make install
```
配置/编译阶段错误处理
*	(1)configure时错误处理
```
checking whether we are cross compiling... configure: error: in `/home/ll/src/google-perftools-1.10':
configure: error: cannot run C compiled programs.
If you meant to cross compile, use `--host'.
See `config.log' for more details.
```
需要在配置时，添加`--host=x86_64`

*	(2)编译阶段错误
```
src/system-alloc.cc: In member function ‘virtual void* MmapSysAllocator::Alloc(size_t, size_t*, size_t)’:
src/system-alloc.cc:270:3: error: ‘failed_’ was not declared in this scope
   failed_ = true;
   ^
src/system-alloc.cc: In member function ‘virtual void* DevMemSysAllocator::Alloc(size_t, size_t*, size_t)’:
src/system-alloc.cc:339:3: error: ‘failed_’ was not declared in this scope
   failed_ = true;
make: *** [libtcmalloc_minimal_internal_la-system-alloc.lo] 错误 1
```
修改configure生成的文件`src/vim config.h`，添加
```cpp
#ifndef HAVE_MMAP
# define HAVE_MMAP 1
#endif
```

*	(3)编译阶段错误
```
src/base/linuxthreads.cc: In function ‘void ListerThread(ListerParams*)’:
src/base/linuxthreads.cc:312:22: error: invalid conversion from ‘void (*)(int, siginfo_t*, void*)’ to ‘void (*)(int, siginfo*, void*)’ [-fpermissive]
     sa.sa_sigaction_ = SignalHandler;
                      ^
make: *** [libtcmalloc_la-linuxthreads.lo] Error 1
```
这是函数的类型不一致导致的，添加编译选项`CXXFLAGS=-fpermissive`


#### 工具1 TCMALLOC
功能：提供更为高效的malloc/new。

gcc在编译时，会默认使用glibc提供的`malloc/new`。google提供的`tcmalloc`具有更高的效率，可用来优化c/c++程序。

在编译你的应用程序时，最好使用`-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free`选项，
在链接时，使用`-ltcmalloc`或`-ltcmalloc_minimal`。

#### 工具2 HEAP PROFILER
功能：分析heap内存使用情况。

链接时，使用`-ltcmalloc`。

*	分析整个程序的内存使用情况
*		使用命令`HEAPPROFILE=/tmp/heapprof <path/to/binary> [binary args]`启动程序，`HEAPPROFILE`环境变量指定生成prof文件的位置。
* 分析特定代码的内存使用情况
*		添加头文件`heap-profiler.h`
*		在开始分析/结束分析的地方，分别加上`HeapProfilerStart()`和`HeapProfilerStop()`，这样只会在特定代码开始和结束时生成prof文件。

分析prof文件：
`pprof <path/to/binary>　 /tmp/test.log.0001.heap`

#### 工具3 HEAP CHECKER
功能：用来检查内存泄露情况。

链接时，使用`-ltcmalloc`。
使用命令`HEAPCHECK=1 <path/to/binary> [binary args]`，`HEAPCHECK`是环境变量，其值有: normal (equivalent to "1"), strict, draconian。

#### 工具4 CPU PROFILER
功能：用来分析CPU使用情况。

链接时使用`-lprofiler`。
运行程序`CPUPROFILE=/tmp/prof.out <path/to/binary> [binary args]`。
分析prof文件`pprof <path/to/binary> /tmp/prof.out`。

注意：在fork后，cpu-profile会有问题。

#### 其它问题
若想同时使用CPU profiler, heap profiler, heap leak-checker，可以使用`gcc -o myapp ... -lprofiler -ltcmalloc`进行编译。

问题(1)在x86_64系统上使用libunwind进行stack跟踪时，可能会使你的程序阻塞住(hang)或崩溃(crash)。该问题不会影响tcmalloc，但会影响剩余3个功能(cpu-profiler, heap-checker, heap-profiler)。glibc2.4(x86_64上)，`dl_iterate_phdr()`函数可能会导致cpu-profiler, heap-profiler, heap-checker死锁。程序中调用`dlopen`、`getgrgid`可能会出现该问题。

问题(2)在x86_64系统上使用`CPUPROFILE`环境变量启动cpu-profiling功能时，可能会导致程序崩溃。在libc中，有函数backtrace()，backtrace通常没有什么问题，但当要处理信号时，就可能会有问题。因为cpu-profile每次使用信号来注册profiling事件，所有每次backtrace需要跨越信号栈(crosses a signal frame)。
根据经验，在`pthread_mutex_lock`过程中，若产生信号，就会有问题。可以使用`ProfilerStart()/ProfilerStop()`来减少这种问题的发生几率，而不要使用`CPUPROFILE`环境变量。

