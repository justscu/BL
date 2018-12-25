## Valgrind

Valgrind可以用来检查数组是否越界、是否有内存泄露等问题。Valgrind工具包包含多个工具，如Memcheck, Cachegrind,Helgrind, Callgrind，Massif等。使用方法：**valgrind [options] prog-and-args**，在检测内存泄漏的时候，一定要让程序正常退出。
```sh
void prog_exit(int /*signo */ ) {
    print("prog_exit");
}
signal(SIGQUIT, prog_exit);
```
new/delete、malloc/free一定要配对。使用完内存，要记得及时释放，并将指针置NULL。

编译选项可能会影响valgrind的报告：
- `-fno-inline`使得函数调用链很清晰；
- `-O2`或以上的优化选项，会使memcheck误报一些未初始化内存错误；最好关闭优化选项；

### 1 Memcheck

命令`valgrind --tool=memcheck --leak-check=full --show-reachable=yes --log-file=./p.log ./proxy ../etc/cfg.xml`。

主要检查下面的程序错误：
- 使用未初始化的内存 (Use of uninitialised memory)
- 使用已经释放了的内存 (Reading/writing memory after it has been free’d)
- 使用超过 malloc分配的内存空间(Reading/writing off the end of malloc’d blocks)
- 对堆栈的非法访问 (Reading/writing inappropriate areas on the stack)
- 申请的空间是否有释放 (Memory leaks – where pointers to malloc’d blocks are lost forever)
- malloc/free/new/delete申请和释放内存的匹配(Mismatched use of malloc/new/new [] vs free/delete/delete [])
- src和dst的重叠(Overlapping src and dst pointers in memcpy() and related functions)

### 2 Callgrind

命令`valgrind --tool=callgrind --log-file=call ./proxy -f ../etc/cfg.xml`，生成一个callgrind.out.<pid>文件<br/>
`--separate-threads=yes`会为每个线程单独生成一个文件.

Callgrind收集程序运行时的一些数据，函数调用关系等信息，还可以有选择地进行cache模拟。在运行结束时，它会把分析数据写入一个文件。`callgrind_annotate`可以把这个文件的内容转化成可读的形式。

(1) 查看调用关系: `callgrind_annotate --inclusive=yes --tree=calling ./callgrind.out.26939 > 3.log` <br/>
(2) 查看被调用　: `callgrind_annotate --inclusive=yes --tree=caller  ./callgrind.out.26939 > 2.log`
```sh
8,350,335  >   /home/ll/project/message.cpp:DFIX::Message::tostr(std::string&, int)  (312x) [/home/ll/bin/proxy]
8,350,335 表示执行的指令数， (312x)表示调用的次数。
```
(3) 对源代码进行注解: `callgrind_annotate callgrind.out.2089  /home/ll/code/src/shl1.cpp[编译时的全路径]` <br/>
    Ir，指令数，Ir越大，说明运行时使用的CPU越多 <br/>
    [kcachegrind](https://kcachegrind.github.io/html/Home.html)

### 3 Cachegrind

命令`valgrind --tool=cachegrind --log-file=cache.log ./proxy ../etc/cfg.xml`，生成一个cachegrind.out.<pid>文件，但该文件需要转换。<br/>
`--branch-sim=yes`，分支预测信息(默认不开启).

它模拟 CPU中的一级缓存L1,D1和L2二级缓存，能够精确地指出程序中 cache的丢失和命中。如果需要，它还能够为我们提供cache丢失次数，内存引用次数，以及每行代码，每个函数，每个模块，整个程序产生的指令数。这对优化程序有很大的帮助。

`cg_annotate  --auto=yes   cachegrind.out.<pid> > out.log`，生成一个可以看懂的文件 <br/>

Cachegrind模拟第一级缓存(I1)和最后一级缓存(IL),其中最后一级缓存对程序的影响最大。

指令数                            | 未命中数                   | 未命中数                   |
----------------------------------|----------------------------|----------------------------|
Ir, 指令缓存（等效于执行的指令数）| I1mr, I1缓存读取指令未命中 | ILmr, IL缓存读取指令未命中 |
Dr, 读缓存（等效于内存读取次数）  | D1mr, D1缓存读取数据未命中 | DLmr, DL缓存读取数据未命中 |
Dw, 写缓存（等效于内存写的次数）  | D1mw, D1缓存写入数据未命中 | DLmw, DL缓存写入数据未命中 |
Bc, 条件分支执行                  | Bcm, 条件分支mispredicted  |
Bi, 间接分支执行                  | Bim, 间接分支错误


### 4 Helgrind

命令`valgrind --tool=helgrind --read-var-info=yes --log-file=helgrind.log ./proxy ../etc/cfg.xml`

它主要用来检查多线程程序中出现的竞争问题。
- unlocking an invalid mutex
- unlocking a not-locked mutex
- unlocking a mutex held by a different thread
- destroying an invalid or a locked mutex
- recursively(递归) locking a non-recursive mutex
- deallocation of memory that contains a locked mutex
- passing mutex arguments to functions expecting reader-writer lock arguments, and vice versa(将mutex作为参数，传给需要RWLock的函数)
- when a POSIX pthread function fails with an error code that must be handled
- when a thread exits whilst still holding locked locks
- calling pthread_cond_wait with a not-locked mutex, an invalid mutex, or one locked by a different thread
- inconsistent bindings between condition variables and their associated mutexes
- invalid or duplicate initialisation of a pthread barrier
- initialisation of a pthread barrier on which threads are still waiting
- destruction of a pthread barrier object which was never initialised, or on which threads are still waiting
- waiting on an uninitialised pthread barrier
- for all of the pthreads functions that Helgrind intercepts(拦截), an error is reported, along with a stack trace, if the system threading library routine returns an error code, even if Helgrind itself detected no error
- Helgrind 寻找内存中被多个线程访问，而又没有一贯加锁的区域，这些区域往往是线程之间失去同步的地方，而且会导致难以发掘的错误。Helgrind实现了名为"Eraser"的竞争检测算法，并做了进一步改进，减少了报告错误的次数。
- Helgrind works best when your application uses only the POSIX pthreads API.

可以检测以下错误（Problems like these often result in unreproducible, timing-dependent crashes, deadlocks and other misbehaviour, and can be difficult to find by other means.）：
- 错误使用POSIX threads API
- 锁的顺序而导致的死锁
- 数据竞争(读取内存中的数据，但缺少锁或同步)

### 5 Massif
堆栈分析器，它能测量程序在堆栈中使用了多少内存，告诉我们堆块，堆管理块和栈的大小。Massif能帮助我们减少内存的使用，在带有虚拟内存的现代系统中，它还能够加速我们程序的运行，减少程序停留在交换区中的几率。
