## 服务器优化

服务器的性能优化问题，通常出现在3个地方：**CPU、网络IO、磁盘IO**。

* cpu: top/perftools/gprof
* mem: top/free/vmstat/valgrind
* net: netstat/ifstat/iftop/tcpdump
* io : iotop/iostat
* 其它: strace

### 1 可利用的工具
* (1)top，top是Linux下常用的性能分析工具，能够实时显示系统中各个进程的资源占用状况。
	* `top -p pid`，显示某个进程的信息。
	* `load average`, 任务队列的平均长度，可以查看1m,5m,15m的负载情况。load的值应该控制在`0.7*cpu核心数`以下，若`load值/cpu核心数>5`，说明CPU的负载比较高。每个cpu，会有一个等待队列，队列中存放了等待cpu执行的线程，这些线程等待分配cpu时间片。cpu的load-average，是一段时间内cpu正在处理的、加上等待cpu处理的(等待队列中)线程之和的统计信息。需要注意的是，即使某个线程获取了cpu的时间片，但可能并不使用cpu，如IO型任务。所以当cpu的负载很高，但load-average可能并不大；或者cpu的负载很小，但load-average很大。
	* `mem`是物理内存的信息，total/used/free分别表示总的/已使用的/未使用的物理内存总量，buffers表示用作内核缓存的物理内存总量。
	* `swap`是linux下虚拟内存分区，它的作用是在物理内存使用完后，将磁盘空间(也就是SWAP分区)虚拟成内存来使用。虽然swap分区能够作为"虚拟"的内存来使用，但它的速度比物理内存慢很多。total/used/free分别表示总的/已使用的/未使用的交换分区的内存总量，cached表示缓冲的交换区总量，物理内存中的内容被换出到swap，而后又被换入到内存，但使用过的swap内容尚未被覆盖，该数值即为这些内容已存在于内存中的交换区的大小。相应的内存再次被换出时可不必再对交换区写入。
	* `VIRT`进程使用的虚拟内存总量，VIRT=SWAP+RES；`SWAP`进程使用的虚拟内存中，被换出的大小；`RES`进程使用的、未被换出的物理内存大小，RES=CODE+DATA；`CODE`可执行代码占用的物理内存大小；`DATA`可执行代码以外的部分(数据段+栈)占用的物理内存大小；`SHR`共享内存大小。

* (2)free，显示系统内存的使用情况。

	```sh
				 total       used       free     shared    buffers     cached
	Mem:       4059240    3457236     602004     507916     308064    1531120
	-/+ buffers/cache:    1618052    2441188
	Swap:      2566140        320    2565820
	```

	* `Mem`行是从*OS角度*来看的内存使用情况。total/used/free分别表示总的/已使用的/未使用的内存数；shared表示被多个进程共享的内存数；buffers是指要写入到设备(如硬盘)的数量(A buffer is something that has yet to be "written" to disk)；cached是指从设备读出的还没被使用的数量(A cache is something that has been "read" from the disk and stored for later use)。OS为了提高系统性能，就从设备中多读一些，这也是cached比较大的原因。
	* `-/+ buffers/cache`行是从*应用程序*的角度来看的。used/free表示已使用的/剩余的内存数。其used=Mem.used-Mem.buffers-Mem.cached; free=Mem.free+Mem.buffers+Mem.cached。
	* `Swap`是交互分区的统计信息。

* (3)vmstat，性能比较全面，可以观察到系统的进程状态、内存使用、虚拟内存使用、磁盘的IO、中断、上下问切换、CPU使用等。`vmstat -t 1`，表示1s更新一次。

	```sh
	procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
	 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
	 2  0    320 596432 308664 1528612    0    0    23    23  314   41  6  2 91  1  0
	```

	* procs
		* `r`表示运行的和等待(CPU时间片)运行的进程数，这个值也可以判断是否需要增加CPU(长期大于cpu个数)。
		* `b`表示处于不可中断状态的进程数，常见的情况是由IO引起的。
	* swap
		* `si`表示由交换内存使用，由磁盘调入内存。
		* `so`表示由交换内存使用，由内存调入磁盘。
		* 内存够用的时候，这2个值都是0，如果这2个值长期大于0时，系统性能会受到影响。磁盘IO和CPU资源都会被消耗。
	* io
		* `bi`表示从块设备读入的数据总量(读磁盘)(单位:KB/s)。
		* `bo`表示写入到块设备的数据总理(写磁盘)(单位:KB/s)。
	* system
		* `in`表示每秒产生的中断次数。
		* `cs`表示每秒产生的上下文切换次数。
		* 这2个值越大，会看到由内核消耗的CPU时间会越多。
	* cpu
		* `us`用户进程消耗的CPU时间百分比，`us`的值比较高时，说明用户进程消耗的CPU时间多。
		* `sy`内核进程消耗的CPU时间百分比，`sy`的值高时，说明系统内核消耗的CPU资源多，这并不是良性的表现，我们应该检查原因。
		* `wa`IO等待消耗的CPU时间百分比。`wa`的值高时，说明IO等待比较严重，这可能是由于磁盘大量作随机访问造成，也有可能是磁盘的带宽出现瓶颈(块操作)。
		* `id`CPU处在空闲状态时间百分比。

