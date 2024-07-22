

未优化系统
====

- RTT, 不开onload, 35+us, 抖动大
- RTT, 开启onload, 35+us, 抖动大

是否开启onload，区别不大

```
CPU: Intel(R) Xeon(R) Gold 6354 CPU @ 3.00GHz
     18core * 2

Current active profile: balanced
   
```


tuned-adm优化
====

`tuned-adm profile network-latency`

- RTT, 不开onload, 20+us, 抖动变小
- RTT, 开启onload, 12+us, 抖动变小


开启大页内存
====

`sysctl -w vm.nr_hugepages=64`,
`echo "vm.nr_hugepages = 64" >> /etc/sysctl.conf`

- RTT, 不开onload, 19+us
- RTT, 开启onload, 4.1 +us

```
[root]# cat /proc/meminfo | grep -i huge
AnonHugePages:   2553856 kB
HugePages_Total:      64
HugePages_Free:       55
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
```


关掉超线程
====

需要设置bios，关闭超线程

- RTT, 开启onload, 4.1 +us



numa亲和性
====

`numactl --hardware`

查看网卡在哪个numa节点: `cat /sys/class/net/<interface>/device/numa_node`, 并将程序绑定在该numa上执行.

`numactl --membind=0 onload program`



关掉网卡校验
====

```
# 查看

ethtool -k ens1f1d1
    Features for ens1f1d1:
    rx-checksumming: off     <--- rx 校验
    tx-checksumming: off     <--- tx 校验
        
ethtool -K <interface_name> rx <on|off>

```


RX checksum offload
====

`ethtool --offload <interface_name> [rx on|off]`



关掉网卡中断聚合
====

`ethtool -C <interface_name> adaptive-rx off`

- RTT, 开启onload, 4.1 +us


```
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

    # 禁用自适应聚合
    ethtool -C <interface_name> adaptive-rx off
    
    # 设置中断聚合时间间隔
    ethtool -C <interface_name> rx-usecs <interval>
    ethtool -C <interface_name> tx-usecs <interval>
    
    # 例子，将接收的中断聚合时间，调整为0.
    ethtool -C <interface_name> rx-usecs 0
```



调整中断亲和性
====

```
# 去掉onload驱动
onload_tool unload --onload-only

# 禁止中断服务
/sbin/service irqbalance stop

# 启动onload驱动
onload_tool reload --onload-only

```

- RTT, 开启onload, 3950 ns



