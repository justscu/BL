# 打造高精度时间同步

`PTP`，精确时间协议 (Precision Time Protocol) ，对时精度可< 1us; `NTP`的对时精度 < 5ms。

PTP网络中，有个精确时钟作为`主时钟`(Master Clock)，其它设备作为`从时钟`(Slave Clock)，用来从主时钟同步自身时间。
主/从时钟的身份，是PTP网络中自动协商来的，在实际配置中，也可以根据需要手动调整。

![image](https://github.com/justscu/BL/blob/master/pics/network_ptp时间同步.png)

- 在使用`PTP`前，须先停用`NTP`服务
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

- 从时钟的设置

服务器一般设置成`纯从时钟`，使其被动接受上游时间.
```
# vim /etc/ptp4l.conf

[global]
# 强制设置成从时钟
slaveOnly 1

```

- 网卡与主时钟同步
```
yum install linuxptp
ptp4l -f /etc/ptp4l.conf -i ethx -m -s -H
-H, 开启硬件打戳
-m, 在终端输出
-s，设置为slave
```

观察日志中的`master offset`，若offset稳定在100~150ns，说明网卡层面已完成对时.

- OS内核与网卡同步
```
phc2sys -s ethx -c CLOCK_REALTIME -w -m
```
若稳定的话，`phc2sys`的日志offset，会稳定在200ns以内.

- 开机启动
```
/etc/systemd/system/ptp4l.service
/etc/systemd/system/phc2sys.service

systemctl enable ptp4l
systemctl enable phc2sys
```
