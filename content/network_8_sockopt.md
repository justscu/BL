
# tcp socket options

 option name                  | functions |
------------------------------|-----------|
TCP_KEEPIDLE                  |设置连接的 idle 延迟时间，超过 idle 延迟后将触发发送 keepalive
TCP_KEEPINTVL                 |设置 keepalive 的发送间隔时间
TCP_KEEPCNT                   |在认定连接失败前，继续发送的 keepalive 报文的次数
TCP_CORK                      |停止发送不足一个 MSS 长度的数据，直至数据长度足够或者 TCP 连接断开
TCP_DEFER_ACCEPT              |在三次握手完成时 TCP 的状态将停留在 SYN_RECV并不进入 ESTABLISH 状态，直至接收到数据
TCP_INFO                      |查询 TCP 连接信息，包含统计信息
TCP_KEEPALIVE_ABORT_THRESHOLD |在放弃之前尝试产生成功的 keepalive 多长时间
TCP_KEEPALIVE_THRESHOLD       |指定保活定时器的空闲时间
TCP_MAXSEG                    |获取当前 TCP 连接的 MSS 大小
TCP_NODELAY                   |禁用 Nagle 算法，小段会立即发送，无需等待先前的段被确认
TCP_QUICKACK                  |启用后，在接收到下一个数据包后立即发送 ACK 消息。 每次使用后，此标志将重置为零，即它是一次性选项。 新连接以确认所有数据包的模式开始，因此该值最初默认为 1
SO_KEEPALIVE                  |设置 TCP 连接的 keepalive 参数
SO_NONBLOCK                   |设置文件描述符非阻塞
SO_TIMESTAMP                  |使能/去使能 为接收到的数据添加`系统时间戳`的功能，时间戳的精度为微秒（us）
SO_TIMESTAMPNS                |使能/去使能 为接收到的数据添加`系统时间戳`的功能，时间戳的精度为纳秒（ns）
SO_TIMESTAMPING               |使能/去使能 为接收到的报文添加硬件时间戳的功能支持
SO_LINGER                     |设置或者获取当前关闭 socket 的`延迟时间`值 
SO_ERROR                      |返回错误的 errno 值
SO_PROTOCOL                   |以整数形式返回套接字协议类型
SO_ACCEPTCONN                 |在 getsockopt()中使用，获取套接字是否可 accept 
SO_BINDTODEVICE               |将此套接字绑定到特定的网络接口 
SO_CONNECT_TIME               |三次握手的时间限制 
SO_DEBUG                      |开启调试信息
SO_EXCLUSIVEADDRUSE           |防止其他套接字使用 SO_REUSEADDR 绑定到同一地址
SO_PRIORITY                   |设置在此套接字上发送的所有数据包的优先级 
SOF_TIMESTAMPING_TX_HARDWARE  |获取硬件生成的传输时间戳
SOF_TIMESTAMPING_SYS_HARDWARE |获得硬件传输时间戳调整系统时间戳
SOF_TIMESTAMPING_OPT_CMSG     |使用 cmsg API 传递时间戳
IP_TRANSPARENT                |允许调用应用程序绑定到非本地 ip 地址
SO_OOBINLINE                  |带外数据应与常规数据一致返回
SO_SNDBUF                     |设置发送窗口大小
SO_RCVBUF                     |设置接收窗口大小
SO_RCVLOWAT                   |设置触发接收操作的最小门限值
SO_SNDTIMEO                   |设置 socket 发送数据的超时时间
SO_RCVTIMEO                   |设置 socket 接收数据的超时时间
SO_REUSEADDR                  |地址复用
SO_RESUSEPORT                 |端口复用
SO_SNDLOWAT                   |设置触发发送操作的最小门限值
SOSOF_TIMESTAMPING_SYS_ HARDWARE|获取调整到系统时基的硬件传输时间戳
SO_TYPE                       |返回 socket 类型
SOCK_NONBLOCK                 |在socket()和accept()中支持，在新的打开文件描述符上设置`O_NONBLOCK`文件状态标志，从而节省对 fcntl(2) 的额外调用以实现相同的结果
SOCK_CLOEXEC                  |在新文件描述符上设置 close-on-exec(FD_CLOEXEC) 标志


