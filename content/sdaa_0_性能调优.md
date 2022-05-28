
### CPU

`numa`架构，CPU访问近端内存比访问远端内存要快得多。

`mpstat`, Multiprocessor Statistics, (`yum install sysstat`)可以统计每个CPU核的信息。<br/>

```
Linux 3.10.0-693.21.1.el7.x86_64 (53-170)       05/28/2022      _x86_64_        (20 CPU)

10:05:52 AM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
10:05:54 AM  all   27.69    0.00    1.75    0.00    0.00    0.10    0.00    0.00    0.00   70.46
10:05:54 AM    0    0.50    0.00    0.50    0.00    0.00    0.00    0.00    0.00    0.00   99.00
10:05:54 AM    1    3.63    0.00    5.18    0.00    0.00    0.00    0.00    0.00    0.00   91.19
10:05:54 AM    2    5.64    0.00    5.64    0.00    0.00    0.00    0.00    0.00    0.00   88.72
10:05:54 AM    3    3.63    0.00    4.66    0.00    0.00    1.04    0.00    0.00    0.00   90.67
10:05:54 AM    4  100.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00
10:05:54 AM    5    5.15    0.00    5.67    0.00    0.00    0.00    0.00    0.00    0.00   89.18
10:05:54 AM    6    4.17    0.00    4.69    0.00    0.00    0.00    0.00    0.00    0.00   91.15
10:05:54 AM    7  100.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00
10:05:54 AM    8    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
10:05:54 AM    9  100.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00
10:05:54 AM   10   99.50    0.00    0.00    0.00    0.00    0.50    0.00    0.00    0.00    0.00
10:05:54 AM   11    2.66    0.00    1.60    0.00    0.00    0.00    0.00    0.00    0.00   95.74
10:05:54 AM   12    4.21    0.00    2.11    0.00    0.00    0.00    0.00    0.00    0.00   93.68
10:05:54 AM   13    4.76    0.00    1.59    0.00    0.00    0.00    0.00    0.00    0.00   93.65
10:05:54 AM   14    0.53    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.47
10:05:54 AM   15    0.53    0.00    1.06    0.00    0.00    0.53    0.00    0.00    0.00   97.87
10:05:54 AM   16    5.85    0.00    1.60    0.00    0.00    0.00    0.00    0.00    0.00   92.55
10:05:54 AM   17    0.00    0.00    1.51    0.00    0.00    0.00    0.00    0.00    0.00   98.49
10:05:54 AM   18   99.50    0.00    0.00    0.00    0.00    0.50    0.00    0.00    0.00    0.00
10:05:54 AM   19    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
```
iowait, 磁盘IO等待时间 <br/>
irq, 硬中断时间 <br/>
soft,软中断时间 <br/>



超线程

### 内存

### 网络
#### 网卡

(1)查看网卡数量及类型 `lspci -vvv | grep "Ethernet controller"`.

```
[root@ ~]# lspci -vvv | grep "Ethernet controller"

19:00.0 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)
19:00.1 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)
3d:00.0 Ethernet controller: Intel Corporation Ethernet Connection X722 for 1GbE (rev 09)
3d:00.1 Ethernet controller: Intel Corporation Ethernet Connection X722 for 1GbE (rev 09)
3d:00.2 Ethernet controller: Intel Corporation Ethernet Connection X722 for 1GbE (rev 09)
3d:00.3 Ethernet controller: Intel Corporation Ethernet Connection X722 for 1GbE (rev 09)
5e:00.0 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 02)
5e:00.1 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 02)
86:00.0 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)
86:00.1 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)
```sh

一共4张卡，10个口: <br/>
    "Intel Corporation Ethernet Connection X722"，是板载网卡，千兆卡，4个口；<br/>
    "Intel Corporation Ethernet Controller X710"，普通万兆网卡，2个口；<br/>
    "Solarflare Communications XtremeScale SFC9250"，SF万兆卡，两张，每张2个口. <br/>


(2)查看各网卡的状态、IP、MAC. `ifconfig -a`.

```
[root@ ~]# ifconfig -a

// bond卡
bond0: flags=5187<UP,BROADCAST,RUNNING,MASTER,MULTICAST>  mtu 1500
        inet 10.xx.xx.xx  netmask 255.255.252.0  broadcast 10.xx.xx.255
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 134709799  bytes 18966348076 (17.6 GiB)
        RX errors 0  dropped 3561158  overruns 0  frame 0
        TX packets 8769699  bytes 6858567325 (6.3 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0


// 第一组：下面4个板载千兆口，都没有插网线，没有“RUNNING”状态
enp11s0f0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp11s0f1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp11s0f2: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp11s0f3: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0


// 第二组：
ens1f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 112  bytes 11253 (10.9 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 294  bytes 23439 (22.8 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

ens1f1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 20.25.25.21  netmask 255.255.240.0  broadcast 20.xx.xx.255
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 10419120807  bytes 6282078698041 (5.7 TiB)
        RX errors 0  dropped 5304  overruns 0  frame 0
        TX packets 6989216  bytes 539782557 (514.7 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0


// 第三组：（镜像）
ens3f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 631213609  bytes 547611383407 (510.0 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 135

ens3f1: flags=6211<UP,BROADCAST,RUNNING,SLAVE,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx txqueuelen 1000  (Ethernet)
        RX packets 78726019  bytes 13001095973 (12.1 GiB)
        RX errors 0  dropped 85  overruns 0  frame 0
        TX packets 8772419  bytes 6859025633 (6.3 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 161


// 第四组：（镜像）
ens4f0: flags=6211<UP,BROADCAST,RUNNING,SLAVE,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 56017072  bytes 5968559012 (5.5 GiB)
        RX errors 0  dropped 107  overruns 0  frame 0
        TX packets 201  bytes 24085 (23.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 187

ens4f1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether xx:xx:xx:xx:xx:xx  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 213


// 本地环回
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 560  bytes 105234 (102.7 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 560  bytes 105234 (102.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

(3)网卡与OS交互方式: `中断IRQ`和`DMA`. DMA的传送工作过程：
* DMAC发出DMA 传送请求；
* DMAC通过连接到CPU的HOLD信号向CPU提出DMA请求；
* CPU在完成当前总线操作后会立即对DMA请求做出响应，CPU 的响应包括两个方面： CPU 将控制总线、数据总线和地址总线浮空，即放弃对这些总线的控制权；CPU 将有效的HLDA 信号加到DMAC 上，以通知DMAC CPU 已经放弃了总线的控制权；
* CPU 将总线浮空，即放弃了总线控制权后，由DMAC 接管系统总线的控制权，并向外设送出DMA 的应答信号。
* DMAC 送出地址信号和控制信号，实现外设与内存或内存之间大量数据的快速传送。
* DMAC 将规定的数据字节传送完之后，通过向CPU 发HOLD 信号，撤消对CPU的DMA 请求。CPU 收到此信号，一方面使HLDA 无效，另一方面又重新开始控制总线，实现正常取指令、分析指令、执行指令的操作。

`cat /proc/interrupts`，可以看到中断在各CPU上的分布。


### 中断