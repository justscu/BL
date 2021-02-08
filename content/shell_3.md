

#### 1. htop 
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


##### 绑定cpu方法

meaning | cmd
----|--------
总物理CPU核数 | 物理CPU个数 * 每个物理CPU的核数
总逻辑CPU核数 | 物理CPU个数 * 每个物理CPU的核数 * 超线程数
查看CPU基本信息 | cat /proc/cpuinfo
物理CPU个数 | "physical id"
物理核(心)数| "cpu cores"
逻辑CPU个数 | "processor"

```cpp
// 1. 对进程绑核(进程中的所有线程) -- 逻辑核
// 整个进程的所有线程，均运行在5-6核心
void bind_process_to_cpu() {
    int32_t num = sysconf(_SC_NPROCESSORS_CONF); // count of cpu cores.

    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    int32_t ret = sched_getaffinity(0, sizeof(old_mask), &old_mask);
    if (ret) {
        std::cerr << "sched_getaffinity failed." << std::endl;
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        // 判断进程是否可以运行到该核心上
        if (CPU_ISSET(i, &old_mask)) {
            std::cout << "process is running processor : " << i << " ." << std::endl;
        }
    }

    // set
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(5, &mask); // <--- core 5
    CPU_SET(6, &mask); // <--- core 6
    ret = sched_setaffinity(0, sizeof(mask), &mask);
    if (ret) {
        std::cerr << "sched_getaffinity failed." << std::endl;
        return;
    }

    // get
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    ret = sched_getaffinity(0, sizeof(new_mask), &new_mask);
    if (ret) {
        std::cerr << "sched_getaffinity failed." << std::endl;
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &new_mask)) {
            std::cout << "process is running processor : " << i << " ." << std::endl;
        }
    }

    std::thread *th1 = new std::thread(std::bind(thread_x));
    std::thread *th2 = new std::thread(std::bind(thread_x));
    while (true) {
        // NOTHING.
    }
}

// 2. 对某个线程绑核 -- 逻辑核
// 该线程在cpu_id上执行
void bind_thread_to_cpu(int32_t cpu_id) {
    int32_t num = sysconf(_SC_NPROCESSORS_CONF); // count of cpu cores.

    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    int32_t ret = pthread_getaffinity_np(pthread_self(), sizeof(old_mask), &old_mask);
    if (ret) {
        std::cerr << "pthread_getaffinity_np failed." << std::endl;
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &old_mask)) {
            std::cout << "thread is running processor : " << i << " ." << std::endl;
        }
    }

    // set
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    ret = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if (ret) {
        std::cerr << "pthread_setaffinity_np failed." << std::endl;
        return ;
    }

    // get
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    ret = pthread_getaffinity_np(pthread_self(), sizeof(new_mask), &new_mask);
    if (ret) {
        std::cerr << "pthread_getaffinity_np failed." << std::endl;
        return ;
    }
    for (int32_t i = 0; i < num; ++i) {
        if (CPU_ISSET(i, &new_mask)) {
            std::cout << "thread is running processor : " << i << " ." << std::endl;
        }
    }

    while(true) {;}
}

```


#### 2. awk

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



#### 3. vim查找与替换
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






