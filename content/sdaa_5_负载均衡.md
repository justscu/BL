## 负载均衡

负载均衡的本质是数据包的转发，即如何将数据包转发到负载最小的服务器上去。
最常用的硬件方案有F5，软件方案有LVS+Keepalived。

![sdaa_3_1](https://github.com/justscu/BL/blob/master/pics/sdaa_3_1.png)

### 1 解决的问题
* 能够将大规模并发访问和数据流量分发到多台内部服务器上，减少用户的等待时间；
* 当有重负载的计算请求时，能够将请求分解成多个任务，并将这些任务分配到内部多个计算服务器上，收集处理内部计算服务器的处理结果，汇总结果并返回给用户；
* 负载均衡能够大大提高系统的处理能力、提高系统灵活性；
* 高可用：当某服务器出现故障时，不影响其它服务器和用户的运行和使用；
* 当后端某个服务器出现故障时，能够将该服务器从服务列表中删除，当服务器恢复时，再将该服务器加入到列表中；
* 可伸缩：能够不影响其它服务器和用户的情况下进行扩容；

### 2 网络层次上的负载均衡
* 二层负载均衡，是通过一个虚拟的MAC地址接收请求，然后再分配到真实的MAC地址；
* 三层负载均衡，是通过一个虚拟的IP地址接收请求，然后分配到真实的IP地址；
* 四层负载均衡，是通过一个虚拟IP+端口进行接收，然后分配到真实的服务器；
* 七层负载均衡，是通过一个虚拟的主机名或URL接收请求，然后分配到真实的服务器。可以根据URL，浏览器类别，语言等，将请求发给不同的内部服务器。

### 3 四层负载均衡
* 首先会配置frontend的IP:PORT与backend的IP:PORT映射关系。当有客户端请求到来时，会根据映射关系，将请求转发到backend的服务器上去。
* 工作在L4层的负载均衡器，不需要对客户端的数据包内容进行解析。如SYN包到来时，负载均衡器只需要选择一个最佳的内部服务器，将SYN包中的dst IP:PORT替换为内部服务器的IP:PORT，并直接转发给该内部服务器即可。对有些部署，可能还需要修改source IP:PORT，这样负载均衡器可以收到内部服务器返回的包。

![sdaa_3_2](https://github.com/justscu/BL/blob/master/pics/sdaa_3_2.png)

### 4 七层负载均衡
L7层负载均衡，是应用层的负载均衡。

负载均衡器需要先和客户端建立连接(TCP三次握手)，接收客户端发过来的报文，然后根据报文特定字段的内容，来选择内部服务器。L7层负载均衡器，是一个代理服务器，需要与客户端和内部服务器间都建立连接。
一般来说，L7层负载均衡的处理能力，低于L4层。

![sdaa_3_3](https://github.com/justscu/BL/blob/master/pics/sdaa_3_3.png)

优点：
- 能够更好的拓展内部网络。如：能够将使用英语的和使用汉语的客户端请求，发送到不同的内部服务器。
- 能够将对图片的请求，发送到图片服务器，同时图片服务器可以加缓存。
- 能够提前过滤掉一些非法的请求和无用的数据包，而不用将这些请求发送到内部服务器，减轻内部服务器的压力。

缺点：
- 速度上，不如L4层快

### 5 负载均衡算法
- 轮循（Round Robin）：将每次请求，轮流的分配给内部服务器。当内部服务器的软硬件配置相当时，比较适合。
- 权重轮循（Weighted Round Robin）：根据内部服务器的配置不同，给每台服务器一个权重。如服务器A的权值被设计成1，B的权值是3，C的权值是2，客户请求依次发给[ABBBCC]。权重大的，分配到的任务就多。
- 随机（Random）：将请求随机分配给内部中的多个服务器。
- 权重随机（Weighted Random）：此种均衡算法类似于权重轮循算法，不过在处理请求分担时是个随机选择的过程。
- 响应速度（Response Time）：负载均衡器与内部服务器建立连接，并定时向内部服务器发ping包（或者其他包也行）。根据各服务器的响应速度，决定将用户请求发给哪台内部服务器。该均衡算法能够较好的反应内部服务器运行状况。
- 连接数（Connections）：有些内部服务器可能直接与客户端建立连接。这时可以询问内部服务器当前的用户连接数，并将新的用户请求发送给连接数最少的内部服务器。比较适合长连接。
- 处理能力（Processing　Capacity）：将内部服务器的CPU/内存/IO/当前负载，根据一定的算法换成负载值，并定时上报给负载均衡器。负载均衡器每次将用户请求转发给当前Load值最低的内部服务器。
- DNS轮询：DNS也可以用来做负载均衡。网络上，客户端一般通过域名来找到服务器的IP地址，DNS服务器在接收客户端查询时，按顺序将服务器的IP地址返回给客户端，来达到均衡的目的。比较适合全局负载均衡。
- Hash：将访问用户的IP地址进行hash，根据hash的结果来决定将该用户定向到哪台后端服务器。

### 6 HAProxy
Solutions to secure, optimize and speed up your network and application flows.
[HaProxy](http://haproxy.com/)由法国人用C语言开发的，高并发、高可用的负载均衡和反向代理服务器。

HAProxy提供负载均衡，同时对内部服务器进行健康检查。
可工作在L4和L7层，通过配置文件对haproxy进行配置和管理。同时还提供监控页面，很方便的掌控内部服务器的状态。
"haproxy-1.4.17/doc/configuration.txt"和"haproxy-1.4.17/doc/haproxy-en.txt"中，有对配置的详细说明。

#### 6.1 安装注意事项
```sh
# 使用libpcre，这是一个regex库，能够快速处理header(deletion,rewriting, allow, deny)
make TARGET=linux26 USE_STATIC_PCRE=1
sudo make install
# 用whereis haproxy，可用发现其安装在/usr/local/sbin/haproxy
```
常用的启动命令为：`/usr/local/sbin/haproxy -f /path/to/haproxy.cfg`

#### 6.2 监控页面
```sh
listen admin_status
    bind 0.0.0.0:10080
    mode http
    log 127.0.0.1 local3 err
    stats refresh 5s
    stats uri /admin?stats
    stats realm itnihao itnihao
    stats auth admin:admin
```
在浏览器中输入`http://ip:10080/admin?stats`，可以打开监控页面。

#### 6.3 L4层代理的配置
```sh
listen appli1-rewrite 0.0.0.0:10001 # 监听端口
    cookie SERVERID rewrite
    balance roundrobin # 负载均衡算法
    server app1_1 192.168.34.23:8080 cookie app1inst1 check inter 2000 rise 2 fall 5 # 后端服务器
    server app1_2 192.168.34.32:8080 cookie app1inst2 check inter 2000 rise 2 fall 5
    server app1_3 192.168.34.27:8080 cookie app1inst3 check inter 2000 rise 2 fall 5
    server app1_4 192.168.34.42:8080 cookie app1inst4 check inter 2000 rise 2 fall 5

frontend http-in #前台
    bind *:80
    mode    http
    option  httplog
    log     global
    default_backend htmpool #静态服务器池
backend htmpool #后台
    balance leastconn #负载均衡算法，最少连接数
    option  httpchk HEAD /index.htm HTTP/1.0 #健康检查
    server  web1 10.16.0.9:8085 cookie 1 weight 5 check inter 2000 rise 2 fall 3
    server  web2 10.16.0.10:8085 cookie 2 weight 3 check inter 2000 rise 2 fall 3
```

#### 6.4 L7层代理的配置
```sh
frontend test.com #定义前端服务器(haproxy)
    bind *:80 #监听地址
    # -i，忽略大小写
    acl static path_end -i .html .css .js # url 结尾文件
    acl web-client path_beg -i /vsphere-client # path_beg，以此路径开始
    acl bbs hdr_reg(host) -i ^(bbs.test.com|shequ.test.com|forum)
    acl monitor hdr_beg(host) -i monitor.test.com #定义ACL名称,对应的请求的主机头是monitor.test.com
    acl www hdr_beg(host) -i www.test.com

    use_backend cache.test.com if static
    use_backend vsphere-client if web-client
    use_backend monitor.test.com if bbs or monitor
    use_backend www.test.com if www
    
    default_backend www.test.com #指定默认的后端服务器

backend monitor.test.com #定义后端服务器群(web server/apache/nginx/iis..)
    mode http
    option forwardfor #后端服务器(apache/nginx/iis/*),从Http Header中获得客户端IP
    balance leastconn #负载均衡的方式,最小连接
    cookie SERVERID #插入serverid到cookie中,serverid后面可以定义
    option httpchk HEAD /check.html #用来做健康检查html文档
    #option httpchk HEAD /index.php HTTP/1.1\r\nHost:monitor.test.com #HTTP && Host
    server server1 10.0.100.70:80 cookie server1 check inter 2000 rise 3 fall 3 weight 3
    #服务器定义:
    #cookie server1表示serverid为server1;
    #check inter 2000 是检测心跳频率(check 默认 );
    #rise 3 表示 3次正确认为服务器可用;
    #fall 3 表示 3次失败认为服务器不可用;

    #weight 表示权重。
backend www.test.com
    ...

backend vsphere-client
    ...
```

#### 6.5 健康检查
- option httpchk，只检查后端server的端口是否可用，不能保证服务器可用
- option httpchk GET /index.html，GET后端服务器的web页面，基本上可以代表后端服务的可用性
- option httpchk HEAD /index.php HTTP/1.1\r\nHost:monitor.test.com，对后端服务访问的头部信息进行匹配检测

#### 6.6 haproxy实现持久连接的方法
* hash(ip)
    * 配置：balance source
    * haproxy将用户的IP经过hash后，定向到固定的后端服务器
    
* cookie识别
    * 配置：cookie SESSION_COOKIE insert indirect nocache
    * haproxy在web服务器发送给客户端的cookie中插入haproxy定义的后端服务器的COOKIE ID
    
* session 识别
    * 配置：appsession <cookie> len <length> timeout <holdtime>
    * haproxy将后端服务器产生的session和后端服务器标识存于haproxy的一张表中。客户端请求时，先查这张表，根据session分配后端server。

### 7 LVS
[LVS](http://www.linuxvirtualserver.org/)(Linux Virtual Server)，早已加入到Linux内核中。
LVS使用的是IP负载均衡技术(ipvs模块实现)。LVS安装在DirectorServer(DS)上，DS根据配置信息虚拟出一个ip(VIP)，同时根据配置信息，生成路由表，将用户的数据包按照一定的法则转发到后端RealServer(RS)上。

LVS进行L4层转发，且在内核中，速度极快。与Keepalived相比，缺少对内部服务器的健康检查，且存在单点故障。
LVS使用ipvsadm来配置ipvs。
数据包的转发，有三种方法：NAT/TUN/DR。

| 模式 | 网络要求 | 是否需要VIP | 端口映射 | DS参与回包 | ARP隔离 | 效率 |
| NAT | 同一网段 | 不需要 | 支持 | 是 | 不需要  | 最慢 |
| TUN | 既可以在同一物理网络，也可以在不同物理网络 | 需要 | - | 否 | - | 中等 |
| DR | 同一物理网络 | 需要 | 不支持 | 否 | 需要 | 最高 |

#### 7.1 NAT，网络地址翻译技术

![sdaa_3_5](https://github.com/justscu/BL/blob/master/pics/sdaa_3_5.png)

DS和RS，都在同一个网段，才能进行NAT模式转发。同时需要将DS的内部IP设置为内部网络的默认网关，RS在回包时，直接发给内部网关（即DS），由内部网关进行转发。
用户通过DS的外部IP地址进行访问。
NAT模式下，是可以进行端口映射的。

整个数据包流程如下：
```sh
假设用户IP为10.15.62.204，使用端口6356；
用户->DS: src(10.15.62.204:6356) dst(10.15.144.71:80)
DS的外部IP收到数据包后，内核进行转发
DS->RS2: src(10.15.62.204:6356) dst(192.168.1.12:10012)
RS2收到数据包，处理完毕后，将回包通过内部网关发出去
RS2->DS: src(192.168.1.12:10012) dst(10.15.62.204:6356)
DS收到会包后，由内核转发给用户
DS->用户: src(10.15.144.71:80) dst(10.15.62.204:6356)
```

#### 7.2 TUN，IP隧道技术

![sdaa_3_6](https://github.com/justscu/BL/blob/master/pics/sdaa_3_6.png)

调度器采用IP隧道技术，将用户的请求转发到RS，RS直接将响应发给用户。
TUN模式下，RS的回包，不需要经过DS，而是直接发给客户端。 

#### 7.3 DR，直接转发

![sdaa_3_7](https://github.com/justscu/BL/blob/master/pics/sdaa_3_7.png)

DS通过改写请求报文的MAC地址，将请求发给RS，RS直接将响应发给用户。
**DR方式的效率最高**，但要求DS和RS在同一物理网络。
DR模式不支持端口映射；同时RS需要抑制关于VIP的ARP应答。

整个数据包处理过程：
```sh
假设客户端IP 10.15.62.204，使用端口6356
用户->DS: src(10.15.62.204:63565) dst(10.15.144.120:80)
DS收到包后，通过负载均衡算法，选择RS2，改写包的目的MAC地址，将数据包发给RS2
DS->RS2: src(10.15.62.204:63565) dst(10.15.144.120:80)，目的MAC发生改变
RS2处理完毕后，将回包直接发给客户端
RS2->用户: src(10.15.144.120:80) dst(10.15.62.204:63565)
```
从转发效率来讲，NAT最差；TUN多了ip隧道的处理，次之；DR效率最高。

#### 7.4 配置脚本
以DR模式为例
```sh
# 安装LVS机器（即DirectorServer）脚本, lvs-DR.sh
# 设置VIP，并设置转发规则

#!/bin/sh

VIP=10.15.144.120
RIP1=10.15.144.102
RIP2=10.15.144.103
RIP3=10.15.144.104

. /etc/rc.d/init.d/functions
case "$1" in
    start)
        echo " start LVS of Director Server"
        /sbin/ifconfig eth0:1 $VIP broadcast $VIP netmask 255.255.255.255 up # 添加虚拟设备eth0:1和虚拟IP
        /sbin/route add -host $VIP dev eth0:1
        echo "1" >/proc/sys/net/ipv4/ip_forward # 允许转发

        /sbin/ipvsadm -C   #Clear IPVS table
        #set LVS
        /sbin/ipvsadm -A -t $VIP:10087 -s wrr -p 60 
        /sbin/ipvsadm -a -t $VIP:10087 -r $RIP1 -g -w 2 # -g 为DR模式， -w 为权重
        /sbin/ipvsadm -a -t $VIP:10087 -r $RIP2 -g -w 1
        /sbin/ipvsadm -a -t $VIP:10087 -r $RIP3 -g -w 1

        /sbin/ipvsadm  # 打印 ipvs 信息
    ;;
    stop)
        echo "close LVS Directorserver"
        echo "0" >/proc/sys/net/ipv4/ip_forward
        /sbin/ipvsadm -C
        /sbin/ifconfig eth0:1 down
    ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac

#-----------------------------------------------------------------------
# 在3台RealServer上，配置VIP，关闭对该VIP的ARP应答，所执行的脚本: rs-DR.sh
#!/bin/bash

VIP=10.15.144.120

. /etc/rc.d/init.d/functions
case "$1" in
    start)
        echo " Start LVS  of  Real Server"
        /sbin/ifconfig lo:0 $VIP netmask 255.255.255.255 broadcast $VIP up
        /sbin/route add -host $VIP dev lo:0
        # 忽略收到的arp广播
        echo "1" >/proc/sys/net/ipv4/conf/lo/arp_ignore   
        # 封装数据包时，忽略源ip(lvs服务器ip),而是将VIP做为源ip
        echo "2" >/proc/sys/net/ipv4/conf/lo/arp_announce  
        echo "1" >/proc/sys/net/ipv4/conf/all/arp_ignore
        echo "2" >/proc/sys/net/ipv4/conf/all/arp_announce
        sysctl -p > /dev/null 2>&1
        ;;
    stop)
        /sbin/ifconfig lo:0 down
        echo "close LVS Director server"
        route del $VIP > /dev/null 2>&1
        echo "0" >/proc/sys/net/ipv4/conf/lo/arp_ignore
        echo "0" >/proc/sys/net/ipv4/conf/lo/arp_announce
        echo "0" >/proc/sys/net/ipv4/conf/all/arp_ignore
        echo "0" >/proc/sys/net/ipv4/conf/all/arp_announce
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac
```
在DS和RS上运行相应的脚本后，LVS负载均衡系统就搭建完毕了。 

#### 7.5 ipvsadm
利用ipvs管理工具ipvsadm，查看DS内部情况
```sh
[root@10.15.144.71 lvs]# ipvsadm
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port Forward Weight ActiveConn InActConn
TCP 10.15.144.120:10087 wrr persistent 2
  -> 10.15.144.102:10087 Route 2 0 4
  -> 10.15.144.103:10087 Route 1 0 0
  -> 10.15.144.104:10087 Route 1 0 0 

# 在10.15.62.204上进行测试，常用命令有： 
ll@ll-rw:~$ wget http://10.15.144.120:10087/index.html
ll@ll-rw:~$ ab -n 100 -c 10 http://10.15.144.120:10087/index.html

[root@10.15.144.71 lvs]# ipvsadm -L -c
IPVS connection entries
pro expire state       source               virtual             destination
TCP 00:22 FIN_WAIT 10.15.62.204:50937 10.15.144.120:10087 10.15.144.102:10087
TCP 00:04 FIN_WAIT 10.15.62.204:50930 10.15.144.120:10087 10.15.144.102:10087
TCP 01:50 FIN_WAIT 10.15.62.204:50959 10.15.144.120:10087 10.15.144.103:10087
TCP 00:50 NONE 10.15.62.204:0 10.15.144.120:10087 10.15.144.103:10087
TCP 00:22 NONE 10.15.62.204:0 10.15.144.120:65535 10.15.144.102:65535
TCP 00:06 FIN_WAIT 10.15.62.204:50931 10.15.144.120:10087 10.15.144.102:10087

# 关掉RealServer服务器中的服务，再次在10.15.62.204测试时，收到的错误信息
ll@ll-rw:~$ wget http://10.15.144.120:10087/index.html
--2014-11-18 16:47:40-- http://10.15.144.120:10087/index.html
Connecting to 10.15.144.120:10087... failed: No route to host.
```
当DR模式时，数据包是直接从RS返回到客户端的，所以在RS上也需要虚拟出设备和IP（lo:0 10.15.14.120）。RS直接利用该IP进行返回。
同时，在同一子网内，有多个Server都拥有10.15.14.120这个IP。当其它机器进行ARP查询时(who has ip 10.15.1.120)，只能够由DS进行响应，其他Server不能够响应。这也是RS需要使用echo "0" >/proc/sys/net/ipv4/conf/lo/arp_ignore的原因。 

#### 7.6 tcpdump抓包分析
使用tcpdump在DS上抓包，可以看到从User来的数据包，直接转发给了RS；RS收到数据包后，也是直接回复给了User，不需要再经过DS转发。下图是在RS(10.15.14.102)上抓包的截图：

![sdaa_3_8](https://github.com/justscu/BL/blob/master/pics/sdaa_3_8.png)

#### 7.7 问题分析
LVS存在单点故障。当LVS服务器挂掉了，整个系统就完了。
LVS不检测内部服务器的状态。当内部服务器挂掉时，仍然将请求发往该服务器。

**LVS+Keepalived**解决方案：
- 实现主备模式解决单点故障。
- 内部服务器有问题时，将其从可用服务器列表中删除；当其恢复时，再将其加入到可用服务器列表。


### 8 Keepalived
[Keepalived](http://www.keepalived.org/)，用C语言编写的开源软件。
The main goal of this project is to provide simple and robust facilities for loadbalancing and high-availability to Linux system and Linux based infrastructures.
Keepalived可作负载均衡、故障隔离（主备服务器都安装Keepalived），对内部服务器做健康检查。
Keepalived在底层会调用ipvs内核模块，该模块是LVS的核心模块(为LVS提供IP负载均衡技术)。
[中文参考文档](http://www.keepalived.org/pdf/sery-lvs-cluster.pdf)

使用Keepalived的一个网络架构图如下(DR模式)：

![sdaa_3_9](https://github.com/justscu/BL/blob/master/pics/sdaa_3_9.png)

* SA和SB各有自己的真实IP，通过Keepalived，虚拟出VIP（10.15.62.208）。
* 当SA能正常工作时，VIP地址会在SA上生效；用户通过VIP地址，连接的其实是SA。
* 若SA不能正常工作，则VIP会立即在SB上生效，用户通过VIP地址，连接的是SB。
* SA与SB间通过VRRP协议（虚拟路由器冗余协议）进行交互。

#### （1）健康检查工作方式
* L3层，Keepalived会定时向服务器集群中的服务器发送ICMP包（即ping包）。若没有ping通，则将该服务器从可用列表中删除。
* L4层，Keepalived会定时检测服务器集群中的服务器的端口有没有启动。
* L7层，会定时检查应用层的情况，看是否正常提供服务。

#### （2）工作方式
Keepalived既可以单独作为主备备份来使用，增加系统可靠性；也可以调用ipvs(LVS核心模块)，实现负载均衡。
实现负载均衡时，配置文件中的lb_kind有`DR|TUN|NAT`三种选择。每种负载均衡的配置均要和单独配置LVS时的要求一致。
* NAT：能够实现端口映射。RealServer上不需要配置VIP，RealServers要在一个局域网内，且将DirectorServer配置成该局域网的默认网关，不需要关闭ARP应答。
* DR：不能实现端口映射。DirectorServer和RealServer上都需要配置VIP，且RealServer要关闭对VIP的ARP应答。
* TUN：和DR一致，但需要DirectorServer和RealServer间要实现隧道通信技术。

#### （3）官方提供的NAT模式例子
[配置实例](http://www.linuxvirtualserver.org/docs/ha/keepalived.html)，
RealServer配置内部IP和默认网关的脚本：
```sh
// nat.sh
#!/bin/bash
#description : Start Real Server
VIP=172.18.1.11
GW=172.18.1.254

. /etc/rc.d/init.d/functions

case "$1" in
    start)
        echo " Start LVS of RealServer"
        /sbin/ifconfig eth0:2 $VIP broadcast $VIP netmask 255.255.255.0 up
        /sbin/route add default gw $GW
    ;;

    stop)
        echo "Stop LVS of RealServer"
        /sbin/ifconfig eth0:2 down
        /sbin/route del default gw $GW
    ;;

    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac
```

#### （4）DR模式，Keepalived配置文件实例
```sh
! Configuration File for keepalived

# 全局定义
global_defs {
    notification_email { #当keepalived发生切换的时候，通知的email
      acassen@firewall.loc
      failover@firewall.loc
      sysadmin@firewall.loc
    }
    notification_email_from Alexandre.Cassen@firewall.loc #发件人
    smtp_server 10.15.62.1
    smtp_connect_timeout 30
    router_id LVS_DEVEL
}

# vrrp实例
vrrp_instance VI_http {
     state MASTER      # SA配置为MASTER，表明是主；SB配置为BACKUP，表明是备
     interface eth0    # 绑定的网卡
     virtual_router_id 51  # 主备的值要一致，相同的virtual_router_id为一个组，他将决定多播的MAC地址
     priority 100      # SB该值设置小点，为50
     advert_int 1      # 检查间隔, 1s
     authentication {
         auth_type PASS
         auth_pass 1111
     }
     virtual_ipaddress { #虚拟IP，可以有多个
         10.15.62.208
     }
    
#     notify_master /path_to_script/script_master.sh     # 表示当切换到master状态时，要执行的脚本
#     (or notify_master “/path_to_script/script_master.sh <arg_list>”)
#     notify_backup /path_to_script/script_backup.sh     # 表示当切换到backup状态时，要执行的脚本
#     (or notify_backup “/path_to_script/script_backup.sh <arg_list>”)
#     notify_fault /path_to_script/script_fault.sh
#     (or notify_fault “/path_to_script/script_fault.sh <arg_list>”)
}

# ------------------------------------ipvs 配置------------------------------------
# vip 和 真实服务器对应关系
virtual_server 10.15.62.208 19080 {  # 客户端通过该端口访问
     delay_loop 6  # 健康检查间隔6s
     lb_algo rr    # lvs调度算法rr
     lb_kind DR   # lvs负载均衡转发规则 NAT | TUN | DR
     nat_mask 255.255.255.0
     persistence_timeout 50 # 会话保持时间，即在50s内，被分配到同一个后端
     protocol TCP           # 健康检查用的是TCP还是UDP
#    sorry_server <IPADDR> <PORT> # 备用server，当后端server都挂了，发到该server上
     real_server 10.15.144.71 19080 {
#        notify_up   <STRING> | <QUOTED-STRING>  # 检查服务器正常(UP)后，要执行的脚本
#        notify_down <STRING> | <QUOTED-STRING>  # 检查服务器失败(down)后，要执行的脚本
         weight 1
         HTTP_GET { # check 健康检查，获取/index.html文件，并比较文件的md5值
             url {
               path /index.html
               digest 6c5a188c7350d3dc28893615306286c5
             }
             connect_timeout 3
             nb_get_retry 3
             delay_before_retry 3
         }
     }

     real_server 10.15.107.74 19080 {
         weight 1
         HTTP_GET {
             url {
               path /index.html
               digest 875ef20a0a68120069b87baaca624f12
             }
             connect_timeout 3
             nb_get_retry 3
             delay_before_retry 3
         }
     }
}
```

健康检查，可以直接在配置文件中进行配置，如HTTP_GET；也可以在配置文件中添加脚本的执行路径，让脚本定时执行并检查脚本返回的结果。最终的目的，就是检查后端服务器的状态。有下面4种health check 方式：
* TCP_CHECK: Layer4，使用非阻塞的，带超时的tcp连接，去连服务器，若连不上（timed-out），则服务器不可用；
* HTTP_CHECK: Layer5，使用HTTP GET　a sepcial url，然后计算返回值的MD5值，看是否跟预期值相匹配；
* SSL_GET: 同HTTP_CHECK；
* MISC_CHECK: 允许用户采用自定义脚本的方式去check，脚本的返回值必须是0或1。

在DR模式下，RealServers需要设置VIP（10.15.62.208）同时关闭对该VIP的ARP应答。
DR模式不能进行端口映射。

#### （5）启动Keepalived
启动命令：`sudo /usr/local/sbin/keepalived -f /usr/local/etc/keepalived/keepalived.conf`，用`-D`，会输出详细的log

Keepalived启动后会有三个进程：
* 父进程：内存管理，子进程管理等等
* 子进程：VRRP子进程
* 子进程：Healthchecker子进程

父进程使用WatchDog来监视子进程的状态。每个子进程打开unix domain socket，父进程连接该socket，并且每5s发送一次hello包。若父进程不能成功发送hello包给子进程，就直接重启子进程。

使用WatchDog有两个好处：①子进程若进入死循环，能够及时被发现；②当子进程不在了，父进程往子进程发包时，会受到broken pipe信号。
使用pstree命令可以查看进程树；使用lsmod，可以看到ip_vs内核模块被加载。ip_vs是LVS的核心模块。 

![sdaa_3_10](https://github.com/justscu/BL/blob/master/pics/sdaa_3_10.png)

#### （6）启动后，使用`ip add`命令，查看ip情况
```sh
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN 
group default
     link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
     inet 127.0.0.1/8 scope host lo
        valid_lft forever preferred_lft forever
     inet6 ::1/128 scope host
        valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast 
state UP group default qlen 1000
     link/ether ec:a8:6b:a4:fb:8f brd ff:ff:ff:ff:ff:ff
     inet 10.15.62.204/24 brd 10.15.62.255 scope global eth0
        valid_lft forever preferred_lft forever
     inet 10.15.62.208/32 scope global eth0    // 虚拟出来的ip
        valid_lft forever preferred_lft forever
     inet6 fe80::eea8:6bff:fea4:fb8f/64 scope link
        valid_lft forever preferred_lft forever
```

#### （7）查看日志`tail -f /var/log/message`
```sh
# SA的启动日志
Nov 13 16:55:17 ll-rw Keepalived_healthcheckers[19144]: IPVS: Service
already exists
Nov 13 16:55:17 ll-rw Keepalived_healthcheckers[19144]: message repeated
2 times: [ IPVS: Service already exists]
Nov 13 16:55:17 ll-rw Keepalived_healthcheckers[19144]: Using LinkWatch
kernel netlink reflector...
Nov 13 16:55:17 ll-rw Keepalived_healthcheckers[19144]: Activating
healthchecker for service [10.15.107.74]:19080
Nov 13 16:55:17 ll-rw Keepalived_healthcheckers[19144]: Activating
healthchecker for service [10.15.144.71]:19080
Nov 13 16:55:18 ll-rw Keepalived_vrrp[19145]: VRRP_Instance(VI_http)
Transition to MASTER STATE
Nov 13 16:55:19 ll-rw Keepalived_vrrp[19145]: VRRP_Instance(VI_http)
Entering MASTER STATE
Nov 13 16:55:19 ll-rw avahi-daemon[726]: Registering new address record
for 10.15.62.208 on eth0.IPv4.

# 停掉10.15.1０7.74上的httpA，会打印日志
Nov 13 16:59:30 ll-rw Keepalived_healthcheckers[19144]: Error connecting
server [10.15.107.74]:19080.
Nov 13 16:59:30 ll-rw Keepalived_healthcheckers[19144]: Removing service
[10.15.107.74]:19080 from VS [10.15.62.204]:19080
Nov 13 16:59:30 ll-rw Keepalived_healthcheckers[19144]: Remote SMTP
server [192.168.200.1]:25 connected.

# 重新启动httpA，输出的日志
Nov 13 17:01:36 ll-rw Keepalived_healthcheckers[19144]: MD5 digest
success to [10.15.107.74]:19080 url(1).
Nov 13 17:01:42 ll-rw Keepalived_healthcheckers[19144]: Remote Web
server [10.15.107.74]:19080 succeed on service.
Nov 13 17:01:42 ll-rw Keepalived_healthcheckers[19144]: Adding service
[10.15.107.74]:19080 to VS [10.15.62.204]:19080
```
在浏览器中输入`http://10.15.62.208:19080/index.html`，可以看到httpA或httpB返回的信息。

#### （8）ipvs的管理工具`ipvsadm`
```sh
# （1）查看内核版本
ll@ll-rw:~$  uname -a
Linux ll-rw 3.13.0-39-generic #66-Ubuntu SMP Tue Oct 28 13:31:23 UTC 2014 i686 i686 i686 GNU/Linux

#（2）建立软链接
ll@ll-rw:~$  sudo ln -sv /usr/src/linux-headers-3.13.0-39- generic/ /usr/src/linux
‘/usr/src/linux’ -> ‘/usr/src/linux-headers-3.13.0-39-generic/’

#（3）安装ipvsadm
ll@ll-rw:~$  sudo apt-get install ipvsadm

#（4） 启动
ll@ll-rw:~$  ipvsadm
Can not initialize ipvs: No space left on device
Are you sure that IP Virtual Server is built in the kernel or as module?
        #    使用非root启动时，报错
ll@ll-rw:~$ sudo ipvsadm
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
TCP  10.15.62.208:19080 rr persistent 50
  -> 10.15.107.74:19080           Masq    1      0          0         
  -> 10.15.144.71:19080            Masq    1      0          1
        #    可以看到连接的后端服务器情况

#（5） 查看客户端访问时，lvs的转发信息
ll@ll-rw:~$ sudo ipvsadm -lcn
IPVS connection entries
pro expire state       source             virtual                  destination
TCP 00:57  SYN_RECV    10.15.144.102:43614 10.15.62.208:19080    10.15.144.71:19080
TCP 00:24  NONE        10.15.62.208:0     10.15.62.208:19080    10.15.144.71:19080
TCP 00:02  NONE        10.15.144.71:0     10.15.62.208:19080    10.15.107.74:19080
TCP 00:43  SYN_RECV    10.15.144.102:43613 10.15.62.208:19080    10.15.144.71:19080
TCP 00:44  NONE        10.15.144.102:0    10.15.62.208:19080    10.15.144.71:19080
```

### 9 使用到的命令
```sh
# 在 eth0:1 设备上增加一个虚拟ip
/sbin/ifconfig eth0:1 10.15.62.208 netmask 255.255.255.255 up
# 或 
/sbin/ifconfig eth0:1 10.15.62.208 broadcast 10.15.62.208 netmask 255.255.255.255 up

# 删除虚拟ip
/sbin/ifconfig eth0:1 down
# 或　
ip addr del 10.15.62.208 dev eth0:1

# 查看ipvs内部信息
ipvsadm
watch -n 1 'ipvsadm -lcn' # 1s更新一次

# 允许LINUX内核做IP包的转发,即允许IP数据包从一个网络接口穿越到另一个网络接口, NAT模式必须，其它模式非必须
　echo "1" > /proc/sys/net/ipv4/ip_forward
# 不运行转发
echo "0" > /proc/sys/net/ipv4/ip_forward

# tcpdump 抓包
sudo tcpdump -i eth0 host 10.15.1.120 port 10087
```
