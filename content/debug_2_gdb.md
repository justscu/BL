### 1 生成core文件
通常**coredmp**包含了程序运行时的`内存，寄存器状态，堆栈指针，内存管理信息`等，可以理解为把程序工作的当前状态存储成一个文件。许多程序和操作系统出错时会自动生成一个core文件。另外，在编译时加上`-g`选项，加入调试信息方便调试。

(1)生成文件

临时生成core文件：`ulimit -c　unlimited`;
要让ulimit永久生效，`vim /etc/profile`，在文件末尾加入`ulimit -c unlimited`，然后使用命令：`source /etc/profile`

(2)配置core文件名称

修改`/etc/sysctl.conf`，可以配置生成core文件的路径和名字。
```sh
# %t，转储时刻；%e，可执行文件名；%p，被转储的进程id
kernel.core_pattern  = /tmp/core/%t-%e-%p.core # 在/tmp/core目录，生产转储文件
kernel.core_pattern  = %t-%e-%p.core           # 在可执行文件的当前目录，生产转储文件
kernel.core_uses_pid = 0   #不在core文件的末尾，自动加上pid
```
最后执行命令：`sysctl -p`

(3)共享内存是否生成core文件

修改文件`/proc/*/coredump_filter`


### 2 gdb基础

(1)启动
- gdb [options] [executable-file [core-file or process-id]]
- gdb [options] --args executable-file [inferior-arguments ...]

(2)设置参数

在gdb启动后，设置启动参数：使用`set args arg1 arg2 ...`
```sh
show args            #显示命令行参数
set variable opt = 1 #将opt设置为1
generate-core-file   #生产转储文件
attach/detatch pid
info proc            #显示进程信息
```

(3)执行
```sh
step into / next / return / continue / until / finish
stepi / nexti #按照汇编指令，一行一行的执行
continue 5(次数)  #遇到5次断点不停，第6次才停
finish #执行完当前函数后停止
until #执行完代码块后停止，常用于退出循环
until addr
```

(4)断点(break)
```sh
b func
b main.cpp:15
b +20          #当前往下20行
b *0x08116f43c #在地址 0x08116f43c 设断点
info break     #查看断点
delete / disable / enable 5(5是断点编号) # 对第5个断点进行操作
clear 函数名/行号/文件名:函数名/文件名:行号  #删除断点
```

(5)条件断点(condition break)
```sh
break  断点 if 条件  #break main.cpp:5 if argc==2
ignore 断点编号 次数  #忽略多少次
```

(6)监视点（watch）

当变量发生变化时，停止
```sh
watch  <表达式>  # 发生变化时停止
rwatch <表达式>  # 被访问时停止
awatch <表达式>  # 被访问、改变时停止
```

(7)栈(backtrace)
```sh
bt
bt N      #只显示开始的Ｎ个frame
bt -N     #只显示最后的Ｎ个frame
bt full   #显示局部变量
bt full 3 #显示3个frame & 局部变量
frame X   #查看第X个frame的信息
up / down
```

(8)打印

`print /格式 变量`，如`p /x $eax`，按16进制打印eax
```sh
x 16进制
d 10进制
u 无符号10进制
o 8进制
t 2进制（two）
f 浮点数
a 地址
c 字符
s 字符串
i 机器语言
```

(9)看内存
```sh
x /nfu <addr>
x /20s argv #打印argv开始的20个地址，以字符串显示
x /i main   #查看main的汇编指令
n：要显示的内存单元的个数
f：显示方式, 可取如下值
    x 按十六进制格式显示变量
    d 按十进制格式显示变量
    u 按十进制格式显示无符号整型
    o 按八进制格式显示变量
    t 按二进制格式显示变量
    a 按十六进制格式显示变量
    i 指令地址格式
    c 按字符格式显示变量
    f 按浮点数格式显示变量
u：表示一个地址单元的长度
    b表示单字节
    h表示双字节
    w表示四字节
    g表示八字节
```

(10)多线程
```sh
bt           # 显示的是主线程的栈
info thread  # 显示多个线程的内容
thread X     # 进入到第X个线程
再次bt        # 显示为第X个线程的栈
```

(11)宏

在make时，将`-g`修改为`-ggdb3`（注：这样会包含宏相关的信息，增大二进制文件）
```sh
(gdb) info macro NGX_OK  #查看有关NGX_OK的信息，哪些文件中定义该宏
(gdb) print NGX_OK       #打印值
```

对于没有定义宏的文件中，若不显示，可以
```sh
(gdb) list 包含宏定义头文件的文件，如　list kprintd.cpp:0
(gdb) info macro NGX_OK
```

(12)寄存器
```sh
(gdb) info reg   #显示各寄存器的值
(gdb) print $eax #寄存器名称前加$号，显示寄存器的内容
```



