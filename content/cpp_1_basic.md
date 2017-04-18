1 new/delete, malloc/free, new[]/delete[]
> malloc/free是函数，new/delete是c++运算符。

> new会调用对象的构造函数；delete会调用对象的析构函数；malloc/free只会分配/释放内存。

> delete[]会先依次调用每个元素的析构函数，然后调用operator delete来释放数组的内存。在使用时，一定要new/delete, new[]/delete[]配对使用。

> 对于内建对象，delete[]可以用来替代delete ('但不推荐这样用')


2 引用
> c++中的引用，不分配内存。使用引用对象和直接操作被引用对象，具有相同的效果。

> 引用在声明时就需要对其初始化。

> 将引用作为函数参数/返回值:
> > (1)能够减少对象的构造和析构次数，与传指针效果相同。若直接传对象，会调用拷贝构造函数。

> > (2)当不想在函数中改变值时，使用const引用.

> > (3)不能返回局部变量的引用，因为在函数执行完毕后，该变量将被销毁。

> lvalue & rvalue
> > (1)C语言中，通常认为在等号左边的为lvalue，在等号右边的为rvalue。如a=b+c中，a为lvalue，b+c为rvalue；另外一个方法，能取地址的、有名字的为lvalue，否则为rvalue。

> > (2)C++11中，将概念扩展。右值包括将亡值(eXpiring rvalue)和纯右值(pure rvalue)。
> > eXpiring rvalue: 将亡值是C++11新增的跟右值引用相关的表达式。如将要被移动的对象、右值引用T&&的函数返回值、std::move的返回值、转换为T&&的类型转换函数的返回值。
> > pure rvalue: 函数返回的非引用类型的值；表达式的值(b+c)；类型转换函数的返回值等。

> > 左值引用(lvalue reference)是具名变量的别名；右值引用(rvalue reference)是匿名变量的别名。

3 临时对象
```cpp
string foo();
void bar(string & s);
// 下面的表达式是非法的
bar(foo());
bar("hello world");
```
foo()函数返回一个临时对象，C++中临时对象是`const`类型的; 在返回临时对象的时候，会调用该对象的构造函数和析构函数.

"hello world"常量也是const类型的.


4 复杂的函数声明
```cpp
float (* (*p1)(int*))(int*);
// p1是一个函数指针，参数为int*类型。返回值是一个指针，指向一个函数，函数的参数为int*，返回值为float.

void* (* (*p2)(int) )[10];
// p2是一个函数指针，参数是int类型。返回值是一个指针，指向一个数组，该数组大小为10，元素为void*.

int (* (*p3)())[10]();
// p3是一个函数指针，参数为void。返回值是一个指针，
// 该指针指向一个数组，数组大小为10，数组中存放的是函数指针，类型为int(*p)()。
```

5 进程与线程的区别
> (1)进程是资源分配的最小单位，线程是CPU调度的最小单位。

> (2)线程共享的环境包括：进程代码段、进程的公有数据(利用这些共享的数据，线程很容易的实现相互之间的通讯)、进程打开的文件描述符、信号的处理函数、进程的当前目录和进程用户ID与进程组ID。

> (3)线程间不同的地方:
> > (1)线程ID不同

> > (2)线程栈不同：每个线程拥有自己的线程栈，是保证线程独立运行所必须的。线程函数可以调用函数，而被调用函数中又是可以层层嵌套的，所以线程必须拥有自己的函数堆栈，使得函数调用可以正常执行，不受其他线程的影响。

> > (3)errno不同

> > (4)线程的优先级: 由于线程需要像进程那样能够被调度，那么就必须要有可供调度使用的参数，这个参数就是线程的优先级。

> > (5)线程的信号屏蔽字: 每个线程所感兴趣的信号不同，所以线程的信号屏蔽字应该由线程自己管理。但所有的线程都共享同样的信号处理函数。

> > (6)**寄存器组的值**: 当从一个线程切换到另一个线程上时，必须将原有的线程的寄存器集合的状态保存，以便将来该线程在被重新切换到时能得以恢复。

6 进程间通信

各有什么优缺点、使用时需要注意一些什么问题。
> (1)用于消息传递：管道pipe，命名管道fifo，消息队列msgqueue

> (2)用于同步（锁）：互斥锁mutex, 读写锁rdlock, 文件锁flock, 条件变量pthread_cond_t, 信号量semphore

> (3)用于通知：signal

> (4)用于共享的：共享内存

> (5)socket, unix-socker, socket-pair

7 fork后父子进程区别
> (1)fork返回值不同，子进程返回0，父进程返回子进程的进程id

> (2)进程id不同