# UDP socket options

 option name                  | functions |
------------------------------|-----------|
SO_NONBLOCK     |设置文件描述符非阻塞 
SO_TIMESTAMP    |使能/去使能 为接收到的数据添加系统时间戳的功能，时间戳的精度为微秒(us)
SO_TIMESTAMPNS  |使能/去使能 为接收到的数据添加系统时间戳的功能，时间戳的精度为纳秒(ns)
SO_TIMESTAMPING |使能/去使能 为接收到的报文添加硬件时间戳的功能
SO_SNDBUF       |设置发送窗口大小
SO_RCVBUF       |设置接收窗口大小
SO_RCVLOWAT     |设置触发接收操作的最小门限值
SO_SNDLOWAT     |设置触发发送操作的最小门限值
SO_SNDTIMEO     |用来设置socket发送数据的超时时间
SO_RCVTIMEO     |用来设置socket接收数据的超时时间
SO_REUSEPORT    |支持多个进程或者线程绑定到同一端口，提高服务器程序的性能
SO_REUSEADDR    |支持地址复用
SO_TYPE         |返回socket类型
SO_PROTOCOL     |以整数形式获取套接字协议，返回一个值，例如 IPPROTO_SCTP
SO_BINDTODEVICE |有多个接口，例如 eth0, eth1, ethx......，在创建套接字的时候绑定相应的接口发送数据
SO_DEBUG        |开启套接字调试模式
SO_ERROR        |返回发生的错误的 errno
SO_EXCLUSIVEADDRUSE           |防止其他套接字使用`SO_REUSEADDR`绑定到相同地址
SO_PRIORITY                   |设置在此套接字上发送的所有数据包的优先级
SOF_TIMESTAMPING_TX_HARDWARE  |获取硬件生成的传输时间戳
SOF_TIMESTAMPING_SYS_HARDWARE |获得硬件传输时间戳调整系统时间戳
SO_BROADCAST                  |支持从广播地址发送/接收数据
SO_LINGER                     |设置或者获取当前关闭 socket 的延迟时间值

协议栈在发送UDP报文时，如果当前待发送的数据超过`MTU`，会对数据执行分片处理。
执行IP分片后，同一个报文的全部分片中的16位标识字段的值将是同一个值，用于重组时识别分片报文是否属于同一个报文。
除了首片外，其余分片将不再包含UDP头部信息。

协议栈在接收到IP头部中分片置位的数据时，如果判断为一个合法的分片，会进行分片重组，
直至将一个报文的全部分片全部收集完成，重组为一个完整的UDP数据包。


# Multicast socket options

 option name              | functions |
--------------------------|-----------|
IP_ADD_MEMBERSHIP         |指定本地接口加入一个不限源的组播组
IP_DROP_MEMBERSHIP        |离开指定的本地接口上不限源的组播组
IP_BLOCK_SOURCE           |对于一个本地接口上已存在的一个不限源的组播组，在本套接字上阻塞接收来自某个源的多播组报文
IP_UNBLOCK_SOURCE         |开通一个先前被阻塞的源
IP_ADD_SOURCE_MEMBERSHIP  |在一个指定的本地接口上加入一个特定源的组播组
IP_DROP_SOURCE_MEMBERSHIP |在一个指定的本地接口上阻塞一个组播组的特定源。
IP_MULTICAST_IF           |指定组播数据包的外出接口
IP_MULTICAST_TTL          |设置发送报文的 TTL
IP_MULTICAST_LOOP         |开启或禁止组播数据报的本地自环

一般在发送数据前，使用`setsockopt`的`IP_MULTICAST_IF`选项设置报文出口。但如果提前`bind`了本地IP，那么也可以不使用`IP_MULTICAST_IF`.
