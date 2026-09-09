打造高精度时间同步

`PTP`，精确时间协议 (Precision Time Protocol) ，对时精度可< 1us; NTP的对时精度 < 5ms。

PTP网络中，有个精确时钟做为`主时钟`(Master Clock)，其它设备做为`从时钟`(Slave Clock)，用来从主时钟同步自身时间。

![image](https://github.com/justscu/BL/blob/master/pics/network_ptp时间同步.png)

- 在使用`ptp`前，须先停用`NTP`服务
```
systemctl status  ntpd
systemctl stop    ntpd
systemctl disable ntpd
systemctl status  chronyd
systemctl stop    chronyd
systemctl disable chronyd
```

- 检查网卡的硬件时戳能力
`ethtool -T ethx`
```
[~]$ ethtool -T enp94s0f1
Time stamping parameters for enp94s0f1:
Capabilities:
    hardware-transmit
    software-transmit
    hardware-receive
    software-receive
    software-system-clock
    hardware-legacy-clock
    hardware-raw-clock
PTP Hardware Clock: 0   <--------- 必须包含
Hardware Transmit Timestamp Modes:
    off
    on
Hardware Receive Filter Modes:
    none
    all
```

- 网卡与主时钟同步
```
yum install linuxptp
ptp4l -f /etc/ptp4l.conf -i ethx -m -s -H, -H开启硬件打戳
```

观察日志中的`master offset`，若offset稳定在100~150ns，说明网卡层面已完成对时.

- OS内核与网卡同步
```
phc2sys -s ethx -c CLOCK_REALTIME -w -m
```
若稳定的话，phc2sys的日志offset，会稳定在200ns以内.

- 开机启动
```
/etc/systemd/system/ptp4l.service
/etc/systemd/system/phc2sys.service

systemctl enable ptp4l
systemctl enable phc2sys
```
