### CH8 异常控制流

正常情况下，程序通过函数调用和栈规则，在不同的函数间跳转.

异常控制流(ECF, Exceptional Control Flow). <br/>
> 硬件层，硬件检测到的某些事件会触发控制突然转移到异常处理程序 <br/>
> 操作系统层，内核通过上下文切换，将控制从一个用户进程转移到另一个用户进程 <br/>
> 应用层，进程A收到另一个进程B发来的信号，进程A会把控制转移到对应的信号处理函数 <br/>

#### 8.1 异常的类别

类别 | 原因             | 异步/同步 | 返回行为
-----|------------------|-----------|---------------------|
中断 | 来自io设备的信号 | 异步      | 总是返回到下一条指令
陷阱 | 有意的异常       | 同步      | 总是返回到下一条指令
故障 | 潜在可恢复的错误 | 同步      | 可能返回到下一条指令
终止 | 不可恢复的错误   | 同步      | 不会返回

中断是来自处理器外部io设备的信号，如网卡、磁盘、定时器芯片等。<br/>
陷阱最重要的用途是在用户进程与内核间提供一个接口，叫做系统调用。如read, fork, syscall n. <br/>

`系统调用`是通过一条syscall的陷阱指令来提供的。Linux系统调用的参数都是通过寄存器(而不是栈)来传递；%rax包含系统调用号, %rdi, %rsi, %rdx, %r10, %r8, %r9包含最多6个参数。
%rax包含返回值. -4095到-1间的负数表明了错误，对应errno.
```cpp
write(stdout, "hello world!", 12);
```
对应的汇编代码
```
movq $1, %rax        // write is system call 1
movq $1, %rdi        // stdout is 1
movq $string, %rsi   // hello world!
movq $12, %rdx       // length: 12
syscall              // make the system call
```

#### 8.2 进程
处理器提供一种机制，限制一个应用可以执行的指令以及它可以访问的地址空间范围。<br/>
控制寄存器中有一个模式位(mode bit)，用来描述进程当前享有的特权。<br/>
当设置模式位时，进程就运行在`内核模式`，可以执行指令集中的任何指令，并且可以访问内存中的任何位置。<br/>
未设置模式位时，进程运行在`用户模式`，该模式中的进程不允许执行特权指令，如停止处理器、发起io操作。<br/>

进程从用户模式变为内核模式的唯一方法是通过中断、系统调用等方法。

#### 8.3 中断
`cat /proc/interrupts` 查看中断是如何分配到CPU上.
```
 IRQ        CPU0       CPU1       CPU2       CPU3       CPU4       CPU5       CPU6       CPU7       
   0:         44          0          0          0          0          0          0          0   IO-APIC-edge      timer  <-- 系统时钟
   1:          1          0          0          0          1          1          0          0   IO-APIC-edge      i8042  <-- 键盘鼠控制器标
   8:          0          0          0          0          0          0          0          1   IO-APIC-edge      rtc0   <-- real time clock 
   9:          0          0          0          0          0          0          0          0   IO-APIC-fasteoi   acpi
  12:          4          0          0          0          0          0          1          0   IO-APIC-edge      i8042
  19:          0          1          0          0          1          9          2          0   IO-APIC-fasteoi 
 120:     177235       5420       5454       4062      14303      13252       9697       8608   PCI-MSI-edge      xhci_hcd
 121:       4465        124         88         42        280        167         92     564265   PCI-MSI-edge      0000:00:17.0
 122:    6552397          4          4          2          9          3          2          3   PCI-MSI-edge      eno1  <-- 网卡
 123:        189         11        460        614         68        892         32        694   PCI-MSI-edge      snd_hda_intel
 NMI:        210        221        213        192        201        108        126        108   Non-maskable interrupts
 LOC:    5844679    6745349    6886422    6989934    3530139    2111295    2372276    2063782   Local timer interrupts
 SPU:          0          0          0          0          0          0          0          0   Spurious interrupts
 PMI:        210        221        213        192        201        108        126        108   Performance monitoring interrupts
 IWI:    1091740     142327      65947      47101      21076      20302      16639      18989   IRQ work interrupts
 RTR:          7          0          0          0          0          0          0          0   APIC ICR read retries
 RES:     405360     341571     292259     304711     187857     207528     178397     178327   Rescheduling interrupts
 CAL:        748        812        871        879        769        840        810        734   Function call interrupts
 TLB:      26927      28311      29044      28625      19304      21306      19733      19085   TLB shootdowns
 TRM:          0          0          0          0          0          0          0          0   Thermal event interrupts
 THR:          0          0          0          0          0          0          0          0   Threshold APIC interrupts
 MCE:          0          0          0          0          0          0          0          0   Machine check exceptions
 MCP:        392        392        392        392        392        392        392        392   Machine check polls
 ERR:          0
 MIS:          0
```

第一列为中断号(IRQ)，后面为在各CPU核心上中断次数. IRQ越小优先级越高。

`cat /proc/irq/122/smp_affinity`显示122号中断绑定在哪个CPU核心上。SMP为对成称多处理器。
```sh
cat /proc/irq/122/smp_affinity
01
# 01，表示在0号CPU上中断(16进制数字显示)
# ff，表示在0-7号CPU上中断

echo "80" > /proc/irq/122/smp_affinity
# 将122号中断绑定到7号CPU上.
```

