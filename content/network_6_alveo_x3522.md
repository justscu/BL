
[X3522](https://china.xilinx.com/products/boards-and-kits/alveo/x3.html#overview)
==


|                          |     |
|--------------------------|-----|
|               硬件规格尺寸 | [DS1002 - Alveo X3522 Data Sheet (DS1002) (v1.3)](https://docs.amd.com/r/en-US/ds1002-x3522/Summary)                               |
|        电压/温度/功耗等信息 | [PG348 - Alveo Card Management Solution Subsystem Product Guide (PG348) (v4.0)](https://docs.amd.com/r/en-US/pg348-cms-subsystem)  |
|             X3522安装手册 | [UG1522 - Alveo X3522 Installation Guide (UG1522) (v1.5)](https://docs.amd.com/r/en-US/ug1522-x3522-installation)                  |
|             X3522用户手册 | [UG1523 - Alveo X3522 User Guide (UG1523) (v1.4)](https://docs.amd.com/r/en-US/ug1523-x3522-user)                                  |
| Alveo X3系列efvi切换手册  | [XN-201257-CD - Alveo X3 Series ef_vi Conversion Guide (v5.0)](https://docs.amd.com/v/u/en-US/XN-201257-CD-X3-ef_vi-conversion)    |
|                 efvi手册 | [SF-114063-CD - ef_vi User Guide (v16.0)](https://docs.amd.com/v/u/en-US/SF-114063-CD-ef_vi_User_Guide)                             |
|            TcpDirect手册 | [SF-116303-CD - TCPDirect User Guide (v15.0)](https://docs.amd.com/v/u/en-US/SF-116303-CD-TCPDirect_User_Guide)                     |
|               Onload手册 | [UG1586 - Onload User Guide (UG1586) (v1.2)](https://docs.amd.com/r/en-US/ug1586-onload-user)                                       |
|                  PTP手册 | [UG1602 - Enhanced PTP User Guide (UG1602) (v1.1)](https://docs.amd.com/r/en-US/ug1602-ptp-user)                                    |


硬件规则尺寸
===

- 尺寸 : 半高半宽
- PCIe : Gen4 ×8 / Gen3 ×8
- OS support: Red Hat Enterprise Linux (RHEL)、Ubuntu
- 支持 Onload, TCPDirect, ef_vi
- 支持 RX checksum offload


X3安装手册
===

- The server must have a 64-bit x86 processor.
- The server must be able to accept a half height half length 8-lane PCIe board.

- 安装完X3522卡后，`lspci -v -d 10ee:`，查看是否有X3522的卡，一份正确的显示

```
    6f:00.0 Ethernet controller: Xilinx Corporation Device 5084
	    Subsystem: Xilinx Corporation Device 1e0b
	    Physical Slot: 1
	    Flags: bus master, fast devsel, latency 0, NUMA node 0
	    Memory at 20fffe100000 (64-bit, prefetchable) [size=1M]
	    Memory at 20fffe202000 (64-bit, prefetchable) [size=4K]
	    Expansion ROM at d5200000 [disabled] [size=1M]
	    Capabilities: [40] Power Management version 3
	    Capabilities: [60] MSI-X: Enable+ Count=64 Masked-
	    Capabilities: [70] Express Endpoint, MSI 00
	    Capabilities: [b0] Vital Product Data
	    Capabilities: [100] Advanced Error Reporting
	    Capabilities: [1c0] #19
	    Capabilities: [220] Transaction Processing Hints
	    Capabilities: [e80] Vendor Specific Information: ID=0020 Rev=0 Len=010 <?>
	    Kernel driver in use: xilinx_efct
	    Kernel modules: xilinx_efct

    6f:00.1 Ethernet controller: Xilinx Corporation Device 5084
	    Subsystem: Xilinx Corporation Device 1e0b
	    Physical Slot: 1
	    Flags: bus master, fast devsel, latency 0, NUMA node 0
	    Memory at 20fffe000000 (64-bit, prefetchable) [size=1M]
	    Memory at 20fffe201000 (64-bit, prefetchable) [size=4K]
	    Expansion ROM at d5100000 [disabled] [size=1M]
	    Capabilities: [40] Power Management version 3
	    Capabilities: [60] MSI-X: Enable+ Count=64 Masked-
	    Capabilities: [70] Express Endpoint, MSI 00
	    Capabilities: [b0] Vital Product Data
	    Capabilities: [100] Advanced Error Reporting
	    Capabilities: [220] Transaction Processing Hints
	    Capabilities: [e80] Vendor Specific Information: ID=0020 Rev=0 Len=010 <?>
	    Kernel driver in use: xilinx_efct
	    Kernel modules: xilinx_efct
```

- 安装X3522需要的驱动软件: `Auxiliary bus driver`和`Network driver`.
    RadHat系统使用RPM包，在安装时，需要跟OS匹配的RPM版本。
    
    `ls /lib/modules/$(uname -r)/build`查看是否已经安装了编译RPM需要的系统包。
    若没有安装的话，用`yum install kernel-devel-$(uname -r)`进行安装。
    
    
#### Auxiliary bus driver

X3522有对应的驱动，需要特定的版本。
查看驱动是否安装：`ls -d /sys/bus/auxiliary`，若没有安装的话，需要下载对应的版本，进行RPM编译和安装。


```
    # 编译
    rpmbuild --rebuild <source_rpm_path>
    # 安装
    rpm -Uvh <path>/kernel-module-auxiliary-<os_version>-<module_version>.rpm
    # 重新加载驱动
    modprobe –r auxiliary
    modprobe auxiliary

```


#### Network driver

X3522有对应的网络驱动

```
    # 编译
    rpmbuild --rebuild <source_rpm_path>
    # 安装
    rpm -Uvh <path>/kernel-module-xilinx_efct-<os_version>-<module_version>.rpm
    # 重新加载驱动
    modprobe –r xilinx_efct
    modprobe xilinx_efct
```


#### 配置网络

查看当前有哪些网口 `ip link` 或 `ifconfig -a`

查看驱动信息 `ethtool -i <interface>`, 检查网口状态 `ethtool -S <interface>`

```
# ethtool -i ens1f1d1
    driver: xilinx_efct  <---驱动
    version: 1.6.3.0
    firmware-version: 1.16.1.8
    expansion-rom-version: 
    bus-info: 0000:6f:00.1
    supports-statistics: yes
    supports-test: yes
    supports-eeprom-access: no
    supports-register-dump: no
    supports-priv-flags: no
```

配置IP
```
# IP
ip addr add 172.8.9.100/24 dev eth0
ip addr del 172.8.9.100/24

# 激活端口
ip link set eth0 up


# 网关
ip route add 172.8.9.0/24 via 172.8.9.1 # 路由通过网关172.8.9.1
ip route add 172.8.9.0/24 dev eth0      # 添加特定接口路由

ip route del 172.8.9.0/24

ip route show

# 添加默认路由
ip route add default via 172.8.9.1
```


X3用户手册
===

在200bytes，1/2 RTT, X3522比X2522快230ns. PCIe4比PCIe3快80ns.

driver管理`RX queues`，并在多个应用之间共享, 共享queue的应用程序使用同样的L3.

TX只支持CTPIO.

X3522使用的驱动为`xilinx_efct`，由onload提供. 
X3522使用`auxiliary bus driver`作为驱动，由AMD提供.
使用`onload`，必须使用`xilinx_efct`和`auxiliary bus driver`.

`TCPDirect`现在提供在它自己的软件包中，与`Onload`分开供应.


#### 迁移 - Onload

无需修改应用程序.
在不共享CTPIO apertures的情况下，最多可以创建15个Onload栈.

onload utilities(工具)仅仅与onload package(包)交互，`onload_tool`不会重新加载`xilinx_efct`驱动程序.


#### 迁移 - TCPDirect

不支持`TX-alternatives `、`Overlapped receives`.

确保没有打开`PIO`.

使用动态库的应用程序，没有上述问题，可以不用重新编译.
使用静态库的应用程序，需要重新编译.


#### 迁移 - ef_vi

X3卡支持发送`jumbo`包，不支持收`jumbo`包.

使用新的事件(events)来接收数据，使用新的函数(call)来发送数据.

驱动程序(driver)管理RX缓存, 不需要再调用post*函数来处理数据包描述符, 但需要在收到数据包后释放.

TX 仅仅支持CTPIO.

必须过滤不需要的数据，包括非TCP/UDP帧(frame).

一些示例程序不支持X3522.


#### 迁移 - sfptpd

X3522需要一个更新版本的sfptpd，以识别其新的PCIe供应商ID.

它将与其他AMD/Solarflare NIC（网络接口卡）以相同的方式被处理, 不需要对配置进行任何更改.


#### 队列处理

如何处理队列(queue handling)，是X3522跟之前网卡的最大区别.

之前的网卡(如X2522)将接收队列、发送队列、事件队列以及中断捆绑在一起，并称之为虚拟接口（VI）。
它们提供了一个非常大的数量（通常是数千个）的VI，可以根据需要动态分配给应用程序。

X3522拥有完全独立的接收和发送队列，可以更灵活地进行分配。
它们可以被指向不同的事件队列，并且事件队列可以共享。
X3522提供的队列数量较少（通常每个CPU一个，但有一个总体限制），应用程序可以根据需要附加到它们。


#### filter

硬件filter支持local IP address/port/Protocol, 或者MAC address + VLAN.

每个网口最多支持512个过滤器.


#### X3522 驱动

使用新的驱动`xilinx_efct`. 

Onload继续提供X3522前身所使用的`sfc`驱动程序，但它不提供`xilinx_efct`和辅助总线驱动程序(auxiliary bus drivers)。
同样，Onload脚本和工具继续与`sfc`驱动程序交互，不与`xilinx_efct`和辅助总线驱动程序交互。

重新加载`auxiliary bus` `xilinx_efct` and `Onload drivers`的顺序为:
```
    # onload_tool unload --onload-only
    # modprobe -r xilinx_efct
    # modprobe -r auxiliary
    
    # modprobe auxiliary
    # modprobe xilinx_efct
    # onload_tool reload --onload-only
```


#### 配置接收队列大小

```
    ethtool -g <interface>
    ethtool -G <interface> [rx N]
```


#### 调优 - 大页内存(Huge Pages)

- 大页内存(Huge Pages). Onload/TCPDirect/ef_vi均使用大页内存. 使用前，先确认网卡需要的大页内存的数量。

- 查看网卡中断对应的接收队列(receive queues)数量. 假设网卡为ens1f1d1.
    `cat /proc/interrupts | grep "<interface_name>-rx" `
    输出结果有几行，便有几个接收队列.

- 查看当前网卡接收队列(rx_ring_buffer_size)大小 `ethtool -g <interface_name>`.
    
```
    # ethtool -g enp1s0f0np0
    Ring parameters for enp1s0f0np0:
    Pre-set maximums:
    RX:             6144
    RX Mini:        0
    RX Jumbo:       0
    TX:             512
    Current hardware settings:
    RX:             2048         <--- 2KB, rx_ring_buffer_size.
    RX Mini:        0
    RX Jumbo:       0
    TX:             512
```

计算大页内存数量的公式:

```
    <num_rx_queues> * [ROUNDUP(rx_ring_buffer_size ÷ 1024) + 1].
    # 假设有16个网卡中断接收队列，网卡接收队列大小为2KB，
    16 * [ROUNDUP(2048 ÷ 1024) + 1] = 16 * 3 = 48 个 Huge Pages.
```

- 查看当前HugePage的大小

```
    # cat /proc/meminfo | grep Huge
    AnonHugePages:    2048 kB
    ShmemHugePages:        0 kB
    HugePages_Total:       0    <--- 当前系统能够提供的数量
    HugePages_Free:        0    <--- 当前系统剩余的量
    HugePages_Rsvd:        0
    HugePages_Surp:        0
    Hugepagesize:       2048 kB  <--- 每个HugePage的大小, 2MB.
    Hugetlb:               0 kB
```
    
如果发现HugePages_Free为0，说明所有的HugePage用光了，可以用下面命令申请

```
    # 临时命令, 开启1024个大内存页
    sysctl -w vm.nr_hugepages=1024
    # 永久命令
    echo "vm.nr_hugepages = 1024" >> /etc/sysctl.conf
```

使用`/proc/meminfo`监控大页的分配和使用情况.



#### 调优 - 中断亲和性(Interrupt Affinity)

onload会根据自己的法则，调整中断亲和性. 其它应用程序，需要自己进行调整，要保证接收相同数据的程序，共享L3缓存.


- 禁止`中断服务`(Disable the Irqbalance Service). Linux系统自带中断服务程序，X3522驱动程序会将中断均匀地分配到可用的主机CPU上。
  为了使用X3522驱动程序的默认亲和性设置（推荐），您必须确保在加载X3522驱动程序之前停止`irqbalance`服务，
  否则它会立即覆盖X3522驱动程序设置的亲和性配置值.
  
```
    # 查看状态
    /sbin/service irqbalance status
    
    # 关闭
    /sbin/service irqbalance stop
    
    # 永久关闭
    /sbin/chkconfig --level 12345 irqbalance off
```

一旦`irqbalance`服务被停止，就可以手动配置中断亲和性了. 确保特定的中断总是由特定的CPU核心处理，从而提高系统处理中断的效率.
    
- 查看系统给某个网卡分配了哪些中断 `ls /sys/class/net/<interface_name>/device/msi_irqs/`.

```
    # ls /sys/class/net/<interface_name>/device/msi_irqs/
    163  166  169  172  175  178  181  184  187  190  193  196  199  202  205  208
    164  167  170  173  176  179  182  185  188  191  194  197  200  203  206  209
    165  168  171  174  177  180  183  186  189  192  195  198  201  204  207  210
```

- 查看网卡的中断号，以及对应的队列. 第一列为中断号; 最后一列的命名规则 `<interface_name>_rx_<queue_num>`.

```
    $ cat /proc/interrupts
             CPU0    CPU1    CPU2    CPU3    CPU4    CPU5     CPU6    CPU7
    ...
     114:      44       0       0       0       1       0   125010       0   PCI-MSI 49283072-edge   enp1s0f0np0-rx-0
     116:  125010       0       0       0       1       0       44       0   PCI-MSI 49283073-edge   enp1s0f0np0-rx-1
     118:   58288      44       1       0       0       0    66723       0   PCI-MSI 49283074-edge   enp1s0f0np0-rx-2
     120:  125010       0       0       0       1       0        0      44   PCI-MSI 49283075-edge   enp1s0f0np0-rx-3
     122:       0       0       0       0       0       0        0       0   PCI-MSI 49283076-edge   enp1s0f0np0-rx-4
     124:       0       0       0       0       0       0        0       0   PCI-MSI 49283077-edge   enp1s0f0np0-rx-5
     126:       0       0       0       0       0       0        0       0   PCI-MSI 49283078-edge   enp1s0f0np0-rx-6
     128:       0       0       0       0       0       0        0       0   PCI-MSI 49283079-edge   enp1s0f0np0-rx-7
     130:      18       0       1       0       0       0       31       0   PCI-MSI 49283080-edge   enp1s0f0np0-tx-0
    ...

```

- 查看某个中断的亲和性(即在哪个CPU上), `cat /proc/irq/<n>/smp_affinity`. 

```
    # cat /proc/irq/164/smp_affinity
    01000000

```

- 调整亲和性

```
    echo <bitmask> > /proc/irq/<n>/smp_affinity
```


#### 调优 - 设置应用程序的CPU和RX queue

- 使用指定CPU

```
    taskset -c <cpu_id> <command line>
```

- 通过下面的命令，可以调整应用程序使用的`Receive queue`. 

`ethtool -N <interface_name> flow-type tcp4|udp4|tcp6|udp6 dst-ip <ip_addr> dst-port <port> queue <n>`

```
    # ethtool -N enp1s0f0np0 flow-type udp4 dst-ip 192.168.10.200 dst-port 1234 queue 3

    # 成功时，输出
    Added rule with ID 0

    # 错误时，输出
    rmgr: Cannot find appropriate slot to insert rule
    Cannot insert classification rule
```

- 查看网卡使用的rules. `ethtools -n <interface_name>`.

```
    # ethtool -n enp1s0f0np0
    
    8 RX rings available   <--- 网卡使用了8个RX队列
    Total 1 rules          <--- 共1个rule

    Filter: 0   <--- rule number: 0.
            Rule Type: UDP over IPv4
            Src IP addr: 0.0.0.0 mask: 255.255.255.255
            Dest IP addr: 192.168.10.200 mask: 0.0.0.0
            TOS: 0x0 mask: 0xff
            Src port: 0 mask: 0xffff
            Dest port: 1234 mask: 0x0
            Action: Direct to queue 3   <--- 队列 3.            
```

- 删除rules.

```
    ethtool -N <interface_name> delete <rule number>
```


#### 调优 - 丢掉不需要的multicast流量(组播反向过滤器)

组播地址范围: 224 ~ 239.

X3522的默认配置中，没有匹配的过滤器时，流量会转给主机操作系统处理，这在某些情况下可能导致不必要的CPU负载。
当没有匹配的过滤器时，流量会被导向队列0。X2522会丢掉流量.

- 添加Multicast反向匹配过滤器(Multicast Mismatch Filter). 

`ethtool -N <interface_name> flow-type udp4 dst-ip 224.0.0.0 m 15.255.255.255 action -1`.

所有经过<interface_name>的组播消息(239.X.X.X)，都会丢掉，而不是转给OS.

- 查看网卡使用的rules. 

```
    ethtools -n <interface_name>
```

- 删除rules.

```
    ethtool -N <interface_name> delete <rule number>
```

- 利用`ethtool -S <interface>`查看因反向过滤器(Mismatch Filter)，丢掉的包数量. `port_rxdp_di_dropped_pkts`字段.

例子，将组播流量(224.0.1.129:319/320)，通过queue 0，发给内核.

```
    $ sudo ethtool -N enp2s0f0np0 flow-type udp4 dst-ip 224.0.1.129 dst-port 319 action 0
    Added rule with ID 0

    $ sudo ethtool -N enp2s0f0np0 flow-type udp4 dst-ip 224.0.1.129 dst-port 320 action 0
    Added rule with ID 1
```

#### 调优 - 网卡中断聚合(Interrupt Coalescing)

中断聚合（Interrupt Coalescing）是一种硬件或软件技术，用于减少中断处理器的频率，通过合并多个小的中断请求为较少的几个大的中断请求。
这项技术主要用于提高系统效率，尤其是在处理大量小规模事件时。

中断聚合对NIC延迟至关重要，中断聚合通过合并多个事件到单个中断请求中，有助于减少CPU的中断处理次数，但同时也可能引入额外的延迟。

- 关掉中断自适应算法(adaptive algorithm)，可以：
    (1) 减少抖动;
    (2) 能够根据实际需要，调整中断聚合的时间间隔.

- 增加中断聚合时间间隔(Increasing the interrupt moderation interval)，导致:
    (1) 更少的中断
    (2) 增加延时
    (3) 提高峰值吞吐量
    
- 关闭中断聚合，导致:
    (1) 生成最多的中断
    (2) 最低的延时
    (3) 最大量的降低峰值吞吐量
    
对于低延时的场景（如：交易），应该禁用中断聚合，这样能达到最低的延时和抖动.

- 在调整网卡的中断聚合间隔之前，建议先`禁用自适应聚合`. 命令如下:

```
    # 禁用自适应聚合
    ethtool -C <interface_name> adaptive-rx off
    
    # 设置中断聚合时间间隔
    ethtool -C <interface_name> rx-usecs <interval>
    ethtool -C <interface_name> tx-usecs <interval>
    
    # 例子，将接收的中断聚合时间，调整为0.
    ethtool -C <interface_name> rx-usecs 0
    
    # 查看中断聚合
    # ethtool -c ens1f1d1
        Coalesce parameters for ens1f1d1:
        Adaptive RX: on  TX: off      <--- 中断 自适应聚合
        stats-block-usecs: 0
        sample-interval: 0
        pkt-rate-low: 0
        pkt-rate-high: 0

        rx-usecs: 0        <--- rx中断聚合时间间隔
        rx-frames: 0
        rx-usecs-irq: 0
        rx-frames-irq: 0

        tx-usecs: 0        <--- tx中断聚合时间间隔
        tx-frames: 0
        tx-usecs-irq: 0
        tx-frames-irq: 0

        rx-usecs-low: 0
        rx-frame-low: 0
        tx-usecs-low: 0
        tx-frame-low: 0

        rx-usecs-high: 0
        rx-frame-high: 0
        tx-usecs-high: 0
        tx-frame-high: 0
```

数据包的接收（RX）和发送（TX）共享中断，因此接收和发送的中断聚合间隔必须相等，
并且NIC驱动程序会自动调整发送使用的微秒数（tx-usecs）以匹配接收使用的微秒数（rx-usecs）.


#### 调优 - RX checksum offload

为了提高性能，关闭RX checksum（默认状态），命令:

```
    ethtool --offload <interface_name> [rx on|off]
```

如果关掉checksum，当有`EF_EVENT_TYPE_RX_REF_DISCARD`事件时，表示数据包的checksum不对. 
应用程序必须处理这些数据包，否则RX-ring会出现偏移错误，并最终耗尽空间.
这种处理就像处理一个普通的数据包一样（获取引用并释放它），以确认这个数据包，即使您不打算对其进行其他操作.

当应用程序接收到意外的或额外的数据包时，它仍然需要按照常规流程来处理这些数据包，
即获取数据包的引用并最终释放它，以确保数据包被正确地确认。
这样做可以防止接收缓冲区溢出或数据包丢失，即使应用程序并不打算对这些数据包进行进一步的处理.


#### 调优 - 网卡TCP/IP checksum

网卡会校验IP头，TCP/UDP头，但可以关掉.

X3的校验和，需要软件自己填充.

```
    ethtool -K <interface_name> rx <on|off>
    
    # 查看
    # ethtool -k ens1f1d1
        Features for ens1f1d1:
        rx-checksumming: off     <--- rx 校验
        tx-checksumming: off     <--- tx 校验
	        tx-checksum-ipv4: off [fixed]
	        tx-checksum-ip-generic: off [fixed]
	        tx-checksum-ipv6: off [fixed]
	        tx-checksum-fcoe-crc: off [fixed]
	        tx-checksum-sctp: off [fixed]
        scatter-gather: on
	        tx-scatter-gather: on [fixed]
	        tx-scatter-gather-fraglist: off [fixed]
        tcp-segmentation-offload: off
	        tx-tcp-segmentation: off [fixed]
	        tx-tcp-ecn-segmentation: off [fixed]
	        tx-tcp6-segmentation: off [fixed]
	        tx-tcp-mangleid-segmentation: off [fixed]
        udp-fragmentation-offload: off [fixed]
        generic-segmentation-offload: on
        generic-receive-offload: on
        large-receive-offload: off [fixed]
        rx-vlan-offload: off [fixed]
        tx-vlan-offload: off [fixed]
        ntuple-filters: off [fixed]
        receive-hashing: off [fixed]
        highdma: on [fixed]
        rx-vlan-filter: off [fixed]
        vlan-challenged: off [fixed]
        tx-lockless: off [fixed]
        netns-local: off [fixed]
        tx-gso-robust: off [fixed]
        tx-fcoe-segmentation: off [fixed]
        tx-gre-segmentation: off [fixed]
        tx-ipip-segmentation: off [fixed]
        tx-sit-segmentation: off [fixed]
        tx-udp_tnl-segmentation: off [fixed]
        fcoe-mtu: off [fixed]
        tx-nocache-copy: off
        loopback: off [fixed]
        rx-fcs: off [fixed]
        rx-all: off
        tx-vlan-stag-hw-insert: off [fixed]
        rx-vlan-stag-hw-parse: off [fixed]
        rx-vlan-stag-filter: off [fixed]
        busy-poll: off [fixed]
        tx-gre-csum-segmentation: off [fixed]
        tx-udp_tnl-csum-segmentation: off [fixed]
        tx-gso-partial: off [fixed]
        tx-sctp-segmentation: off [fixed]
        rx-gro-hw: off [fixed]
        l2-fwd-offload: off [fixed]
        hw-tc-offload: off [fixed]
        rx-udp_tunnel-port-offload: off [fixed]
```

#### 调优 - 调整TCP/UDP收发缓存

通常调整最大值(max)就可以了, 它定义了缓冲区可以增长到的最大值. 
链路延迟、丢包和CPU缓存大小等因素都会影响max的值.
可以直接修改`/etc/sysctl.conf`文件，也可以用命令修改.

```
    # 修改命令
    sysctl net.ipv4.tcp_rmem="<min> <default> <max>"
    sysctl net.ipv4.tcp_wmem="<min> <default> <max>"
    sysctl net.ipv4.udp_rmem="<min> <default> <max>"
    sysctl net.ipv4.udp_wmem="<min> <default> <max>"
```


#### 调优 - CPU

- `cpuspeed`, 多数Linux发行版，会运行`cpuspeed`服务. `cpuspeed`会根据当前系统负载，动态调整CPU时钟频率.
  动态调整CPU时钟频率，会降低CPU功耗，但会增加延时. 延迟敏感型应用，应该禁用`cpuspeed`.
  
```
    systemctl status cpuspeed
    # 关掉
    systemctl stop cpuspeed
    
    # 系统启动时就关掉
    /sbin/chkconfig -level 12345 cpuspeed off
```
  
- `cpupower`, 在RHEL7，使用`cpupower`替代`cpuspeed`.

```
    systemctl stop cpupower
    systemctl disable cpupower
    
    # 查看CPU状态
    cpupower monitor
```
  
- `tuned`, RHEL7系统，可以关掉`tuned`来降低延时.
  `tuned`是RHEL7中用于优化系统性能的工具，它可以根据不同的系统负载情况动态调整系统参数。但是，这些优化可能不适用于所有场景，特别是系统对延迟极其敏感时。

```
    systemctl stop tuned
    systemctl disable tuned
```

- CPU、内存与X3522 PCIe适配器之间的通信延迟可能取决于PCIe插槽。
有些插槽可能“更接近”CPU，因此具有更低的延迟和更高的吞吐量。如果可能，应将NIC安装在靠近所需NUMA节点的插槽中。

为了优化性能，建议将PCIe适配器安装在与其将要通信的NUMA节点相对应的本地插槽中。这样可以减少数据在系统总线上的传输距离，从而减少延迟并提高性能。


#### 调优 - 内存

许多芯片组使用`多通道`来访问主系统内存。只有当芯片组能够同时使用所有通道时，才能实现最大内存性能。
在选择服务器中要安装的内存模块（DIMM）数量时，应考虑这一点。为了在系统中获得最佳内存带宽，可能需要:
- 所有`DIMM`插槽都应被安装内存;
- 所有`NUMA`节点都应安装有内存;


#### 硬件监控

- `sensors`

```
    # 安装sensors
    yum install lm_sensors

    # sensors
        i350bb-pci-1700             <---  PCI适配器
        Adapter: PCI adapter
        loc1:         +41.0°C  (high = +120.0°C, crit = +110.0°C)

        power_meter-acpi-0          <--- ACPI（高级配置和电源接口）
        Adapter: ACPI interface
        power1:      281.00 W  (interval =   1.00 s)

        coretemp-isa-0000           <--- CPU核心
        Adapter: ISA adapter
        Package id 0:  +50.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 0:        +45.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 1:        +50.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 2:        +47.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 3:        +49.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 4:        +47.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 5:        +48.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 6:        +47.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 7:        +48.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 8:        +48.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 9:        +48.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 10:       +48.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 11:       +46.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 12:       +46.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 13:       +47.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 14:       +45.0°C  (high = +93.0°C, crit = +103.0°C)
        Core 15:       +43.0°C  (high = +93.0°C, crit = +103.0°C)
```


ethtool -S
===

`ethtool -S <interface_name>`

| Field  |  Description|
|:-------|-------------|
rx_drops          |	Total number of received packets dropped by the net driver. This includes the following counters: rx_mcast_mismatch、rx_alloc_skb_fail、rx_broadcast_drop、rx_other_host_drop
tx_drops          |	Total number of transmitted packets dropped by the net driver.
port_tx_pause (暂停帧) | Number of pause frames transmitted with valid pause op_code.
port_tx_unicast	      | Number of unicast packets transmitted. Includes flow control packets.
port_tx_multicast     |	Number of multicast packets transmitted.
port_tx_broadcast     |	Number of broadcast packets transmitted.
port_tx_lt64	      | Number of frames transmitted where the length is less than 64 bytes(小于64字节).
port_tx_64	          | Number of frames transmitted where the length is exactly 64 bytes(等于64字节).
port_tx_65_to_127	  | Number of frames transmitted where the length is between 65 and 127 bytes.
port_tx_128_to_255	  | Number of frames transmitted where the length is between 128 and 255 bytes.
port_tx_256_to_511	  | Number of frames transmitted where the length is between 256 and 511 bytes.
port_tx_512_to_1023	  | Number of frames transmitted where length is between 512 and 1023 bytes.
port_tx_1024_to_15xx  |	Number of frames transmitted where the length is between 1024 and 1518 bytes (1522 with VLAN tag).
port_tx_15xx_to_jumbo |	Number of frames transmitted where length is between 1518 bytes (1522 with VLAN tag) and 9000 bytes.
port_rx_good	      | Number of packets received with correct CRC value and no error codes.
port_rx_bad	          | Number of packets received with incorrect CRC value.(CRC错误)
port_rx_bad_bytes	  | Number of bytes with invalid FCS(FCS校验失败). Includes bytes from packets that exceed the maximum frame length.
port_rx_pause (暂停帧) |	Number of pause frames received with valid pause op_code.
port_rx_unicast	      | Number of unicast packets received.
port_rx_multicast	  | Number of multicast packets received.
port_rx_broadcast	  | Number of broadcasted packets received.
port_rx_lt64	      | Number of packets received where the length is less than 64 bytes(小于64字节).
port_rx_64	          | Number of packets received where the length is exactly 64 bytes(等于64字节).
port_rx_65_to_127     | Number of packets received where the length is between 65 and 127 bytes.
port_rx_128_to_255    | Number of packets received where the length is between 128 and 255 bytes.
port_rx_256_to_511    | Number of packets received where the length is between 256 and 511 bytes.
port_rx_512_to_1023   |	Number of packets received where the length is between 512 and 1023 bytes.
port_rx_1024_to_15xx  | Number of packets received where the length is between 1024 and 1518 bytes (1522 with VLAN tag).
port_rx_15xx_to_jumbo | Number of packets received where the length is between 1518 bytes (1522 with VLAN tag) and 9000 bytes.
port_rx_gtjumbo       | Number of packets received with a length is greater than 9000 bytes.
port_rx_bad_gtjumbo   | Number of packets received with a length greater than 9000 bytes, but with incorrect CRC value.
port_rx_align_error   | Number of packets received with an align error.
port_rx_length_error  | Number of packets received with a length error.
port_rx_nodesc_drops  | Number of packets dropped by the network adapter because of a lack of RX descriptors in the RX queue. <br/> Packets can be dropped by the NIC when there are insufficient(信息不足) RX descriptors in the RX queue to allocate to the packet. This problem occurs if the receive rate is very high and the network adapter receive cycle process has insufficient time between processing to refill the queue with new descriptors. <br/> A number of different steps can be tried to resolve this issue:<br/> (1)Disable the irqbalance daemon in the OS. <br/> (2)Distribute the traffic load across the available CPU/cores by setting rss_cpus=cores. Refer to Receive Side Scaling section. <br/> (3)Increase receive queue size using ethtool.
port_pm_discard_vfifo_full   | Number of packets dropped because of a lack of main packet memory on the adapter to receive the packet into.
port_rxdp_q_disabled_packets | Increments when the filter indicates the packet should be delivered to a specific RX queue which is currently disabled due to configuration error or error condition.
port_rxdp_di_dropped_packets | Number of packets dropped because the filters indicate the packet should be dropped. Can happen because:<br/> (1)the packet does not match any filter.<br/> (2)the matched filter indicates the packet should be dropped.
port_ctpio_underflow_fail | When the host fails to push packet bytes fast enough to match the adapter port speed. The packet is truncated and data transmitted as a poisoned packet.
port_ctpio_success        | Number of successful CTPIO TX events.
ptp_invalid_sync_windows  | Number of times that the PTP window (pre and post time sample) is unexpectedly large.
ptp_skipped_sync          | Number of times that PTP synchronization is skipped due to repeated invalid_sync_windows.
ptp_tx_dropped_timestamp  | Number of transmitted PTP packets dropped because of timestamp.
pps_fw                    | Number of PPS pulses received from firmware.
pps_in_count              | Number of external PPS pulses received.
pps_in_offset_mean        | Mean offset of the PPS pulse from the adapter clock, in nanoseconds.
pps_in_offset_last        | Last offset of the PPS pulse from the adapter clock, in nanoseconds.
pps_in_offset_max         | Maximum offset of the PPS pulse from the adapter clock, in nanoseconds.
pps_in_offset_min         | Minimum offset of the PPS pulse from the adapter clock, in nanoseconds.
pps_in_period_mean        | Mean period between successive PPS pulses, in nanoseconds.
pps_in_period_last        | Last period between successive PPS pulses, in nanoseconds.
pps_in_period_max         | Maximum period between successive PPS pulses, in nanoseconds.
pps_in_period_min         | Minimum period between successive PPS pulses, in nanoseconds.
pps_in_bad                | Number of bad PPS periods.
pps_in_oflow              | Number of PPS overflows.
tx_stop_queue         | Number of times the transmit queue has been stopped because space was not available.
evq_time_sync_events  | Number of time sync events received.
evq_error_events      | Number of error events received.
evq_flush_events      | Number of flush events received.
evq_unsol_overflow    | Number of times event queue has run out of unsolicited credits.
evq_unhandled_events  | Number of unhandled events received.
rx_ip_hdr_chksum_err  | Number of packets received with IP header checksum errors.
rx_tcp_udp_chksum_err | Number of packets received with TCP/UDP checksum errors.
rx_mcast_mismatch     | Number of unsolicited multicast packets received. Unwanted multicast packets can be received because a connected switch simply broadcasts all packets to all endpoints or because the connected switch is not able or not configured for IGMP snooping – a process from which it learns which endpoints are interested in which multicast streams.
rx_merge_events       | Number of RX completion events where more than one RX descriptor was completed.
rx_merge_packets      | Number of packets delivered to the host through merge events.
rx_alloc_skb_fail     | Number of times allocating a socket buffer to receive packets has failed.
rx_broadcast_drop     | Number of received broadcast packets that have been dropped.
rx_other_host_drop    | Number of received packets dropped because the MAC address does not match any MAC filter of the interface.
rx_nbl_empty          | Number of times an RX packet event is received but the network buffer list is empty.
rx_buffers_posted     | Number of super buffers that have been posted to receive packets.
rx_rollover_events    | Number of times that a rollover event has been received.
rx_aux_pkts           | Number of packets handled by the auxiliary device.
tx-n.tx_packets       | Per TX queue transmitted packets.
rx-n.rx_packets       | Per RX queue received packets.



X3-efvi切换手册
===

- X2使用的架构: Ef10.
- X3使用的架构: EfCt.


#### 官方数据

10G网络，官方数据, 1/2RTT latency, 32 byte message:

|          TYPE | latency ns |          Prog |
|--------------:|-----------:|--------------:|
|    Onload-UDP |       1095 | sfnt-pingpong |
|    Onload-TCP |       1110 | sfnt-pingpong |
| TcpDirect-UDP |        864 |               |
| TcpDirect-TCP |        870 |               |
|     ef_vi-UDP |        819 |     eflatency |
|    Kernel-UDP |       2750 | sfnt-pingpong |
|    Kernel-TCP |       3257 | sfnt-pingpong |


#### 实测数据

X2机器: Linux version 3.10.0-1160.el7.x86_64, Intel(R) Xeon(R) Gold 6256,  锁频3.6G. Onload 7.1.2.141, Solarflare Communications XtremeScale SFC9250.

X3机器: Linux version 3.10.0-1160.el7.x86_64, INTEL(R) XEON(R) GOLD 6544Y, 锁频4.1G. Onload 8.1.3.40, Xilinx Corporation Device 5084.

1/2RTT时间，单为ns.

|    TYPE |  64 | 128 |  256 |  512 | 1024 | 1460 |
|:--------|----:|----:|-----:|-----:|-----:|-----:|
| X2_DMA  | 1860| 1950| 2100 | 2265 | 2900 | 3350 |
| X2_CTPIO| 1250| 1290| 1450 | 1550 | 2100 | 2530 |
| X3_DMA  | 980 | 990 | 1190 | 1500 | 2000 | 2434 |
| X3_CTPIO| 940 | 960 | 1090 | 1350 | 1850 | 2300 |


#### 需要的驱动

- `auxiliary bus driver`
- `xilinx_efct`, X3 series network driver
- `libciul`，在Onload包中

在"src/tests/ef_vi/"目录，下列这些程序可以使用X3:
- `efsend`, 
- `eflatency`, CTPIO mode.
- `efsink`, multicast-mis, udp/tcp: filter options only


#### 一些基础变动

在X2卡上，DMA模式和CTPIO都可以正常工作。DMA模式对应的函数有`ef_vi_transmit()` / `ef_vi_transmit_init()` / `ef_vi_transmit_push()`等.

发送, X3卡只支持CTPIO模式. X3的efvi库，会把DMA转换成CTPIO模式. X3卡CTPIO模式，不会生成IP/TCP/UDP校验码, 需要自己算. 
       使用的发送函数为`ef_vi_transmitv_ctpio()`、`ef_vi_transmit_ctpio_fallback()`.
       就算校验码的函数为`ef_tcp_checksum()`、`ef_udp_checksum()`.

接收, X3支持的filter更少. 
    `ef_filter_spec_set_ip4_local()`, 
    `ef_filter_spec_set_eth_local()` / `ef_filter_spec_set_vlan()`, 
    `ef_filter_spec_set_multicast_mismatch()`.

程序必须处理 `EF_EVENT_TYPE_RX_REF` & `EF_EVENT_TYPE_RX_REF_DISCARD`.

X3收数据时，没有必要再refill RX ring.

X2的数据，只会被送入一个应用程序。


#### 共享和过滤流量（Sharing and Filtering Traffic）

X3卡(EFCT架构)，数据包很容易被多个程序接收. 

- `不共享(No Sharing)`

两个虚拟接口（Vis），每个都有一个针对不同组的过滤器，分配给不同的接收队列. 
正如Ef10一样，传入的流量是独立过滤的. 流量会被发送到一个应用程序或者另一个应用程序. 

如果应用程序被分配到同一个接收队列，那么它们都能看到彼此的流量. 

- `故意共享(Deliberate Sharing)`

两个虚拟接口（Vis），均使用相同的过滤器. 正如Ef10一样，符合过滤条件的组播流量，会被送入到2个应用程序. 
跟Ef10不同的是，这同样适用于非组播流量，也不再有优先级的延迟. 在Ef10中，第一个程序获取第一份拷贝，第二个程序获得第二份拷贝，差不多有10ns的延时.
在EFCT中，只有一份拷贝，同时对多个应用程序可见.

在EFCT中，可能多个应用程序共享队列，所以多个应用程序都会看到这个队列的所有流量. 应用程序需要使用软件进行过滤(must use software filtering to limit your view).

- `非故意共享(Non-deliberate Sharing)`

两个虚拟接口（Vis），使用不同的过滤器. 但第二个应用程序还插入了一个匹配第一个过滤器的过滤器. 
这样的话，第二个应用程序会看到第一个应用程序所有的流量，而不仅仅是匹配它们共享过滤器的流量.
这意味着第二个应用程序通过添加一个额外的过滤器，能够访问到第一个应用程序接收的全部流量，包括那些原本只应该由第一个应用程序接收的流量.

例如，第一个程序监听"192.0.2.123:1234"和"239.6.5.4:6789"，第二个程序监听"239.6.5.4:6789"，但是第二个程序也会看到"192.0.2.123:1234"的流量.

X3可以使用`ethtool --show-ntuple/--config-ntuple`来查看/修改过滤器.


[X4522](https://www.amd.com/en/support/downloads/solarflare-downloads.html/ethernet-adapters/solarflare/x4-series.html)
==

硬件规则尺寸
===

- PCIe : Gen5 ×8, half height half length
- 64-bit x86 processor
- 支持 Onload, TCPDirect, ef_vi
- 支持 RX checksum offload


硬件设别
===

系统能够识别网卡: `lspci -d 1924:`

```
01:00.0 Ethernet controller: AMD Solarflare Device 0c03
01:00.1 Ethernet controller: AMD Solarflare Device 0c03
```

ethtool 找到网卡: `ethtool -i <NIC_NAME>`

```
# ethtool -i <interface>
driver: sfc                          <--- 驱动名字
version: 4.18.0-372.9.1.el8.x86_64   <--- AMD网络驱动版本
firmware-version: 1.0.0.0 rx1 tx1
```

kernel相关的头文件，必须要存在 `ls /lib/modules/$(uname -r)/build`, 若不存在，需要安装 `yum install kernel-devel-$(uname -r)`.

若AMD网络驱动版本较低，需要先升级，获取更好的性能. [Solarflare Net v6 driver Source RPM](https://www.amd.com/en/support/downloads/solarflare-downloads.html/ethernet-adapters/solarflare/x4-series.html),下载最新驱动(XN-202496-LS).

```
# 编译
rpmbuild --rebuild <source_rpm_full_path>

# 更新, rpm包可以在上一步编译结果中看到
rpm -Uvh <path>/kernel-module-sfc-<os_version>-<module_version>.rpm

# 重新查看
ethtool -i <NIC_NAME>

driver: sfc
version: 6.2.0.1000   <--- 更新后的网络驱动版本
firmware-version: 1.0.7.3 rx1 tx1
expansion-rom-version: 
bus-info: 0000:8a:00.0

# 读取最新的网卡硬件信息
ip link

# 重新编译 initramfs
dracut -f
```

去掉旧版本onload
===

确认所有的onload程序都已经终止了, `onload_stackdump -z`.

```
# unload
onload_tool unload

# rm 
rpm -qa | grep onload  | xargs rpm -e
rpm -qa | grep sfc     | xargs rpm -e
rpm -qa | grep sfutils | xargs rpm -e

# 卸载
onload_uninstall

```

确认相关内核程序已卸载

```
# 查看
lsmod | grep onload
lsmod | grep sfc

# 卸载
rmmod onload
rmmod sfc_char

find /lib/modules/$(uname -r) -name 'sfc*.ko' | xargs rm –rf
rmmod sfc

update-initramfs -u -k <kernel version>

# 
dracut –f

```

安装新版本onload
===

先安装 [Solarflare Net v6 driver Source RPM](https://www.amd.com/en/support/downloads/solarflare-downloads.html/ethernet-adapters/solarflare/x4-series.html)

再安装onload

```
onload_install

dracut –f

onload_tool unload
modprobe sfc
onload_tool reload

```


#### X4 Server Requirements

64-bits x86 processor.
PCIe Gen5 x 8 board.
packets: kernel-headers; kernel-devel, or kernel-smp-devel (for Symmetric Multi-Processing systems).


[~]# lspci -vv | grep net
84:00.0 Ethernet controller: Solarflare Communications Device 0c03
                Product Name: AMD Solarflare X4522-PLUS Ethernet Adapter
84:00.1 Ethernet controller: Solarflare Communications Device 0c03
                Product Name: AMD Solarflare X4522-PLUS Ethernet Adapter
                
[~]# lspci -d 1924:
84:00.0 Ethernet controller: Solarflare Communications Device 0c03
84:00.1 Ethernet controller: Solarflare Communications Device 0c03

`1924:`是 Solarflare Communications 在 PCI 设备列表里的厂商 ID（Vendor ID）。

如何鉴别驱动是否是新的（非linux自带的驱动）

```
# Linux自带驱动显示如下

# ethtool -i <interface>
    driver: sfc
    version: 4.18.0-372.9.1.el8.x86_64
    firmware-version: 1.0.0.0 rx1 tx1


# ethtool -i <interface>
    version: 4.0
    firmware-version: 1.0.0.0

```



重新加载Network Driver，
`modprobe –r sfc`
`modprobe sfc`

确认网卡驱动被加载
`ip link`

查看网卡驱动版本
`ethtool -i <interface>`

重新编译`initramfs`:
确认新驱动已加载后，再执行`dracut -f`，把新驱动打包进启动镜像，
防止重启时系统又拉回旧`in-tree`驱动

`Enterprise datapath`
继承 X2/8000/7000 的全功能企业级特性（多队列、卸载、统计、ACL…），只是资源更多、性能更强。

`Express datapath`
从X3的“超低延迟”血统进化而来，把功能裁到最少、路径最短，换的是纳秒级延迟。


一张 X4 卡在系统里会暴露出 两组网络接口（通常叫 ensXf0/ensXf1 和 ensXf0-expr/ensXf1-expr，或类似命名）。
应用启动时 把 socket 绑定到其中一组接口即可：
需要功能全开（VXLAN、TC flower、大队列…）→ 绑定 Enterprise 接口。
需要极限低延迟（Onload/TCPDirect kernel-bypass）→ 绑定 Express 接口。
同一进程的不同 socket 甚至可以分别绑定 Enterprise 和 Express；不同进程也各选各的，互不干扰。
所以“each application can use one or both” 强调：
不是“混合跑在同一条流”，而是“应用按需挑接口，两套硬件通路任你选，甚至能同时开两 socket 各走各路”。




测试准备
===

- 创建两个网络命名空间(ns0/ns1)

```
sudo ip netns add ns0
sudo ip netns add ns1
# 查看结果
sudo ip netns show
# 预期结果
# ns1
# ns0
```

- 将两个网卡移动到命名空间

```
sudo ip link set enp94s0f0 netns ns0
sudo ip link set enp94s0f1 netns ns1
# 查看原命名空间
sudo ip link show
# 预期结果
# enp94s0f0 & enp94s0f1 不在原来的命名空间
# 查看ns0/ns1命名空间内的设备
sudo ip netns exec ns0 ip link show enp94s0f0
sudo ip netns exec ns1 ip link show enp94s0f1
# 预期结果，设备均down
# 2: enp94s0f0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
#     link/ether 00:0f:53:a3:11:90 brd ff:ff:ff:ff:ff:ff
# 3: enp94s0f1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
#     link/ether 00:0f:53:a3:11:91 brd ff:ff:ff:ff:ff:ff
```

- 为设备设置IP
```
sudo ip netns exec ns0 ip addr add 172.20.1.100/24 dev enp94s0f0
sudo ip netns exec ns1 ip addr add 172.20.1.101/24 dev enp94s0f1
# 验证结果，能看到IP
sudo ip netns exec ns0 ip addr show enp94s0f0
sudo ip netns exec ns1 ip addr show enp94s0f1
# 2: enp94s0f0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
#     link/ether 00:0f:53:a3:11:90 brd ff:ff:ff:ff:ff:ff
#     inet 172.20.1.100/24 scope global enp94s0f0
#        valid_lft forever preferred_lft forever
# 3: enp94s0f1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
#     link/ether 00:0f:53:a3:11:91 brd ff:ff:ff:ff:ff:ff
#     inet 172.20.1.101/24 scope global enp94s0f1
#        valid_lft forever preferred_lft forever
```

- 启用设备

```
sudo ip netns exec ns0 ip link set dev enp94s0f0 up
sudo ip netns exec ns1 ip link set dev enp94s0f1 up
# 验证结果，能看设备state为UP
sudo ip netns exec ns0 ip addr show enp94s0f0
sudo ip netns exec ns1 ip addr show enp94s0f1
# 2: enp94s0f0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
#     link/ether 00:0f:53:a3:11:90 brd ff:ff:ff:ff:ff:ff
#     inet 172.20.1.100/24 scope global enp94s0f0
#        valid_lft forever preferred_lft forever
#     inet6 fe80::20f:53ff:fea3:1190/64 scope link 
#        valid_lft forever preferred_lft forever
# 3: enp94s0f1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
#     link/ether 00:0f:53:a3:11:91 brd ff:ff:ff:ff:ff:ff
#     inet 172.20.1.101/24 scope global enp94s0f1
#        valid_lft forever preferred_lft forever
#     inet6 fe80::20f:53ff:fea3:1191/64 scope link 
#        valid_lft forever preferred_lft forever
```

- 跨网络命名空间通信

```
sudo ip netns exec ns0 ping 172.20.1.101
sudo ip netns exec ns0 tcpdump -nn -i enp94s0f0
```

- del网络命名空间

```
# 删除ns0/ns1
sudo ip netns del ns0
sudo ip netns del ns1
```



Xilinx官方测量工具
===

#### sfnt-pingpong & sfnt-stream

[源码](https://github.com/Xilinx-CNS/cns-sfnettest), `cd sfnettest-ver/src & make`，编译生成"sfnt-pingpong、sfnt-stream"两个可执行程序.

- sfnt-pingpong, `sfnt-pingpong [options] [tcp|udp|pipe|unix_stream|unix_datagram [host[:port]]]`

```
# 服务端
onload -v -p latency-best ./sfnt-pingpong --affinity="5;8"

# client
onload -v -p latency-best ./sfnt-pingpong --affinity="5;8" udp 172.20.1.100
```

#### eflatency

[源码](https://github.com/Xilinx-CNS/onload), `cd onload-src/scripts & onload_build`, 编译完毕，进入"build/gnu_x86_64/tests/ef_vi"目录.

- eflatency, `eflatency [options] <ping|pong> <interface> [<tx_interface>]`

```
# 服务端
taskset -c 5 ./eflatency -s 64:1500 pong enp94s0f0

# client
taskset -c 8 ./eflatency -s 64:1500 ping enp94s0f1
```
