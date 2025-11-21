`网络适配器`(网卡, NIC, Network Interface Card), 工作在物理层和数据链路层。主要由PHY/MAC芯片、Tx/Rx FIFO、DMA等组成.

网线通过变压器接PHY芯片、PHY芯片通过MII总线接MAC芯片、MAC芯片接PCI总线.

- 物理层(PHY芯片): CSMA/CD、数模转换、编解码、串并转换.
- 数据链路层: 包含MAC(介质访问控制子层)和 LLC(逻辑链路控制子层), bit流和数据帧的转换、CRC校验、包过滤(L2 Filtering、VLAN Filtering、Manageability/Host Filtering).
- 网卡通过[中断](https://github.com/justscu/BL/blob/master/content/CSAPP-8-异常控制流.md)收发数据包.

#### 网卡基本信息

`lspci | grep -i Ethernet`, 找到"Ethernet controller", 可见网卡信息和设备商信息

```
18:00.0 Ethernet controller: Broadcom Inc. and subsidiaries NetXtreme BCM5720 2-port Gigabit Ethernet PCIe   <--- 4个板载口
18:00.1 Ethernet controller: Broadcom Inc. and subsidiaries NetXtreme BCM5720 2-port Gigabit Ethernet PCIe
19:00.0 Ethernet controller: Broadcom Inc. and subsidiaries NetXtreme BCM5720 2-port Gigabit Ethernet PCIe
19:00.1 Ethernet controller: Broadcom Inc. and subsidiaries NetXtreme BCM5720 2-port Gigabit Ethernet PCIe
5e:00.0 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)  <--- 2个SF口
5e:00.1 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)

```

`lspci -vvv`, 查看网卡设备信息

`ethtool -i eno1`, 网卡驱动版本信息

```
driver: sfc                              <--- SF卡
version: 4.15.12.1008
firmware-version: 8.0.0.1015 rx1 tx1     <--- 固件版本信息
expansion-rom-version: 
bus-info: 0000:5e:00.0
supports-statistics: yes
supports-test: yes
supports-eeprom-access: no
supports-register-dump: yes
supports-priv-flags: yes

```

`ethtool eno1`, 网卡工作模式

```sh

# ethtool eno1

Settings for p2p2:
	Supported ports: [ FIBRE ]   <--- TP，双绞线; FIBRE, 光纤
	Supported link modes:   1000baseT/Full   <---  1G
	                        10000baseT/Full  <--- 10G
	                        25000baseCR/Full <--- 25G
	Supported pause frame use: Symmetric Receive-only
	Supports auto-negotiation: Yes
	Supported FEC modes: None BaseR RS
	Advertised link modes:  1000baseT/Full 
	                        10000baseT/Full 
	                        25000baseCR/Full 
	Advertised pause frame use: Symmetric
	Advertised auto-negotiation: Yes  <--- 是否自动协商
	Advertised FEC modes: None BaseR RS
	Link partner advertised link modes:  Not reported
	Link partner advertised pause frame use: No
	Link partner advertised auto-negotiation: No
	Link partner advertised FEC modes: None
	Speed: 10000Mb/s <--- 网卡速度 10G
	Duplex: Full     <--- 工作模式(Full, 全双工; Half, 半双工)
	Port: FIBRE      <--- 光口
	PHYAD: 255
	Transceiver: internal
	Auto-negotiation: on
	Supports Wake-on: d
	Wake-on: d
	Current message level: 0x000020f7 (8439)
			       drv probe link ifdown ifup rx_err tx_err hw
	Link detected: yes

```

#### 网卡统计信息

`ifconfig eno1`, 统计收发包信息

```sh
[ll@ll]$ ifconfig eno1
eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500   <----- MULTICAST 支持组播
        inet 10.25.26.219  netmask 255.0.0.0  broadcast 10.255.255.255
        inet6 fe80::4a0f:cfff:fe32:7277  prefixlen 64  scopeid 0x20<link>
        ether 48:0f:cf:32:72:77  txqueuelen 1000  (Ethernet)
        RX packets 79300274  bytes 33409979936 (31.1 GiB)  <----- 收(RX)
        RX errors 0  dropped 61  overruns 0  frame 0
        TX packets 6110486  bytes 2978220458 (2.7 GiB)     <----- 发(TX)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 16  memory 0xe1000000-e1020000  
```
- `RX errors`: 总的收包的错误量。包括too-long-frames 错误、Ring Buffer 溢出错误、CRC 校验错误、帧同步错误、 fifo overruns 以及 missed pkg 等
- `RX dropped`: 数据包已经进入Ring Buffer，但由于内存不够等系统原因，导致从网卡拷贝到内存过程中被丢弃
- `RX overruns`: 数据到达网卡的RingBuffer，但CPU处理中断不及时，导致RingBuffer满了，于是网卡丢掉包。RingBuffer的传输IO大于kernel能处理的IO导致的（RingBuffer指发起IRQ请求前的那块buffer）。也有可能是中断分布不均匀导致的(如所有中断都在core0上做)。<br/>


`ethtool -S eno1`, 统计收发包信息

```sh

# ethtool -S eno1

NIC statistics:
     rx_packets: 79291446
     tx_packets: 6110399
     rx_bytes: 33409389770
     tx_bytes: 2978213932
     rx_broadcast: 15564847
     tx_broadcast: 1883
     rx_multicast: 57822435
     tx_multicast: 3126805
     rx_errors: 0
     tx_errors: 0
     tx_dropped: 0
     multicast: 57822435
     collisions: 0
     rx_length_errors: 0
     rx_over_errors: 0 
     rx_crc_errors: 0       <----- CRC 校验，是否有物理层干扰
     rx_frame_errors: 0
     rx_no_buffer_count: 0  <----- Ring Buffer不够，调用"ethtool -G eno1 rx XX"来设置
     rx_missed_errors: 0
     tx_aborted_errors: 0
     tx_carrier_errors: 0
     tx_fifo_errors: 0
     tx_heartbeat_errors: 0
     tx_window_errors: 0
        ... ...
```

#### 网卡多队列

`网卡多队列`，指网卡内部维护多个收发队列，并产生多个中断信号，使用多个CPU来处理网卡收到的包，来提升网络处理性能.

- `cat /proc/interrupts`，查看中断号
- `cat /proc/irq/中断号/smp_affinity`，中断分配到哪个CPU上，可以调整


把一块网卡的`收/发`包任务，拆成多条独立通道，每条通道叫做一个`RX/TX Queue`，对应一个`CPU core`，实现`并行处理`,
解决单核打满与缓存乒乓问题.

- 数据包处理流程, "网卡收到帧 -> 写内存(Rx-Ring) -> 触发中断 -> 内核软中断 -> 协议栈 -> 应用";
- 如果只有一个Rx-Queue，则所有的中断，都会落到同一个CPU, 导致很容易把CPU打满，丢包;
- 多队列，网卡把不同报文，`散列`到不同队列，每个队列再中断到不同核，实现`并行收包`.

硬件层面散列(`RSS`), Receive-Side Scaling（RSS）是网卡硬件功能

- 根据五元组（sip + dip + sport + dport + proto）算 hash -> 取模 -> 决定放进哪条队列;
- 队列数 ≤ 核数时，一条队列固定中断到一个核，天然绑核；
- 队列数 > 核数时，多条队列可以软件绑到同一核（irqbalance / echo 8 > smp_affinity）.

软件层面的`绑核`, RPS/RFS/XPS

<div align="center">

| 机制 | 作用 | 与队列关系|
|-----|------|---------|
|RPS  |把软中断分到多核 | 单队列网卡也能用，软件模拟`多队列`
|RFS  |把同一个流固定到运行应用的核 | 减少缓存迁移
|XPS  |发方向，把发包软中断绑到队列所在核 | 避免跨核竞争 TX ring

</div>

收数据的流程

- 网卡DMA帧到队列N的内存;
- 产生`MSI-X`中断，中断表项里写着"CPU bitmap = 1<<k";
- `CPU-k`响应中断，在软中断上下文里收帧、走协议栈;
- 如果应用线程也绑在`CPU-k`，则数据、元数据、应用上下文都在同一条 L1/L2 缓存，性能最佳.


网卡多队列 = 把"网络处理"拆成多条流水线，每条流水线固定到一颗 CPU 核，核越多、队列越多，就能并行处理越高的 PPS/BPS;
队列和核之间通过中断表、RSS hash、irq affinity 实现"硬绑定"，再通过 RPS/RFS 做“软微调”，
最终目的都是让报文从网卡 DMA 到应用只在一个核的缓存里完成，避免跨核跳跃和锁竞争.


查看网卡`中断队列(IRQ channels)`的分配情况，即网卡的`多队列(RSS，Receive Side Scaling)` 配置
```
[root@~]ethtool -l enp59s0f1

Channel parameters for enp59s0f1:

Pre-set maximums: <-- 硬件支持最大值
RX:        n/a      <-- 没有单独设置接收队列Rx-queue，通过Combined统一管理
TX:        n/a      <-- 没有单独设置接收队列Tx-queue，通过Combined统一管理
Other:       1      <-- 有一个其它用途的队列
Combined:   63      <-- 网卡最多支持63个组合队列(Rx+Tx合并)，即最多支持63个CPU核心同时处理网络中断

Current hardware settings: <-- 当前使用值
RX:        n/a
TX:        n/a
Other:       1
Combined:   20  <-- 网卡使用了20个CPU核心来处理网络中断
```

可以使用`ethtool -S enp59s0f1`，来查看每个通道上的中断情况.


#### 通过指定 Rx-queue & cpu core 收组播

<font color=red> 让组播"230.2.3.4:5566"数据，通过"enp94s0f1"网卡的"Rx-Queue 5"接收，并绑定到"cpu 8"上 </font>

1. 检查"enp94s0f1"在哪个`numa`节点，不要绑错了numa节点. 若跨了numa节点，会导致20%~30%的性能损失.

```
[~]$ lspci -vv -s $(ethtool -i enp94s0f1 | grep bus-info | awk '{print $2}')

5e:00.1 Ethernet controller: Solarflare Communications XtremeScale SFC9250 10/25/40/50/100G Ethernet Controller (rev 01)
	Subsystem: Solarflare Communications XtremeScale X2522-25G Network Adapter
	Control: I/O- Mem+ BusMaster+ SpecCycle- MemWINV- VGASnoop- ParErr- Stepping- SERR- FastB2B- DisINTx+
	Status: Cap+ 66MHz- UDF- FastB2B- ParErr- DEVSEL=fast >TAbort- <TAbort- <MAbort- >SERR- <PERR- INTx-
	Latency: 0
	Interrupt: pin B routed to IRQ 62
	
	NUMA node: 0           <-------- NUMA node 0.
	
	Region 0: Memory at b8800000 (64-bit, non-prefetchable) [size=8M]
	Region 2: Memory at b9800000 (64-bit, non-prefetchable) [size=16K]
	Expansion ROM at b9880000 [disabled] [size=256K]
	Capabilities: <access denied>
	Kernel driver in use: sfc
	Kernel modules: sfc

```

再通过`lscpu`， 查看对应的numa上，有哪些CPU core可用.

```
    NUMA:
      NUMA node(s):         2
      NUMA node0 CPU(s):    0,2,4,6,8,10,12,14,16,18,20,22
      NUMA node1 CPU(s):    1,3,5,7,9,11,13,15,17,19,21,23
```


3. 确认网卡具有把组播地址，hash到"Queue 5"的能力.

```
[~]$ ethtool -k enp94s0f1 | grep -E "ntuple|rfs"
    
    ntuple-filters: on   <--- 显示on，则表示支持

```

4. 写一条`ntuple`规则

```
    ethtool -N enp94s0f1 flow-type udp4 dst-ip 230.2.3.4 dst-port 5566 action 5
    
    # action 5 : 表示硬件把匹配报文推送到queue 5.
```

查看规则是否生效: `ethtool -n enp94s0f1`

```
[~]$ ethtool -n enp94s0f1
12 RX rings available   <-- 共12条规则可用
Total 1 rules

Filter: 0     <-- 这就是 loc

	Rule Type: UDP over IPv4
	Src IP addr: 0.0.0.0 mask: 255.255.255.255
	Dest IP addr: 230.2.3.4 mask: 0.0.0.0
	TOS: 0x0 mask: 0xff
	Src port: 0 mask: 0xffff
	Dest port: 5566 mask: 0x0
	Action: Direct to queue 5

```

删除这条规则 `ethtool -N enp94s0f1 delete 0` (loc is 0).


5. 验证"Rx-Queue 5"生效

收组播数据时，看看"rx-5"队列是否在上涨, `ethtool -S enp94s0f1 | grep "rx-5.rx_packets"`.

```
    watch -n 1 'ethtool -S enp94s0f1 | grep "rx-5.rx_packets"'
```

6. 找到"Queue 5"对应的中断号, `cat /proc/interrupts | grep enp94s0f1-5`.

```
  97:          0          0   ...   PCI-MSI 49285127-edge      enp94s0f1-5
  
  # "97"就是"Queue-5"的中断号
```

7. 绑定到"CPU 8"

"CPU 8"的二进制是"1 0000 0000"，16进制掩码是"0x100". (CPU编号从`0`开始，bitmap的bit号也要从`0`对齐)

```
        # 把中断97，绑定到7号CPU
    echo 8 > /proc/irq/97/smp_affinity_list
        # 或者
    echo 0x100 > /proc/irq/97/smp_affinity
    
```

8. 针对网卡enp94s0f1, 彻底关闭内核的`RPS`(Receive Packet Steering)功能，确保"软中断"也留在CPU 8.

```
    # 把 队列 5 的 RPS 目标 CPU 掩码清成 0 -> 内核不会再把报文软中断分发到任何其他核
    echo 0 > /sys/class/net/enp94s0f1/queues/rx-5/rps_cpus
    
    # 把 RPS 流表项数 设成 0 -> 内核不再为这条队列维护“哪个流该去哪个 CPU”的哈希表，节省内存也避免意外迁移
    echo 0 > /sys/class/net/enp94s0f1/queues/rx-5/rps_flow_cnt
```

`RPS`被关掉后，`软中断`跟随`硬中断`，谁收包谁处理.



#### 网卡发包过程

`PCIe`(Peripheral Component Interconnect express), 高速串行计算机扩展总线标准, 数据传输速率高, 可达10GB/s

- The host creates a descriptor ring and configures one of the 82599's transmit queues with the address location, length, head, and tail pointers of the ring (one of 128 available Tx queues).
- The host is requested by the TCP/IP stack to transmit a packet, it gets the packet data within one or more data buffers.
- The host initializes the descriptor(s) that point to the data buffer(s) and have additional control parameters that describes the needed hardware functionality. The host places that descriptor in the correct location at the appropriate Tx ring.
- The host updates the appropriate Queue Tail Pointer(`TDT`).
- The 82599's DMA senses a change of a specific TDT and as a result sends a PCIe requests to fetch the descriptor(s) from host memory.
- The descriptor(s) content is received in a PCIe read completion and is written to the appropriate location in the descriptor queue.
- The DMA fetches the next descriptor and processes its content. As a result, the DMA sends PCIe requests to fetch the packet data from system memory.
- The packet data is being received from PCIe completions and passes through the transmit DMA that performs all programmed data manipulations (various CPU offloading tasks as checksum offload, TSO offload, etc.) on the packet data on the fly.
- While the packet is passing through the DMA, it is stored into the transmit FIFO. After the entire packet is stored in the transmit FIFO, it is then forwareded to transmit switch module.
- The transmit switch arbitrates between host and management packets and eventually forwards the packet to the MAC.
- The MAC appends the L2 CRC to the packet and sends the packet over the wire using a pre-configured interface.
- When all the PCIe completions for a given packet are complete, the DMA updates the appropriate descriptor(s).
- The descriptors are written back to host memory using PCIe posted writes. The head pointer is updated in host momory as well.
- An interrupt is generated to notify the host driver that the specific packet has been read to the 82599 and the driver can then release the buffer(s).


#### 网卡收包过程

- The host creates a descriptor ring and configures one of the 82599's receive queues with the address location, length, head, and tail pointers of the ring(one of 128 available Rx queues).
- The host initializes descriptor(s) that point to empty data buffer(s). The host places these descriptor(s) in the correct location at the appropritate Rx ring.
- The host updates the appropriate Queue Tail Pointer(`RDT`).
- A packet enters the Rx MAC.
- The MAC forwards the packet to the Rx filter.
- If the packet matches the pre-programmed criteria of the Rx filtering, it is forwarded to an Rx FIFO.
- The receive DMA fetches the next descriptor from the appropriate host memory ring to be used for the next receive packet.
- After the entire packet is placed into an Rx FIFO, the receive DMA posts the packet data to the location indicated by the descriptor through the PCIe interface. If the packet size is greater than the buffer size, more descriptors are fetched and their buffers are used for the received packet. 
- When the packet is placed into host memory, the receive DMA updates all the descriptor(s) that were used by the packet data.
- The receive DMA writes back the descriptor content along with status bits that indicate the packet information including what offloads were done on the packet. 
- The 82599 initiates an interrupt to the host to indicate that a new received packet is ready in host memory.
- The host reads the packet data and sends it to the TCP/IP stack for further processing. The host releases the associated buffer(s) and descriptor(s) once they are no longer in use.

