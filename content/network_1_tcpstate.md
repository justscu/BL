## tcp状态分析

使用`netstat -ant`命令，可以看到tcp的一些状态信息，如LISTEN、ESTABLISHED、CLOSE_WAIT、TIME_WAIT等。
当系统中有大量的CLOSE_WAIT/TIME_WAIT时，就要小心了，可能出问题了。

`netstat -ano | awk '/^tcp/ {t[$6]++} END{for(state in t) {print state, t[state]} }'`命令，可以统计各个状态的个数。
除此外，还应该看看每个客户端(即每个IP)到本机有多少个tcp连接，各连接处于什么状态。 


### 1 socket关闭时的状态变迁图

![time_wait1](https://github.com/justscu/BL/blob/master/pics/network_1_1.png)

TCP的关闭有2种方式：(1)TCP四次挥手；(2)发RST包。

`int shutdown(int sockfd, int how); `，how可以是SHUT_WR、SHUT_RD、SHUT_RDWR中的一个。

上图中的close()，可能为shutdown()，也可能是close()。`shutdown(fd, SHUT_RDWR)`等价于`close(fd)`。
- （1）若A关闭socket的读通道后，B再往该socket写数据，会引发一些问题。B在第一次写时，A会返回RST包，第二次写时，B的应用程序会收到SIGPIPE信号。
- （2）在发送RST包关闭连接时，不必等缓冲区的包都发出去，直接就丢弃缓冲区的包发送RST包。而接收端收到RST包后，也不必发送ACK包来确认。

### 2 CLOSE_WAIT
CLOSE_WAIT只会出现在被动关闭的一方(客户端和服务器端均可能出现)。

A调用`shutdown(fd,SHUT_WR)`，A的协议栈就会往B发送fin包，B的协议栈收到fin包后，回复一个ack包，同时B的应用程序（read/recv等）会收到长度为0的数据包。B回复ack包后，B就进入CLOSE_WAIT状态。
- （1）若B调用close()，B的协议栈向A发送fin包，接下来B进入LASK_ACK状态。当B收到A的ack包后，B就进入CLOSE状态，之后关闭从B到A的写通道（关闭socket，释放fd等资源）。 
- （2）若B的应用程序不处理read返回0的数据包；或者忙于其它事情，而没有处理该数据包；或者B还有数据需要发给A，而没有调用close()函数。这样的话，B就不会给A发送fin包，从而导致B一直处于CLOSE_WAIT状态，同时Ａ处于FIN_WAIT_2状态。 

问题：若是因为B的程序错误，导致B没有调用close()函数，B就会一直处于CLOSE_WAIT状态。
这样会导致Ａ和B间的链接一直存在，占用系统的fd、端口和内存资源。一个系统的fd是有限的，过多的CLOSE_WAIT，会导致服务器端的fd耗尽，新的客户端无法连接上服务器。服务端的accept失败，错误信息为**"too many open files"**。可以使用**lsof**命令查看一个进程打开的资源信息。

出现大量CLOSE_WAIT的解决方法：
- （1）正确编写程序。 
- （2）若read()返回0，则要close该连接。 
- （3）若read()返回负值，需要检查errno，若errno != EAGAIN，则close该连接。 
- （4）扩大系统的fd。 
- （5）在AB间加心跳。若B是服务器的话，可以在B端加心跳检测包。B处于CLOSE_WAIT时，AB间的socket处于半关闭状态，A到B是关闭的，B到A是打开的。A能收到B的心跳包，但B不会收到A回复的数据包。心跳包，即可以在应用程序添加一个专门的beat-heart线程，也可以打开tcp协议栈的TCP_KEEPALIVE属性，由内核处理。 


### 3 TIME_WAIT
TIME_WAIT只会出现在主动关闭的一方（客户端和服务器端均可能出现）。

上图中，B调用close()后，B的协议栈会往Ａ发送一个fin包，之后B进入LAST_ACK状态，A的协议栈收到fin包后，会回复一个ack包，同时A处于TIME_WAIT状态。
A要先进入TIME_WAIT状态，大概在2MSL时间后，才会进入CLOSE状态，并释放资源。

**MSL**(Maximum Segment Lifetime)，报文被丢弃之前在网络上生存的最大时间，常为2分钟；在实际应用中，使用的是TTL，而不是定时器。

设置2MSL的目的：
- （1）可靠的关闭tcp。当A发给B的ack没有正常到达B时，B会在超时后重新发fin包给A。新的fin包到达A的时间，最多为2MSL。 
- （2）经过2MSL后，B的所有的报文，都将在网络上消失（避免新启动的程序使用老的报文）。 

在2MSL后，A的fd所占用的端口及资源，是不能再被重复使用的（报**"address already in use"**错误）。当然，端口在被设置了SO_REUSEADDR属性后，可以被重用；但被占用的fd及资源，是不能够被重用的。
在2MSL时间段内，B发给A的迟到的报文将被丢弃。

TCP中相同的socket-pair对(即相同的源IP源port，目的IP目的port)是不能够被重复使用的。

A处于2MSL时间段内，所带来的问题：
- （1）若A是客户端，那么可以另外随机选取一个端口来避免这个问题；  
- （2）若A是服务器，A必须使用原来的端口（客户端靠该端口来连接）。若客户端想连接到A，必须使用不同的port（生成不同的socket pair对），或者等2MSL时间以后再连接。 

一般系统中不会出现大量的TIME_WAIT，除非受到攻击。如发现系统存在大量TIME_WAIT状态的连接，通过调整内核参数解决，`vim /etc/sysctl.conf`，编辑文件，加入以下内容：
```sh
#表示开启SYN Cookies。当出现SYN等待队列溢出时，启用cookies来处理，可防范少量SYN攻击，默认为0，表示关闭；
net.ipv4.tcp_syncookies = 1 
#表示开启重用。允许将TIME-WAIT sockets重新用于新的TCP连接，默认为0，表示关闭；
net.ipv4.tcp_tw_reuse = 1 
#表示开启TCP连接中TIME-WAIT sockets的快速回收，默认为0，表示关闭。  
net.ipv4.tcp_tw_recycle = 1 
#修改系统默认的 TIMEOUT 时间
net.ipv4.tcp_fin_timeout = 30 
```
然后执行`/sbin/sysctl -p`让参数生效。


### 4 ESTABLISHED

处于ESTABLISHED时，AB两端都可以发送数据包。 

对没有开启心跳的服务器端，用`netstat -anot`，可能会观察到同一个客户端(ip地址)有多个与服务器的链接，而且大多处于ESTABLISHED状态。这可能是有问题的。

比如，A为客户端，B为服务器。在某个时间点，A的主机突然挂掉重启，A没有发送给B一个fin包，这段时间内，B也不主动给A发送数据包。从B端看，这条socket一直处于ESTABLISHED状态。A的主机重启后，与B建立一条新的链接。这样的话，从B看，AB间有2个socket处于ESTABLISHED状态，实际上只有一个是有用的。

解决的方法就是在AB间加心跳。


### 5 开启心跳

既可以在应用层添加代码，也可以设置内核参数让tcp协议启用心跳机制。无论哪种方法，都需要在应用层开启**SO_KEEPALIVE**属性。
#### 5.1 服务器加心跳的方法
```cpp
// SO_KEEPALIVE 保持连接,检测对方主机是否崩溃，避免永远阻塞于TCP连接的输入
int keepAlive = 1;
if (0 != setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)))
{
    std::cout << "socketopt error." << strerror(errno) << std::endl;
    close(m_fd);
    return -1;
}

// 当TCP空闲1分钟后，开始检测,发送keepAlive数据
int idleTime = 120;
if (0 != setsockopt(m_fd, SOL_TCP, TCP_KEEPIDLE, &idleTime, sizeof(idleTime)))
{
    std::cout << "socketopt error." << strerror(errno) << std::endl;
    close(m_fd);
    return -1;
}
// 每隔20s重，发送一次keepAlive数据
int intervalTime = 20;
if (0 != setsockopt(m_fd, SOL_TCP, TCP_KEEPINTVL, &intervalTime, sizeof(intervalTime)))
{
    std::cout << "socketopt error." << strerror(errno) << std::endl;
    close(m_fd);
    return -1;
}
// 一共发送 10个keepAlive数据
int count = 10;
if (0 != setsockopt(m_fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count)))
{
    std::cout << "socketopt error." << strerror(errno) << std::endl;
    close(m_fd);
    return -1;
}
```

#### 5.2 设置内核参数
内核启用tcp心跳机制，定时发送心跳包来检测对方是否在线，若在规定时间内收不到对方的回复包，就认为对方不存在了。
```sh
cd /proc/sys/net/ipv4
# The number of seconds a connection needs to be idle before TCP begins sending out keep-alive probes.
# Keep-alives are only sent when the SO_KEEPALIVE socket option is enabled.
# The default value is 7200 seconds (2 hours). An idle connection is terminated after approximately an 
# additional 11 minutes(9 probes an interval of 75 seconds　apart) when keep-alive is enabled.
echo 120 > tcp_keepalive_time #TCP不发数据包多久后开始探测，缺省值为2小时

# The number of seconds between TCP keep-alive probes.
echo 20 >　tcp_keepalive_intvl #两次探测的时间间隔

# The maximum number of TCP keep-alive probes to send before giving up and killing the connection 
# if no response is obtained from the other end.
echo 10 > tcp_keepalive_probes #最多探测多少次
```

### 6 扩大系统连接数(fd)的方法

查看现有的fd数目：`ulimit -a`，查看open files这一项；

第一种：编辑[/etc/profile]，在文件末尾，加上[ulimit -n 1000000]，保存文件；然后`source /etc/profile`。

第二种：编辑[/etc/security/limits.conf]，在文件末尾加上下面内容
```sh
    * soft nofile 65536
    * hard nofile 65536
```

### 7 RST

![rst](https://github.com/justscu/BL/blob/master/pics/network_1_2.png)

RST表示请求重置连接。

A发给B一个RST包时，丢弃发送缓冲区的包，直接发给B一个RST包（fin是需要等发送缓冲区的数据发完后，再发fin包）。B在收到RST包后，不必给Ａ回复ack包。

A发送给B一个tcp数据包，若B的协议栈不能够处理该数据包，就会向A发一个RST包，表明该数据包所标识的链接出现了错误，请求tcp协议栈清除该链接。

在4种情况下，B的协议栈会发给A一个RST包：
- （1）A向B的主机发SYN包，但B的主机上没有进程在监听该port。B的主机的协议栈会发给A一个RST包，A收到该包后，返回给应用层一个**"connection refused"**错误。 
- （2）B的协议栈想放弃一个与A的已经存在的链接。 
- （3）B的主机在某个时间内重启过了，而A不知情。A发送给B一个tcp数据包时，B的协议栈不能够处理该数据包，就会向A发RST包，表明该数据包所标识的链接出现了错误，请求tcp协议栈清除该链接。A的应用层会收到**"connection reset by peer"**错误。
- （4）B端关闭了socket的读，A仍然往该socket写数据。A在第一次写时会收到RST，第二次写时，A的应用程序会收到SIGPIPE信号。 

A发送一个包给不存在的ip时（如B主机崩溃时），会收到**"destination unreachable"**错误，即EHOSTUNREACH，由ICMP协议回复。 


### 8 EWOULDBLOCK/EAGAIN
read一个非阻塞的socket时，若socket中没有数据，就会返回该错误，表明应该过段时间再读。 


### 9 SIGPIPE

Broken pipe: write to pipe with no readers，即向一个没有reader的管道中写入数据，会引发broken pipe。

`int shutdown(int sockfd, int how);` how可以是SHUT_WR、SHUT_RD、SHUT_RDWR中的一个。
shutdown(fd, SHUT_RDWR)等价于close(fd)。

若A调用了close()，A会同时关闭读写通道，B的read会返回0，表明A已经关闭了该socket。若A继续往该socket写入数据，第一次会收到RST，第二次会收到SIGPIPE信号。若B的应用程序不忽略SIGPIPE信号，程序将直接退出。
- （1）write一个已经close掉的socket，第一次写时会返回0，第二次写时会返回EPIPE错误。若系统不忽略SIGPIPE信号，程序将直接退出。
- （2）read一个已经close掉的socket，会返回0。 


### 10 SO_REUSEADDR
- （1）当该socket处于TIME_WAIT时，该端口是可以重复使用的，不会报错。
- （2）当socket处于其他状态时，若重复使用端口，会报"port is already in use"错误，所以SO_REUSEADDR的作用是当处于TIME_WAIT时，有效。 


### 11 半打开(ESTABLISHED)
- （1）假设A处于正常状态，并且从B可达。A发给B数据包后，B回复给A一个ack包。
- （2）若B正常关机，则B的协议栈会发给A一个fin包。
- （3）若B突然宕机（或宕机后重启之中），则B不会发给A一个fin包。这时A并不知道B的状态，A和B间的连接一直存在（直到A主动发给B数据包），这种状态叫做半打开，且这种状态可能持续数天。若A发给B数据包，会返回数据不可达的错误。
- （4）若B宕机，之后正常重启。A发给B数据包后，A会收到RST包（连接被重置）。

A处于半打开状态时，也会占用fd和资源，需要用心跳来解决这个问题。 


#### 12 半关闭
A向B发送fin包，B回复ack。之后，A就处于FIN_WAIT_2状态，B处于CLOSE_WAIT状态，即半关闭状态。 


#### 13 延迟关闭

默认情况下，A调用close()函数，A的TCP协议栈会把发送缓冲区中的数据全部发送给B，等待B的ack确认，然后再给B发送FIN包，close()函数返回。`SO_LINGER`用来改变close()的默认动作。 
```cpp
struct linger {
    int l_onoff; // linger active ?
    int l_linger; // how long to linger for
};
```

* （1）l_onoff=0，close()时，tcp采用默认动作。
* （2）l_onoff=1，l_linger=0，close()时，A的协议栈向B发送RST包(不是FIN包)来关闭tcp，之后A跳过TIME_WAIT而直接进入CLOSE状态。若A的发送缓冲区中有数据，则被直接丢掉。[这种方法可以减少TIME_WAIT状态，但应该慎用]
* （3）l_onoff=1，l_linger>0，close()时，将进行延迟关闭，close()将会被阻塞。A的协议栈根据l_linger来设置一个超时，如果socket的发送缓冲区中仍残留数据，进程进入睡眠，内核进入定时状态去尽量去发送这些数据。
    * (a)在超时之前，如果所有数据都发送完且被对方确认，内核用正常的FIN|ACK|FIN|ACK四个分组来关闭该连接，close()成功返回。
    * (b)如果超时之时，数据仍然未能成功发送及被确认，则A向B发送RST包，之后A跳过TIME_WAIT而直接进入CLOSE状态，若A的发送缓冲区中有数据，则被直接丢掉，close()返回EWOULDBLOCK。

