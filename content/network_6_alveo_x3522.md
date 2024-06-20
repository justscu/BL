
[Alveo X3522](https://china.xilinx.com/products/boards-and-kits/alveo/x3.html#overview)
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


X3522安装手册
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
ip addr add 192.168.1.100/24 dev eth0
# 激活端口
ip link set eth0 up
# 网关
ip route add default via 192.168.1.1
```


X3522用户手册
===

在200bytes，1/2 RTT, X3522比X2522快230ns. PCIe4比PCIe3快80ns

driver管理`RX queues`，并在多个应用之间共享, 共享queue的应用程序使用同样的L3.

TX只支持CTPIO.

X3522使用的驱动为`xilinx_efct`，又onload提供. 
X3522使用`auxiliary bus driver`作为驱动，由AMD提供.
使用`onload`，必须使用`xilinx_efct`和`auxiliary bus driver`.

`TCPDirect`现在提供在它自己的软件包中，与`Onload`分开供应.


#### Onload 迁移

无需修改应用程序.
在不共享CTPIO apertures的情况下，最多可以创建15个Onload栈.

onload utilities(工具)仅仅与onload package(包)交互，`onload_tool`不会重新加载`xilinx_efct`驱动程序.


#### TCPDirect 迁移

不支持`TX-alternatives `、`Overlapped receives`.

确保没有打开`PIO`.

使用动态库的应用程序，没有上述问题，可以不用重新编译.
使用静态库的应用程序，需要重新编译.


#### ef_vi 迁移

使用新的事件(events)来接收数据，使用新的函数(call)来发送数据.

驱动程序(driver)管理RX缓存, 不需要再调用post*函数来处理数据包描述符, 但需要在收到数据包后释放.

TX 仅仅支持CTPIO.

必须过滤不需要的数据，包括非TCP/UDP帧(frame).

一些示例程序不支持X3522.


#### sfptpd 迁移

X3522需要一个更新版本的sfptpd，以识别其新的PCIe供应商ID.

它将与其他AMD/Solarflare NIC（网络接口卡）以相同的方式被处理, 不需要对配置进行任何更改.


#### 队列处理

如何处理队列(queue handling)，是X3522跟之前的网卡最大的区别.

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


#### RX checksum offload

为了提高性能，关闭RX checksum（默认状态），命令:

```
    ethtool --offload <interface> [rx on|off]
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