> (3)子进程的tms_utime, tms_stime, tms_cutime, tms_cstime均设置为0。

> > tms_utime记录的是进程执行用户代码的时间. 

> > tms_stime记录的是进程执行内核代码的时间. 

> > tms_cutime记录的是子进程执行用户代码的时间. 

> > tms_cstime记录的是子进程执行内核代码的时间.

> (4)父进程设置的文件锁，不会被子进程继承；

> (5)子进程未处理的定时器alarm被清除

> (6)子进程未处理的信号集设置为空

> (7)父子进程的信号屏蔽相同，环境变量相同


8 exec后父子进程的区别
> (1)exec使用全新的程序替代了当前进程的正文段(.code)数据段和堆栈段

> (2)环境变量既可以使用新传入的，也可以使用原来父进程的

> (3)保留了原来的进程id, 文件锁，进程信号屏蔽，未处理信号，资源限制、tms_utime等各种time

9 有关信号的几个概念
> (1)内核阻塞一个信号是指，不要忽略该信号，而是在该信号发生的时候记住它，在进程做好准备时，再将信号通知到进程。

> (2)信号未决，在信号产生时，内核通常会在进程表中设置一个标记。在信号产生到信号递送之间的时间间隔，称信号是未决的。

> (3)中断的系统调用，某些低速的系统调用(read等)在阻塞期间可能会捕捉到一个中断，则该系统调用就会被终止，不再继续执行。该系统调用会返回错误(errno=EINTR)。有些被中断的系统调用会自动重启(如ioctl, read, write)，而有些不会（如wait/waitpid）。

> (4)可重入函数，何时发生信号并调用信号处理函数是随机的。可能某个信号处理函数执行了一半，信号就发生了。若在信号处理过程中调用了一些函数，可能会引起错误，包括：(a)使用静态的数据结构;(b)使用了malloc/free;(c)标准i/o函数;(d)printf

10 动态库静态库的区别
> (1)编译后的大小、发布程序的方便程度、后续修改库文件、启动速度

> (2)-fPIC，在编译生成动态库时，需要使用该编译参数，生成位置无关的代码。否则会出现什么问题？

> (3)各使用什么命令来使用(static/shared)

> (4)代码和函数导出表

11 用户进程/内核进程的区别

12 机数srand(), rand()生成区间[100, 1 00 000]，如何生成[0, 0x7FFF FFFF]的随机数

13 STL中常用的容器及实现原理; string类的实现
> List, vector, map, set, multimap

14 虚函数的作用和实现原理
> 实现多态和接口

> 虚指针表

15 进程的退出方式有哪些? exit , `_exit`, return, abort 收到信号后退出等的不同

16 linux的内存管理机制

17 linux的任务调度机制

18 sigleton的实现区别
```cpp
class A {
public:
    // 只会调用一次new A;　但不会析构
    static A* getInstance1() {
        static A *p = new A;
        return p;
    }
    // 每次都会调用new A;
    static A* getInstance2() {
        static A *p = NULL;
        p = new A;
        return p;
    }
    // 之后构造一次，会析构
    static A* getInstance3() {
        static A a;
        return &a;
    }

    void print(const char* p) {
        std::cout << p;
    }
    ~A() {
        std::cout << "~A(),";
    }
private:
    A() {
        std::cout << "A(),";
    }
};
```

19 stack VS heap

> stack and heap are both stored in computer RAM.

> 在stack上分配空间比在heap上分配空间速度更快;（使用局部变量比new的速度更快）

> stack的使用总是伴随着特定的数据结构; heap的使用一般需要new/alloc，一般需要使用指针来访问；

> 在stack上的变量会自动释放; 在heap上的变量需要free/delete/delete[]; 内存泄漏发生在heap上；

> stack用来存放局部变量、返回地址、参数传递；

> stack的大小一般是有限制的（编译选项, ulimit -s, 4M）;

> 在heap上申请太大的空间时，可能会失败；

> stack overflow: 递归太深、局部变量占用空间太大；

> fragmentation: 当太多的alloctions/dealloctions时，会形成内存碎片；

> you should use stack if you know exactly how much data you need to allocte before compile-time and it's not too big.

> you should use heap if you don't know exactly how much data you will need at run-time or if you need to allocate a lot of data. 

20 why stack faster than heap ?

> In heap, all free momory is maintained by list, may not contiguous.

> In stack, all free memory is always contiguous(连续的), just a single pointer to the current top of the stack. Compilers usually store this pointer in a special, fast register for this purpose. 

> What's more, subsequent(后续) operations on a stack are usually concentrated  on a stack are usually concentrated within very nearby areas of memory, which at a very low level is good for optimization by the processor on-die caches.
