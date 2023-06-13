查看超线程状态

```
# 查看当前是否开启了超线程，1为开启；0为未开启
cat /sys/devices/system/cpu/smt/active
1 
# 开启超线程
echo on  > /sys/devices/system/cpu/smt/control
# 关闭超线程
echo off > /sys/devices/system/cpu/smt/control

```


查看cpu信息:`lscpu`

```
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian

CPU(s):                48     <--- 共48CPU机器
On-line CPU(s) list:   0-47
Thread(s) per core:    2      <--- 每个物理核，2个线程，启用了超线程
Core(s) per socket:    12     <--- 每颗CPU，12个物理core
Socket(s):             2      <--- 2颗物理CPU
NUMA node(s):          2      <--- 2NUMA

Vendor ID:             GenuineIntel
CPU family:            6
Model:                 85
Model name:            Intel(R) Xeon(R) Gold 6256 CPU @ 3.60GHz
Stepping:              7
CPU MHz:               4299.829    <--- 实时CPU频率
CPU max MHz:           4500.0000   <--- 最高频率
CPU min MHz:           1200.0000   <--- 最低频率
BogoMIPS:              7200.00
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              1024K
L3 cache:              33792K

NUMA node0 CPU(s):     0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46
NUMA node1 CPU(s):     1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47
Flags:                 fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch epb cat_l3 cdp_l3 invpcid_single intel_ppin intel_pt ssbd mba ibrs ibpb stibp ibrs_enhanced tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm cqm mpx rdt_a avx512f avx512dq rdseed adx smap clflushopt clwb avx512cd avx512bw avx512vl xsaveopt xsavec xgetbv1 cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local dtherm ida arat pln pts pku ospke avx512_vnni md_clear spec_ctrl intel_stibp flush_l1d arch_capabilities

```

机器是否启用numa: `grep -i numa /var/log/dmesg`

```
# grep -i numa /var/log/dmesg

# 未启用
[    0.000000] No NUMA configuration found

# 启用
[    0.000000] NUMA: Initialized distance table, cnt=2
[    0.000000] NUMA: Node 0 [mem 0x00000000-0x7fffffff] + [mem 0x100000000-0x207fffffff] -> [mem 0x00000000-0x207fffffff]
[    0.000000] Enabling automatic NUMA balancing. Configure with numa_balancing= or the kernel.numa_balancing sysctl
[    1.206216] pci_bus 0000:00: on NUMA node 0
[    1.217988] pci_bus 0000:17: on NUMA node 0
[    1.220065] pci_bus 0000:3a: on NUMA node 0
[    1.221605] pci_bus 0000:5d: on NUMA node 0
[    1.222342] pci_bus 0000:80: on NUMA node 1
[    1.226575] pci_bus 0000:85: on NUMA node 1
[    1.228777] pci_bus 0000:ae: on NUMA node 1
[    1.230421] pci_bus 0000:d7: on NUMA node 1

```


查看numa cpu和内存: `numactl --hardware`

```
available: 2 nodes (0-1)
node 0 cpus: 0 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40 42 44 46
node 0 size: 128384 MB    <--- node0上的内存，128G
node 0 free: 98888 MB     <--- node0上的free内存，98G. 如果node0上的需要再申请的内存大于98G，需要跨节点申请内存, 需要swap

node 1 cpus: 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31 33 35 37 39 41 43 45 47
node 1 size: 128987 MB    <--- node1上的内存，128G
node 1 free: 90370 MB     <--- node1上的free内存，90G
node distances:
node   0   1 
  0:  10  21 
  1:  21  10 

```


查看numa各节点内存  状态: `numastat`

```
# numastat
                           node0           node1
numa_hit              3165351925      5291427350
numa_miss              121228076        77256262       <--- miss比较多，说明需要调整分配策略，如绑核，以提高内存命中率
numa_foreign            77256262       121228076       <--- 被其它node访问的内存大小
interleave_hit             60113           60168
local_node            3165338089      5291352630       <--- 在该Node中的进程，在该Node节点中成功分配内存的次数
other_node             121241912        77330982       <--- 在该Node中的进程，在其它Node节点中成功分配内存的次数


# numastat -p 4119， 查看进程（id=4119）的信息

Per-node process memory usage (in MBs) for PID 4119 (test)
                           Node 0          Node 1           Total
                  --------------- --------------- ---------------
Huge                         0.00            0.00            0.00
Heap                       280.50            4.35          284.85
Stack                        0.05            0.00            0.05
Private                   4103.52          697.27         4800.79
----------------  --------------- --------------- ---------------
Total                     4384.06          701.62         5085.69

```


numa应该保证本node的cpu访问本node的内存，以提高内存访问效率.

`numactl --cpunodebind=1 program`, 把程序绑定到node1节点. <br/>
`numactl --physcpubind=1,3,5,7 program`, 把程序绑定到1/3/5/7 cpu上. <br/>
`numactl --membind=0 program`, 在node0节点上分配内存，若该节点内存不足，会分配失败. <br/>
`numactl --localalloc program`, 在当前节点分配内存. <br/>

```
# 示例
numactl --cpunodebind=0 onload_tool reload #将onload绑定在NODE0上
numactl --cpunodebind=1 --membind=1  ./test
numactl --cpunodebind=1 --localalloc ./test
```


查看网卡在哪个NUMA节点上 `cat /sys/class/net/<interface>/device/numa_node`



