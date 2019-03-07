NIC(Network Interface Card), 网络适配器, 工作在物理层和数据链路层.
- 物理层: 光-电信号转换、数据编解码、电路
- 数据链路层: 寻址、数据帧构建、CRC校验、传输层控制、提供接口给网络层
- 网卡收发包，都是通过[中断](https://github.com/justscu/BL/blob/master/content/CSAPP-8-异常控制流.md)

#### 网卡基本信息

`lspci | grep -i eth`, 找到"Ethernet controller", 可见网卡信息和设备商信息

`lspci -vvv`, 查看网卡设备信息

`ethtool -i eno1`, 网卡驱动版本信息

`ethtool eno1`, 网卡工作模式

```sh
[ll@ll] sudo ethtool eno1
Settings for eno1:
    Supported ports: [ TP ] <----- TP，双绞线; FIBRE, 光纤
    Supported link modes:   10baseT/Half 10baseT/Full 
                            100baseT/Half 100baseT/Full 
                            1000baseT/Full 
    Supported pause frame use: No
    Supports auto-negotiation: Yes
    Advertised link modes:  10baseT/Half 10baseT/Full 
                            100baseT/Half 100baseT/Full 
                            1000baseT/Full 
    Advertised pause frame use: No
    Advertised auto-negotiation: Yes  <----- 是否自动协商
    Speed: 1000Mb/s   <----- 网卡速度
    Duplex: Full      <----- 工作模式(Full, 全双工; Half, 半双工)
    Port: Twisted Pair
    PHYAD: 1
    Transceiver: internal
    Auto-negotiation: on
    MDI-X: on (auto)
    Supports Wake-on: pumbg
    Wake-on: g
    Current message level: 0x00000007 (7)
                   drv probe link
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

```


`ethtool -S eno1`, 统计收发包信息<br/>

```sh
[ll@ll]$ ethtool -S eno1
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
     tx_abort_late_coll: 0
     tx_deferred_ok: 0
     tx_single_coll_ok: 0
     tx_multi_coll_ok: 0
     tx_timeout_count: 0
     tx_restart_queue: 0
     rx_long_length_errors: 0
     rx_short_length_errors: 0
     rx_align_errors: 0
     tx_tcp_seg_good: 26208
     tx_tcp_seg_failed: 0
     rx_flow_control_xon: 0
     rx_flow_control_xoff: 0
     tx_flow_control_xon: 0
     tx_flow_control_xoff: 0
     rx_csum_offload_good: 63526598
     rx_csum_offload_errors: 0
     rx_header_split: 0
     alloc_rx_buff_failed: 0
     tx_smbus: 0
     rx_smbus: 5044
     dropped_smbus: 0
     rx_dma_failed: 0
     tx_dma_failed: 0
     rx_hwtstamp_cleared: 0
     uncorr_ecc_errors: 0
     corr_ecc_errors: 0
     tx_hwtstamp_timeouts: 0
```

#### 网卡发包过程
(1) `数据链路层`从PCI总线收到IP数据包后，将之拆分(MTU限制)成64字节到1518字节的帧。该帧包含MAC层的头部信息（源MAC, 目的MAC, CRC, IP包类型）。<br/>
(2) `物理层`收到从数据链路层过来的帧后，进行编码（每4bit增加1bit的检验码），然后按照物理层的编码规则(NRZ，曼彻斯特编码)把数据编码，再转换成光/电信号发出去。<br/>
(3) 在发数据前，要先进行碰撞检测。

#### 网卡收包过程
(1)网络上的包先被网卡获取。网卡检查MAC是否为本机MAC，校验package的CRC、去掉头部，得到Frame。<br/>
(2)网卡将Frame拷贝到内部FIFO缓冲区，触发中断。<br/>
(3)网卡驱动程序通过中断处理函数，构建sk_buff，将Frame从网卡FIFO拷贝到内存skb，之后交给内核处理。 <br/>


