## stack

### 1 程序地址空间

![debug](https://github.com/justscu/BL/blob/master/pics/debug_4_1.png) 

- 0 - 3GB为用户空间；3GB - 4GB为内核空间。
- 初始化的数据段，包括 已经初始化的static 类型变量、已经初始化的全局变量、全局常量等。
- 未初始化数据段，包括 未初始化的static类型变量、未初始化的全局变量。（exec会将其初始化为0）
- new/malloc，从堆中分配空间。
- 栈：函数参数的传递、返回；局部变量等，都在栈中分配空间。

### 2 进程堆/线程栈
对进程来讲，虚拟地址空间的大小是固定的，进程只有一个堆，大小通常不是问题。

对线程来讲，同样大小的虚拟地址空间，必须被所有的线程共享。
如果应用程序的线程过多，会导致线程栈的累计大小超过可用的虚拟地址空间，这时需要减小默认栈的大小。
如果线程调用分配了大量的局部变量或递归调用很深，需要的栈空间可能比默认栈空间大。

给`sysconf()`传入_SC_THREAD_ATTR_STACKADDR、_SC_THREAD_ATTR_STACKSIZE来检查系统是否支持线程栈属性。

调整线程栈的函数
- int pthread_attr_getstack(const pthread_attr_t* attr, size_t* stacksize); #获取线程栈起始地址和大小
- int pthread_attr_setstack(pthread_attr_t* attr, size_t stacksize);        #设置线程栈起始地址和大小

线程属性guardsize，控制着线程栈末尾之后的PAGESIZE(默认值)个字节，用以避免栈溢出的扩展内存的大小。
- int pthread_attr_getguardsize(const pthread_attr_t* attr, size_t* guardsize);
- int pthread_attr_setguardsize(pthread_attr_t* attr, size_t stacksize);

### 3 函数调用过程 
函数的调用，其实就是一个保护调用现场、入栈、出栈的过程。
栈用于保存局部变量、传递函数参数、返回地址、保存函数地址和返回值；有时栈也用来临时保存寄存器中的内容。
对返回地址的解释：在调用f2时，栈中的返回地址即为a++指令的地址。
```sh
void f1() {
    int a;
    f2(a);
    a++; //返回地址
}
void f2(int) {
 ... 
}
```

### 4 Segmentation Violation（SIGSEGV） 
当出现栈错误时，内核会向应用程序发送**SIGSEGV**信号，并（默认）终止该进程。
若安装了SIGSEGV信号处理函数，发生该信号时，就会去调用信号处理函数，但可能运行信号处理函数所需的栈无法保障。为了捕获栈溢出，需使用备用栈，即sigaltstack(2)。
```sh
static void sigseg_handler(int sig) {
     int x = 0;
     write (STDERR_FILENO, "Caught signal %d (%s)\n", sig, strsignal(sig));
     write (STDERR_FILENO, "Top of handler stack near %10p\n", (void *)&x);
     fflush(NULL);
     void* buffer[1024+1];
     memset(buffer, 0, sizeof(buffer));
     int nptr = backtrace(buffer, 1024);
     backtrace_symbols_fd(buffer, nptr, STDERR_FILENO); // dump backtrace to stderr.
     _exit(EXIT_FAILURE);
}

string f_s(string & a) {
     return f_s(a) + "kfm ";
}

int main() {
     //backtrace_symbols(buffer, nptr);
     stack_t ss;
     ss.ss_sp = malloc(SIGSTKSZ);
     if (ss.ss_sp == NULL)
     {
         return 0;
     }
     ss.ss_size = SIGSTKSZ;
     ss.ss_flags = 0;
     if (sigaltstack(&ss, NULL) == -1)
     {
         return 0;
     }
     struct sigaction sa;
     sa.sa_handler = sigseg_handler;
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_ONSTACK; // 信号处理函数使用可变栈
     if(sigaction(SIGSEGV, &sa, NULL) == -1)
     {
         perror("sigaction");
         exit(EXIT_FAILURE);
     }
     string a;
     f_s(a);
     return 0;
}
```

### 5 栈调试
#### 5.1 基本命令

gdb的backtrace就是通过搜索栈中的信息来实现的。常用的gdb命令：
```sh
bt +n
bt -n 
bt n 
frame num 
up 
down 
info frame num
```

```sh
#查看栈大小
ulimit -s

#查看进程内存映像(非转储文件)
(gdb) info proc mappings 
$sp的值，应该在栈空间中，否则可能栈溢出

#查看进程内存映像(转储文件)
(gdb) info files
(gdb) backtrace
(gdb) frame full 2

#当栈被破坏时，就看看寄存器的信息吧
(gdb) info reg 
      esp            0xbf0dfff0   0xbf0dfff0
      eip            0x804941e    0x804941e <f_s(std::string&)+24>
(gdb) x /g 0xbf0dfff0  # 栈顶内容（取8字节）或 x /g $esp
      0xbf0dfff0:    Cannot access memory at address 0xbf0dfff0 
无法获取栈顶内容，说明栈顶已被破坏
(gdb) x /i 0x804941e   # eip，本条指令 或 x /i $eip
      => 0x804941e <f_s(std::string&)+24>:    mov    %edx,0x4(%esp)
```

#### 5.2 backtrace失效时
对局部变量的引用、返回局部变量的指针，数组越界写都可能导致栈被破坏。
bt命令显示的信息，是根据栈中的信息反推得到的。若栈被破坏了，bt就会显示一些错误的函数地址（只有地址信息，找不到对应的函数）。
只有当栈没有被破坏时，bt的结果才是值得信赖的。
```sh
(gdb) bt
    #0  0xb74b8827 in ?? ()
    #1  0xb7635000 in ?? ()
    #2  0xb758637b in ?? ()
```
0xb74b8827表示指令的地址，list *0xb74b8827，无法显示代码，说明可能是程序跳转出了问题（也有可能是链接文件，不包含调试信息）。可以沿着frame，往下查。

```sh
(gdb) bt
   #0  0x0804941e in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
   #1  0x0804942a in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
   #2  0x0804942a in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
   #3  0x0804942a in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
   #4  0x0804942a in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
   #5  0x0804942a in f_s (a=...) at /home/ll/project/test/pcretest/main.cpp:33
```
可以看出是递归调用。

#### 5.3 跳转对程序的影响
- 代码段中跳转指令。由于代码段只读，这个不会有问题的。尝试修改代码段的内容，会抛出现段错误。
- 在运行完被调用函数后，执行ret指令。跳转的地址，在栈中，但该值可能在栈被破坏的时候被修改。

### 6 利用栈信息绘制函数调用图(tracestack)

程序在运行过程中，发生函数调用时，会有入栈、出栈的动作。记录函数入栈、出栈等信息，便可以查看在运行过程中，函数的被调用情况和被调用次数，还可以加上时间信息，以便记录函数的耗时。

#### 6.1 记录函数的入栈、出栈函数
- void __cyg_profile_func_enter( void *func_address, void *call_site )__attribute__ ((no_instrument_function));
- void __cyg_profile_func_exit ( void *func_address, void *call_site )__attribute__ ((no_instrument_function));

GCC提供功能，用来记录函数的入栈和出栈。
当函数被调用时，`__cyg_profile_func_enter`先被调用，func_address为被调用函数的入口地址；
当退出函数时， `__cyg_profile_func_exit`先被调用，func_address为被调用函数的入口地址。这样，便可以记录函数在什么时候入栈出栈了。
要想使用该功能，需要将编译选项`CFLAGS="-g -finstrument-functions"`加入到Makefile中。
```sh
void __cyg_profile_func_enter( void *this, void *callsite )
{
    fprintf(fp, "E%p\n", (int *)this); // 打印入栈信息
}
void __cyg_profile_func_exit( void *this, void *callsite )
{
    fprintf(fp, "X%p\n", (int *)this); // 打印出栈信息
}
```

#### 6.2 main函数的构造和析构
- void main_constructor( void ) __attribute__ ((no_instrument_function, constructor)); 在main函数被构造的时候调用
- void main_destructor ( void ) __attribute__ ((no_instrument_function, destructor)); 在main函数被析构的时候调用

```sh
static FILE *fp;

void main_constructor( void ) {
    fp = fopen( "trace.txt", "w" );
    if (fp == NULL)
        exit(-1);
}

void main_deconstructor( void ) {
    fclose( fp );
}
```
可以将上面的代码拷贝到单独的文件中，并加入到自己的工程中，make CFLAGS="-g -finstrument-functions"。之后运行自己的程序，便可以得到一个trace.txt文件，该文件中，记录了函数的入栈、出栈情况。

#### 6.3 将函数地址翻译成函数名
trace.txt中，都是以函数地址形式（16进制）记录下来的，最好是将函数地址翻译成对应的函数名。addr2line可以做到这点，一个典型的命令形式为：`addr2line -e ./nginx -f -s 0x804a587`，这样就可以得到对应的函数地址和该函数所在的文件名。

#### 6.4 函数调用关系及次数
进一步分析函数的入栈出栈信息，可以得到在程序运行过程中，函数的调用关系和调用次数。
利用我们自己编写的程序，可以将这种调用描述为一个dot文件。
linux平台下的xdot程序，可以将dot文件显示成图的形式。

#### 6.5 tracestack
[tracestack](https://github.com/justscu/tracestack)，通过记录函数的调用栈信息，生成函数调用图标，以及函数的每一步调用过程。
