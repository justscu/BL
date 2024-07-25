
```cpp

#if (USED_BYTE_ORDER == LITTLE_ENDIAN)
#define proto_ip  0x0008
#define proto_tcp   0x06
#define proto_udp   0x11
#endif


struct mac_hdr {
    uint8_t  dstaddr[6];
    uint8_t  srcaddr[6];
    uint16_t      proto; // IP, 0x0800; ARP, 0x0806.
};


struct ip_hdr {
#if (USED_BYTE_ORDER == LITTLE_ENDIAN)
    uint8_t     ihl:4; // 首部长度 X4
    uint8_t version:4; // 版本
#else
    uint8_t version:4; // 版本
    uint8_t     ihl:4; // 首部长度 X4
#endif

    uint8_t          tos; // 服务类型
    uint16_t   total_len; // 本IP报文长度
    uint16_t          id; // 标识
    uint16_t frag_offset; // 片偏移(需*8)
    uint8_t          ttl;
    uint8_t     protocol; // 协议类型
    uint16_t     chk_sum; // 首部校验和
    union {
        struct {
            uint32_t  src_ip; // 目的IP
            uint32_t  dst_ip; // 源IP
        };
        uint64_t ip;
    };
    // ... options ...
};


// TCP header
struct tcp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t   seq_no; // 本次发送的开始序号
    uint32_t   ack_no; // 确认序号
#if (USED_BYTE_ORDER == LITTLE_ENDIAN)
    uint8_t  reserved:4;
    uint8_t       thl:4; // tcp header length
    uint8_t  flag_fin:1; // 8 bit标志
    uint8_t  flag_syn:1;
    uint8_t  flag_rst:1;
    uint8_t  flag_psh:1;
    uint8_t  flag_ack:1;
    uint8_t  flag_urg:1;
    uint8_t  flag_ece:1;
    uint8_t  flag_cwr:1;
#elif (USED_BYTE_ORDER == BIG_ENDIAN)
    uint8_t       thl:4; // tcp header length
    uint8_t  reserved:4;
    uint8_t  flag_cwr:1; // 8 bit标志
    uint8_t  flag_ece:1;
    uint8_t  flag_urg:1;
    uint8_t  flag_ack:1;
    uint8_t  flag_psh:1;
    uint8_t  flag_rst:1;
    uint8_t  flag_syn:1;
    uint8_t  flag_fin:1;
#else
#error  "Endian is not LE nor BE..."
#endif
    uint16_t window_size;
    uint16_t chk_sum; // TCP check sum
    uint16_t  urgt_p; // 紧急指针
    // ... options ...
};


struct udp_hdr {
    uint16_t   src_port;
    uint16_t   dst_port;
    uint16_t     length; // UDP数据报长度(包含头,最小为8)
    uint16_t    chk_sum; // UDP校验和
};

```

#### UDP最大包长度

最大传输单元(MTU)与网络有密切关系，一般默认为1500 bytes。

- UDP包: 去掉MAC|IP|UDP头部、MAC校验和 (14+20+8+4=46), 剩下1454 bytes;
- TCP包: 去掉MAC|IP|UDP头部、MAC校验和 (14+20+20+4=58), 剩下1442 bytes;


实际上，在局域网内，一个UDP的最大长度为65535 - 46 = 65489 bytes。即调用`sendto`发送数据的最大长度为`65489`bytes。


#### UDP数据经历的阶段

可能丢包的地方:
- udp包载荷太大(超过 65489 bytes)
- udp发包速度太快
- 接收缓存太小
- recvfrom读数据太慢
- 交换设备

（1）网卡（网络适配器），网卡有自己的`硬件环形缓冲区`。当网络数据流量大于网卡可以处理的流量时，新数据会覆盖旧数据。

使用`netstat -i em1 udp`，查看`RX-DRP`列，可以查看网卡是否丢数据。

```sh
# netstat -i em1 udp

Kernel Interface table
Iface      MTU    RX-OK RX-ERR RX-DRP RX-OVR    TX-OK TX-ERR TX-DRP TX-OVR Flg
em1       1500 1711923485    0  23771 0     260015743      0      0      0 BMRU
em2       1500        0      0      0 0             0      0      0      0 BMU
em3       1500        0      0      0 0             0      0      0      0 BMU
em4       1500        0      0      0 0             0      0      0      0 BMU
lo       65536      773      0      0 0           773      0      0      0 LRU
p5p1      1500        0      0      0 0             0      0      0      0 BMU
p5p2      1500        0      0      0 0             0      0      0      0 BMU
```

