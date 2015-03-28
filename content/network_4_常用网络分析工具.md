## 常用网络分析工具

### 1 tcpdump
tcpdump用来捕获网络中的数据包，对数据包进行分析；并可以设置过滤条件，只捕获特定的数据包。抓取数据包时需要将网卡设置为混杂模式（root才有权限）。帮助：man tcpdump。

基本命令
```sh
sudo tcpdump -i eth0 #抓取数据包
sudo tcpdump -i eth0 -w tcp.out #将数据包写入到tcp.out文件中
sudo tcpdump -r tcp.out         #读取tcp.out文件
sudo tcpdump -c 10              #接收10个数据包后退出

    -e  打印链路层信息 
    -v  输出一个稍微详细的信息，例如在ip包中可以包括ttl和服务类型的信息； 
    -vv 输出详细的报文信息； 
    -i  指定监听的网络接口； 
    -n  不把网络地址转换为名字. 
    如[ll-Aspire-1601M.39091 > 10.15.144.71.ssh: ] => [10.15.62.112.39091 > 10.15.144.71.22:] 
```
    
过滤
* `sudo tcpdump host, net, port XXXX`，例如：
    * tcpdump host 10.15.144.71，指明10.15.144.71是一台主机；
    * tcpdump net 10.15.144.0， 指明 10.15.144.0是一个网络地址；
    * tcpdump port 23，指明端口号是23。如果没有指定类型，缺省的类型是host。 
* `sudo tcpdump src, dst, dst or src, dst and src XXXX`，例如
    * tcpdump src 10.15.144.71 and dst 10.17.211.168，同时指定src和dst
* `sudo tcpdump ip,arp,rarp,tcp,udp [指定协议]`，例如
    * tcpdump arp

除了这三种类型的关键字之外，其他重要的关键字如下: gateway,broadcast,less,greater,还有三种逻辑运算，取非运算是 'not ' '! ',与运算是'and','&&';或运算 是'or','││'；这些关键字可以组合起来构成强大的组合条件来满足人们的需要，下面举几个例子来说明。
- `tcpdump ip host 10.15.144.71 and ! 10.17.211.168`，获取主机10.15.144.71除了和主机10.17.211.168之外所有主机通信的ip包; 
- `tcpdump host 10.15.144.71 and \(10.17.211.168 or 10.17.211.169 \) `，截获主机10.15.144.71 和主机10.17.211.168或10.17.211.169的通信，在命令行中使用括号时，一定要添加'\'。

### 2 netstat
Print network connections, routing tables, interface statistics, masquerade connections, and multicast memberships. 命令用于显示各种网络相关信息，如网络连接，路由表，接口状态 (Interface Statistics)，masquerade 连接，多播成员 (Multicast Memberships) 等等。 

netstat是一个用于监控进出网络的包和网络接口统计的命令行工具，可以用来监控网络性能，定位并解决网络相关问题。

常用命令：
* `netstat -ano  | grep 6020 | awk '{print $5}' | sort | uniq | wc -l`，用来计算有多少个IP连接到本地6020端口
* `netstat -ano | awk '/^tcp/ {t[$6]++} END{for(state in t) {print state, t[state]} }'` ，统计各状态的个数

#### 2.1 非本地通信数据包
```sh
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address               Foreign Address             State      
tcp        0      0     10.15.144.71:messageasap  10.15.94.41:nexus-portal    ESTABLISHED 
tcp        0      0     10.15.144.71:6020         10.15.94.65:50140      ESTABLISHED 
tcp        0      0     10.15.144.71:6020         10.15.144.7:58452      ESTABLISHED 
tcp        0    169   10.15.144.71:messageasap    10.17.214.217:51972    ESTABLISHED  --- 发送队列中，有数据包堆积
tcp        0      0   ::ffff:10.15.144.71:61616   ::ffff:10.15.97.222:38038    TIME_WAIT   
tcp   998535      0   ::ffff:10.15.144.71:61519   ::ffff:10.15.144.71:43774    ESTABLISHED  --- 接收队列中，有数据包堆积
tcp6       0      0    ::1:631                    :::*                         LISTEN     
udp        0      0    0.0.0.0:5353               0.0.0.0:*  
```

