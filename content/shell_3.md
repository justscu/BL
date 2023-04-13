
|             |
|-------------|
| [htop](#htop) |
| [SAR](#SAR)   |
| [AWK](#awk)   |
| [vim](#vim)   |



## htop 

[htop源码路径](https://github.com/htop-dev/htop)

##### top命令查看CPU使用情况
top后，查看cpu这一行，各字段具体含义
缩写|含义
---|---
us | user space占用CPU百分比
sy | (system)Kernel space占用CPU时间百分比
ni | niceness，nice进程占用CPU时间百分比
id | idle，CPU空闲时间百分比，该值越小CPU越忙
wa | wait, CPU等待外部IO占用CPU时间百分比。这段时间内CPU不能干其它事，值高的话说明外部设备有问题
hi | hardware interrupt, 响应硬件中断占用CPU时间百分比
si | software interrupt, 响应软件中断占用CPU时间百分比
st | stole time, （只对虚拟机有效）被同一台物理机上的其它虚拟机偷走的时间

##### htop 快捷键
key | short key | description 
---|------|-------------
F1 | h | 帮助
F2 | S | 设置
F3 | / | 搜索进程
F4 | \ | 对进程进行过滤
F5 | t | 显示树形结构
F6 |<,>| 选择排序方式
F7 | [ | nice-，提高进程优先级
F8 | ] | nice+，降低进程优先级
F9 | k | 杀死进程
F10| q | 退出


##### htop 交互式命令

key | description
:--:|------------
q         | 退出
space     | 对进程进行标记，可以同时标记多个
s         | 选择某一进程，按下s后，使用strace追踪系统调用
l         | 选择某一进程，按下l后，显示lsof
shift + m | 按内存大小排序
shift + p | 按cpu使用率排序
shift + t | 按进程运行时间排序
shift + h | 收缩线程
K         | 显示/隐藏内核线程


##### Setup 详情
按F2进入Setup
items | description
:----:|------------
Meters|设定htop顶端的信息（CPU/MEM/SWAP/Task/Load Avg等）。Available meters, htop提供的显示功能. <br/> Task counter: 任务信息. <br/> Load average: 负载信息. <br/> Clock: 时钟
Display options|显示的内容<br/> Hide kernel threads: 隐藏内核线程. <br/> Hide userland process threads: 隐藏用户线程.
Colors |显示的颜色
Columns|Active Columns:激活的显示条目. <br/> Available Columns: 提供的条目.


##### cpu

meaning | cmd
----|--------
总物理CPU核数 | 物理CPU个数 * 每个物理CPU的核数
总逻辑CPU核数 | 物理CPU个数 * 每个物理CPU的核数 * 超线程数
查看CPU基本信息 | cat /proc/cpuinfo， lscpu
物理CPU个数 | "physical id"
物理核(心)数| "cpu cores"
逻辑CPU个数 | "processor"


## SAR(System Activity Reporter)

sar工具对linux系统当前运行状态采样分析。安装命令`yum install sysstat`。<br/>
sar配置文件"/etc/sysconfig/sysstat"，日志文件"/var/log/sa/"。<br/>

在"/etc/sysconfig/sysstat"文件中增加`ENABLE="false"`，可以禁止自动采集。`ENABLE="true"`，自动采集。`systemctl restart sysstat`，重起后修改生效。

```
常用监控命令
(1) sar -b        // IO传送速率
(2) sar -B        // 页交换速率
(3) sar -c        // 进程创建的速率
(4) sar -d        // 块设备的活跃信息
(5) sar -n DEV    // 网路设备的状态信息
(6) sar -n SOCK   // SOCK的使用情况
(7) sar -n ALL    // 所有的网络状态信息
(8) sar -P ALL    // 每颗CPU的使用状态信息和IOWAIT统计状态 
(9) sar -q        // 队列的长度（等待运行的进程数）和负载的状态
(10) sar -r       // 内存和swap空间使用情况
(11) sar -R       // 内存的统计信息（内存页的分配和释放、系统每秒作为BUFFER使用内存页、每秒被cache到的内存页）
(12) sar -u       // CPU的使用情况和IOWAIT信息（同默认监控）
(13) sar -v       // inode, file and other kernel tablesd的状态信息
(14) sar -w       // 每秒上下文交换的数目
(15) sar -W       // SWAP交换的统计信息(监控状态同iostat 的si so)
(16) sar -x 2906  // 显示指定进程(2906)的统计信息，信息包括：进程造成的错误、用户级和系统级用户CPU的占用情况、运行在哪颗CPU上
(17) sar -y       // TTY设备的活动状态
(18) 将输出到文件(-o)和读取记录信息(-f)
```

每10秒统计一次所有CPU情况：`sar -u 10`； 每5秒统计一次core 2的情况: `sar -P 2 5`

```
CPU    ：all表示统计信息为所有 CPU的平均值。
%user  ：显示在用户级别(application)运行使用 CPU总时间的百分比。
%nice  ：显示在用户级别，用于nice操作，所占用 CPU总时间的百分比。
%system：在核心级别(kernel)运行所使用 CPU总时间的百分比。
%iowait：显示用于等待I/O操作占用 CPU总时间的百分比。
%steal ：管理程序(hypervisor)为另一个虚拟进程提供服务而等待虚拟CPU 的百分比。
%idle  ：显示 CPU空闲时间占用 CPU总时间的百分比。

```

```
#IFACE    网卡名称
#rxerr/s  每秒钟接收到的损坏的数据包
#txerr/s  每秒钟发送的数据包错误数
#coll/s   当发送数据包时候，每秒钟发生的冲撞（collisions）数，这个是在半双工模式下才有
#rxdrop/s 当由于缓冲区满的时候，网卡设备接收端每秒钟丢掉的网络包的数目
#txdrop/s 当由于缓冲区满的时候，网络设备发送端每秒钟丢掉的网络包的数目
#txcarr/s 发数据包时，每秒钟载波错误发生的次数
#rxfram   收数据包时，每秒钟发生的帧对其错误的次数
#rxfifo   收数据包时，每秒钟缓冲区溢出的错误发生的次数
#txfifo   发数据包时，每秒钟缓冲区溢出的错误发生的次数
```



## awk

```sh
# 过滤，由'/ /'包裹
awk '/Reponse info/' proxy.06-20.log;  # 过滤含有Reponse info的行, 正则表达式将由 '/ /' 包裹
awk '!/Reponse info/' proxy.06-20.log; # 过滤不含有Reponse info的行
awk '/Request/;/Reponse/'  proxy.06-20.log  # 两个条件进行过滤(与)
awk '/1164691776/&&(/Request/ || /Reponse/)' proxy.06-20.log # 使用多个条件进行过滤
awk '/1164691776/&&(/Request/||/Reponse/)　{if($2 ~ /^11/) print $0}'  # 使用正则表达式作为过滤条件
awk '($2 ~ /^02:02/) {print $0}'   proxy.06-20.log

awk '/Reponse info/ {if (length($7) > 17) print $2, $7}' proxy.06-20.log  # 使用长度作为条件

# 统计, 行数
awk '($2 ~ /^02:02/)  {count++} END{print count}'  proxy.06-20.log
# 统计, 各状态的个数
netstat -ano | awk '/^tcp/ {t[$6]++} END{for(state in t) {print state, t[state]} }'
#有多行这种数据，2015-09-10 10:20:28 [info] synco: brand(1000)，求()中数字之和
awk '/synco/ {print $5}' system.log | awk -F '(' '{print $2}' | awk -F ')' '{print $1}' | awk '{a+=$0} END{print a}'

# 计算
awk '{a += $2} END{print a}' proxy.log

#使用 [ 作为分隔符， 默认用空格作为分隔符
awk -F '['   '/socket close/ {print $4}' proxy.06-25.log | sort | uniq

#改变某列的值(把第二列设置为空)
awk '{print $2=null,$0}' system.log

# 作为参数传入
CHS=(2011 2012 2014)
for CH in ${CHS[@]}
do
    cat seq.${CH} | awk -v chnn=${CH} 'if($2==chnn) { print $0}'
done
```



## vim
1.查找单词<br/> 
把光标移动到单词上: (1)只查单词，按下`*`；(2)模糊匹配，按下`g*`

2.查找<br/>
在normal模式下，可以使用正则表达式进行查找。
```
`/abc`,不分大小写，匹配所有
`/^abc`，abc开头
`/abc$`，abc结尾
`/abc/c`,大小写不敏感
`/abc/C`，大小写敏感
大小写敏感，也可以设置`set smartcase`; 大小写不敏感，也可以设置`set ignorecase`
```

3.模式匹配<br/>
```
`/[vim]`，匹配v,i,m中之一
`/[a-zA-z]`，匹配任意字母
`/[1-5]`，匹配1-5间的数字
`/.`，匹配任意字符
`/vi*m`，`*`前字符出现大于等于0次。如vm, vim, viim, viiim
`/vi\+m`，`+`前字符出现大于等于1次。如vim, viim, viiim
`/vi\?m`, `?`前字符出现0次或一次。如vm, vim
`/^vim`, 匹配开头
`/vim$`，匹配结尾
`/vim\|kv`, 匹配vim或kv
```

4.替换<br/>
`:{作用范围}s/{目标}/{替换}/{替换标识}`
 
当前行: `:s/foo/bar/g` <br/>
全文: `:%s/foo/bar/g` <br/>
第4-10行: `:4,10s/foo/bar/g` <br/>
当前行和接下来2行: `:.,+2s/foo/bar/g` <br/>

`g`，global,全局替换；`i`，大小写不敏感； `I`，大小写敏感； `c`, 需要确认

`:s/foo/bar/gI`等价与`:s/foo\C/bar/g`

去掉windows格式末尾的\r\n, `:%s/^M//g`, 在命令中，^M的输入方式为: Ctrl+v, Ctrl+m,是一个字符，不是两个.






