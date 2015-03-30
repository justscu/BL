### 1 编译参数
- CFLAGS=-O0 -g  # -O0表示不优化，-g在编译时加上调试信息
- CFLAGS=-Werror # 把所有警告当做错误来处理
- CFLAGS=-Wall   # 开启对代码的检查，当不符合要求时，会给出warning

对于一些开源软件的配置，可以使用：
- 配置阶段：./configure --with-cc-opt='-g -O0'　或者　CFLAGS="-g -O0" ./configure
- make阶段：make CFLAGS="-g -O0"

由于.c/.cpp文件是否编译，是根据时间戳来的，可以采用下面的方法，强制编译：
- find . -name "*.c" | xargs touch　  ＃刷新时间戳
- make  -B                            #强制编译 

### 2 assert
**assert**是宏。assert在Debug下默认是开启的，在Release下默认关闭。可以使用`-DNDEBUG`关闭该宏，使用`-DDEBUG`开启该宏。

assert(pStr != NULL);

### 3 日志打印
打印日志，也是分析程序的一个好方法，特别是多线程程序。
```sh
#define H_RT
	printf("Error:" __DATA__ "at" __TIME__);
#endif /*H_RT*/
```
在编译选项中，加上-D参数。如gcc -DH_RT test.cpp -o test.out

### 4 awk
```sh
#过滤含有Reponse info的行, 正则表达式将由 '/ /' 包裹
awk '/Reponse info/' proxy.06-20.log;      
#过滤不含有Reponse info的行
awk '!/Reponse info/' proxy.06-20.log;
#两个条件进行过滤(与)
awk '/Request/;/Reponse/'  proxy.06-20.log
#使用多个条件进行过滤
awk '/1164691776/&&(/Request/ || /Reponse/)' proxy.06-20.log
#使用正则表达式作为过滤条件
awk '/1164691776/&&(/Request/||/Reponse/)　{if($2 ~ /^11/) print $0}'
awk '($2 ~ /^02:02/) {print $0}'   proxy.06-20.log
#统计多少行
awk '($2 ~ /^02:02/)  {count++} END{print count}'  proxy.06-20.log
#使用长度作为条件
awk '/Reponse info/ {if (length($7) > 17) print $2, $7}' proxy.06-20.log
#使用[作为分隔符， 默认用空格作为分隔符
awk -F '['   '/socket close/ {print $4}' proxy.06-25.log | sort | uniq
#统计各状态的个数
netstat -ano | awk '/^tcp/ {t[$6]++} END{for(state in t) {print state, t[state]} }'
```

### 5 源码查找
find ./ -name "*.cpp" -exec grep -Hn 'Insert' {} \;

### 6 strace
strace可以显示由用户空间发起的所有的系统调用。还可以显示参数及其返回值。
```sh
-p pid：通过进程号来指定被跟踪的进程
-o filename：将跟踪信息输出到指定文件。如：strace -T -o /home/ll/Desktop/a.out  ls
-f：跟踪其通过frok调用产生的子进程
-t：输出每一个系统调用的发起时间
-T：输出每一个系统调用消耗的时间
```
另外：pstack pid，查看调用栈


### 7 常用命令
- du -h --max-depth=1 #查看磁盘
- ./test 2 >> log | tee -a a.log  #将标准出错，输入到a.log中，所有的输出到log中
- cat  /proc/meminfo  #查看内存信息
- free , free -m , free -g
- free -m -s5   #5s显示一次，以M为内存单位进行显示
- ps -aux  #查看进程信息
- ps -eLf | grep “java” | wc -l    #监视java 进程数目
- netstat -ano | grep tcp | grep 侦听端口 | wc -l  # 查看网络客户连接数
- 查看IO性能：(1)top，查看CPU信息 (2)iostat (3)iotop
- top: c 切换显示命令名称和完整命令行； T 根据时间/累计时间进行排序。

![debug](https://github.com/justscu/BL/blob/master/pics/debug_4_2.png)