#### 2.2 本地通信unix-socket的数据包 
Proto显示连接使用的协议,RefCnt表示连接到本套接口上的进程号,Types显示套接口的类型,State显示套接口当前的状态,Path表示连接到套接口的其它进程使用的路径名。 
```sh
    Active UNIX domain sockets (servers and established)
    Proto RefCnt Flags       Type       State         I-Node          Path
    unix  2      [ ACC ]     STREAM     LISTENING     8499     /tmp/.X11-unix/X0
    unix  2      [ ACC ]     STREAM     LISTENING     11042    /tmp/ssh-rXkTPW1744/agent.1744
    unix  2      [ ACC ]     STREAM     LISTENING     11082    /tmp/.ICE-unix/1744
    unix  2      [ ]         DGRAM                    18186923 
    unix  3      [ ]         STREAM     CONNECTED     18039656 
    unix  3      [ ]         STREAM     CONNECTED     18039655 
    unix  3      [ ]         STREAM     CONNECTED     18039550 
    unix  3      [ ]         SEQPACKET  CONNECTED     15495    
    unix  3      [ ]         SEQPACKET  CONNECTED     15494    
    unix  3      [ ]         STREAM     CONNECTED     15491     /var/run/dbus/system_bus_socket
    unix  3      [ ]         STREAM     CONNECTED     15490     
```

#### 2.3 参数
```sh
netstat -a  显示所有选项，默认不显示LISTEN相关
netstat -l  仅列出有在 Listen (监听) 的服務状态
netstat -t  仅显示tcp相关选项 
netstat -u  仅显示udp相关选项
netstat -x  仅显示unix相关选项
netstat -lu 仅显示监听udp相关选项
netstat -o  显示socket定时器的信息
netstat -n  拒绝显示别名，能显示数字的全部转化成数字
netstat -p  显示程序名和进程号 
            如[tcp  0  0 10.15.144.71:36298  10.15.144.73:eforward  ESTABLISHED  26848/./pushproxy_V] 

    -r 显示路由信息，路由表 
    -e 显示扩展信息，例如uid等 
    -s 按各个协议进行统计 
    -c 每隔一个固定时间，执行该netstat命令。 

    提示：LISTEN和LISTENING的状态只有用-a或者-l才能看到 
```

### 3 TCP状态
![image](https://github.com/justscu/BL/tree/master/pics/tcp_state.png)

The state of the socket. Since there are no states in raw mode and usually no states used in UDP, this
column may be left blank. Normally this can be one of several values:
* LISTEN - The  socket is listening for incoming connections.  Such sockets are not included in the output
unless you specify the --listening (-l) or --all (-a) option.
* SYN_SENT - The socket is actively attempting to establish a connection.
* SYN_RECV - A connection request has been received from the network.
* ESTABLISHED - The socket has an established connection.
* FIN_WAIT1 - The socket is closed, and the connection is shutting down.
* CLOSE_WAIT - The remote end has shut down, waiting for the socket to close.
* FIN_WAIT2 - Connection is closed, and the socket is waiting for a shutdown from the remote end.
* LAST_ACK - The remote end has shut down, and the socket is closed. Waiting for acknowledgement.
* TIME_WAIT - The socket is waiting after close to handle packets still in the network.
* CLOSE - The socket is not being used.
* CLOSING - Both sockets are shut down but we still don't have all our data sent.
* UNKNOWN - The state of the socket is unknown.

### 4 lsof
list open file
- 某个进程打开了哪些文件描述符；
- 某个文件描述符被哪些进程打开了； 
```sh
lsof -p pid      # 显示指定的进程打开的所有文件描述符
lsof -t /opt/PushProxy/etc/9.xml      # 显示哪些进程打开了该文件
lsof -c ssh     # 显示指定的命令(ssh)打开的文件描述符
lsof -i @10.15.144.71:ssh      # 显示socket文件描述符
 
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF  NODE           NAME
ssh     3296   ll   3u     IPv4    91137   0t0  TCP   ll-Aspire-M:39729->10.15.144.71:ssh (ESTABLISHED) 
```

### 5 其它工具
- iotop，监控并显示实时磁盘I/O和进程的统计功能。在查找具体进程和大量使用磁盘读写进程的时候，这个工具就非常有用。
- iostat，用于收集显示系统存储设备输入和输出状态统计的简单工具。这个工具常常用来追踪存储设备的性能问题，其中存储设备包括设备、本地磁盘，以及诸如使用NFS等的远端磁盘。
- NetHogs，是一个开放源源代码的很小程序（与Linux下的top命令很相似），它密切监视着系统上每个进程的网络活动。同时还追踪着每个程序或者应用所使用的实时网络带宽。

[监控Linux性能的18个小工具](http://www.oschina.net/translate/command-line-tools-to-monitor-linux-performance?cmp&p=1#)