上面em1网卡丢了23771个UDP数据，应当增加该网卡缓冲区的大小。

命令`ethtool -g em1`，可以得到缓冲区的设置。

```sh
# ethtool -g em1

Ring parameters for em1:
Pre-set maximums:
RX:      2047 #接收缓冲区可设置的最大值
RX Mini:    0
RX Jumbo:   0
TX:       511

Current hardware settings:
RX:      1023 #接收缓冲区当前值
RX Mini:    0
RX Jumbo:   0
TX:       511
```

第一部分(Pre-set)是预设的最大值，表示该参数的可设置的最大值。

第二部分(Current)是当前值。可以使用`ethtool -G em1 rx 1023`来设置RX的当前最新值。该设置立即生效，无需重启网卡和系统。


（2）之后数据就会被送到`操作系统的缓冲区`。 来自多个网卡的数据，都会经过该缓冲区。操作系统是否丢数据包，取决于：
操作系统缓冲区大小、系统负载、系统性能、网络负载等。

命令`watch -d "cat /proc/net/snmp | grep -w Udp"`可以查看系统丢包情况。重点关注RcvbufErrors。

```sh
Udp: InDatagrams NoPorts InErrors OutDatagrams RcvbufErrors SndbufErrors InCsumErrors
Udp:      136243    7396    35165        41128            0            2            0
```

- InDatagrams, udp收包量;
- NoPorts, packets to unknown port received(未知端口接收的数据包);
- InErrors, NoPorts之外的其它原因引起的UDP包无法送达到应用层错误。包括"接收缓冲区满"、"入包校验错误"、其它;
- OutDatagrams, udp发包量;
- RcvbufErrors, 接收缓冲区溢出的包数;
- SndbufErrors, 发送缓冲区溢出的包数;

检查系统缓存的当前设置`sysctl -a | grep net | grep -E "mem|backlog"`

```sh
# sysctl -a | grep net | grep -E "mem|backlog"

net.core.netdev_max_backlog = 1000
net.core.optmem_max = 20480
net.core.rmem_default = 212992
net.core.rmem_max = 212992
net.core.wmem_default = 212992
net.core.wmem_max = 212992
net.ipv4.igmp_max_memberships = 20
net.ipv4.tcp_max_syn_backlog = 1024
net.ipv4.tcp_mem = 763149   1017532  1526298 # min, pressure, max
net.ipv4.tcp_rmem =  4096     87380  6291456
net.ipv4.tcp_wmem =  4096     16384  4194304
net.ipv4.udp_mem = 765069   1020094  1530138
net.ipv4.udp_rmem_min = 4096
net.ipv4.udp_wmem_min = 4096
```

临时设置命令(os重启后失效)
```sh
sysctl -w net.core.rmem_default="16777216" # 16M
sysctl -w net.core.rmem_max="33554432"     # 32M
sysctl -w net.core.wmem_default="16777216"
sysctl -w net.core.wmem_max="33554432"

sysctl -w net.ipv4.udp_mem="16777216 33554432 33554432"
sysctl -w net.ipv4.udp_rmem_min="16777216"
sysctl -w net.ipv4.udp_wmem_min="16777216"
```

（3）之后数据被送到`应用程序的套接字缓存`。若应用程序不及时从应用程序的套接字缓存读走数据包，新数据会覆盖旧数据。
本层数据是否丢失取决于：套接字缓存大小、应用程序读取数据的速度。

命令`watch -d "cat /proc/pid/net/snmp | grep -w Udp"`可以查看每个进程的丢包情况。


#### 设置应用程序缓存大小

Windows，打开注册表的“HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Afd\Parameters”选项，
增加“DefaultReceiveWindow”字段，DWORD 类型，大小设置为 33554432(32M)。 
发送端，可以增加“DefaultSendWindow”字段，DWORD 类型，大小设置为 33554432(32M)

在程序中，使用下列代码查看缓存

```cpp
int32_t val = 0;
int32_t val_len = sizeof(val);
getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&val, (unsigned int *)&val_len);
getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&val, (unsigned int *)&val_len);
```

使用下面代码，设置缓存

```cpp
const int32_t size = 16 * 1024 * 1024;
// 接收缓冲区
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(int));
//发送缓冲区
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(int));
```
注意: client端，SO_RCVBUF须在connect前设置; server端，SO_RCVBUF须在listen前设置。
