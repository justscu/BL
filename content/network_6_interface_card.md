NIC(Network Interface Card), 网络适配器(网卡)，工作在物理层和数据链路层。主要由PHY/MAC芯片、Tx/Rx FIFO、DMA等组成.

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


#### 网卡队列

`网卡多队列`，指网卡内部维护多个收发队列，并产生多个中断信号，使用多个CPU来处理网卡收到的包，来提升网络处理性能

`cat /proc/interrupts`，查看中断号

`cat /proc/irq/中断号/smp_affinity`，中断分配到哪个CPU上，可以调整
