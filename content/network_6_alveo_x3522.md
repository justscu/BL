
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