* (4)strace，可以用来查看一个进程在执行过程中的系统调用和所接收的信号。`strace -p xxx -c`。

	```sh
	[root@localhost ~]# strace -p 28781 -c
	Process 28781 attached
	^CProcess 28781 detached
	% time     seconds  usecs/call     calls    errors syscall
	------ ----------- ----------- --------- --------- ----------------
	 94.48    0.015461         618        25           futex
	  5.30    0.000868          33        26           epoll_wait
	  0.21    0.000035          12         3         1 read
	------ ----------- ----------- --------- --------- ----------------
	100.00    0.016364                    54         1 total
	```

* (5)tcpdump，抓包分析工具，分析网络。

* (6)gprof，程序中每个函数的CPU使用时间。每个函数的调用次数，并提供简单调用关系图。

* (7)google-perftools，用来查看cpu和内存的统计信息，哪些函数耗费了多少cpu和内存。

* (8)ifstat，统计interface信息。

* (9)iftop，display bandwidth usage on an interface by host.如`iftop -i eth0 -F 10.15.144.105/32`，用来查看eth0网口，特定ip的数据。

* (10)iostat

### 2 优化方法

* (1)提高CPU性能

	* 并发，利用多线程/进程。进程/线程数不要大于cpu个数(请参考：[http://www.ibm.com/developerworks/cn/linux/l-threading.html](http://www.ibm.com/developerworks/cn/linux/l-threading.html))
	* 尽量减少上下文切换(context-switch)，进程/线程切换，是程序并发性能的真正杀手。过多的切换，会使CPU的资源消耗在上下文切换上，而不是业务处理上。(a)活跃线程数 > CPU核心数，活跃的线程越多，上下文切换越频繁(比较糟糕的设计，一个用户一个线程)。(b)对请求的处理，从一个线程转移到另外一个线程。比如用A线程接收用户的请求，之后将请求转给B线程处理，B处理完后，将结果返回给A，这样会增加2次上下文切换
    * 尽量减少锁的使用
	* 预分配资源，包括线程池、内存池、连接池等,除非内存比较紧张，否则就应该使用预分配资源；分配内存时，注意内存对齐
	* 尽量减少数据拷贝
	* 减少字符串操作，比如sprintf/snprintf，因为%d/%s等等都需要CPU资源去做词法分析，数量多的话，开销较大
	* 减少系统调用，例如time，主要消耗在用户态和内核态之间的切换
	* 一个好的架构，服务器的CPU总消耗总是平均的分布在各个cpu上，CPU的消耗在70%左右

* (2)提高网络IO

	* 使用epoll代替select
	* 减少阻塞函数的使用，使用非阻塞的模式来开发

* (3)提高磁盘IO
    * Linux可以利用空闲内存作文件系统访问的cache，因此系统内存越大存储系统的性能也越好
    * 利用顺序写，减少寻道次数
    * Cache策略，充分利用cpu和内存的资源来缓解磁盘读写压力
	* [推荐文章](http://elf8848.iteye.com/blog/1944219)
    * 查看磁盘常用命令: `df -h`查看磁盘分区及使用情况；`du -h path`递归查看各子目录下文件大小；`du -s path`只统计path的大小；
    * 磁盘的读写方式(顺序/随机)、缓存机制都对IO速度有很大影响。磁盘性能测试，推荐使用`hdparm`，如`hdparm -tT /dev/sda`。也可以用`dd`命令简单测试下磁盘性能：
```
# 测试/home/ll/目录写速度
time dd if=/dev/zero bs=1024 count=1000000 of=/home/ll/test
# 读
time dd if=/home/ll/test bs=1024 count=1000000 of=/dev/null
```

### 3 其它概念
	* 阻塞/非阻塞
	* 同步/异步
	
