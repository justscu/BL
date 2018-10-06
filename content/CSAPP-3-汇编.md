###             CH3 汇编基础

#### 3.1 通用寄存器

x86-64共16个通用寄存器，都是64bit的，遵循的规则:
> 生成1字节和2字节数字的指令会保持其余的字节内容不变<br/>
> 生成4字节数字的指令会把高4字节置0.

(1) 通用寄存器名称

 名称 |     作用    | 备注 |
-----:|------------:|------|
 %rax |返回值       |
 %rbx |被调用者保存 |
 %rcx |第4个参数    |
 %rdx |第3个参数    |
 %rsi |第2个参数    |
 %rdi |第1个参数    |
 %rbp |被调用者保存 |
 %rsp |栈指针       |用于指明运行时栈的结束位置
 %r8  |第5个参数    |
 %r9  |第6个参数    |
 %r10 |调用者保存   |
 %r11 |调用者保存   |
 %r12 |被调用者保存 |
 %r13 |被调用者保存 |
 %r14 |被调用者保存 |
 %r15 |被调用者保存 |


(2) 标志寄存器

标志位 |      名称    |   值为 1 时   |
------:|-------------:|---------------|
   CF  |     进位标志 | 发生进位      |
   PF  |     奇偶标志 | 偶数          |
   AF  | 辅助进位标志 | 发生进位      |
   ZF  |       零标志 | 比较结果相等  |
   SF  |     符号标志 | 负数          |
   OF  |     溢出标志 | 发生溢出      |


(3) 寻址方式

 类型 |       格式     |       操作数值       |     寻址方式    |     示例   |
-----:|---------------:|--------------------- |----------------:|------------|
立即数|           $Imm | Imm                  | 立即数寻址      | $0x108 <br/> 立即数0x108 |
寄存器|           ra   | R[ra]                | 寄存器寻址      | %rax <br/> 寄存器rax中的值 |
存储器|           Imm  | M[Imm]               | 绝对寻址        | 0x104 <br/> addr=0x104 <br/> addr中的值| 
存储器|           (ra) | M[R[ra]]             | 间接寻址        | (%rax) <br/>  addr=寄存器rax中的值 <br/> addr中的值 |
存储器|        Imm(rb) | M[Imm+R[rb]]         | (基址+偏移)寻址 | 4(%rax) <br/> addr=寄存器rax中的值+4 <br/> addr中的值 |
存储器|       (rb, ri) | M[R[rb]+R[ri]]       | 变址寻址        | (%rax, %rdx) <br/> addr=寄存器rax中的值+寄存器rdx中的值 <br/> addr中的值 |
存储器|    Imm(rb, ri) | M[Imm+R[rb]+R[ri]]   | 变址寻址        | 9(%rax, %rdx) <br/> addr=寄存器rax中的值+寄存器rdx中的值+9 <br/> addr中的值 | 
存储器|       (,ri, s) | M[R[ri]+s]           | 比例变址寻址    | (, %rdx, 8) <br/> addr=寄存器rdx中的值*8 <br/> addr中的值 |
存储器|    Imm(,ri, s) | M[Imm+R[ri]+s]       | 比例变址寻址    | 0xFC(, %rdx, 8) <br/> addr=寄存器rdx中的值*8+0xFC <br/> addr中的值 |
存储器|    (rb, ri, s) | M[R[rb] + R[ri]*s]   | 比例变址寻址    | (%rax, %rdx, 4) <br/> addr=寄存器rax中的值+寄存器rdx中的值*4 <br/> addr中的值 |
存储器| Imm(rb, ri, s) | M[Imm+R[rb]+R[ri]*s] | 比例变址寻址    | 0xFC(%rax, %rdx, 4) <br/> addr=寄存器rax中的值+寄存器rdx中的值*4+0xFC <br/> addr中的值 |

比例因子s必须为1，2，4，8;


(4) x86-64指令位宽

C语言  |指令 | 位宽(bit) | 示例 |           其它     |
------:|-----|----------:|------|--------------------|
int8   | b   |         8 | movb |
int16  | w   |        16 | movw |
int32  | l   |        32 | movl | 4字节的指令，会把寄存器高4字节清0
int64  | q   |        64 | movq |
long   | q   |        64 |      |
char*  | q   |        64 |      |
float  | ss  |        32 |vomvss|
double | sd  |        64 |vmovsd|


(5) 指令

指令           |    类型    |含义                      |
---------------|:----------:|--------------------------| 
`MOV S, D`     |传送        |目的为内存地址或寄存器    |
`movb`         |            |
`movw`         |            |
`movl`         |            |传送32bit
`movq`         |            |
`movabsq I, R` |            |传送绝对的64bit到寄存器R  |
`MOVZ S, R`    |零扩展传送  |目的为寄存器，以0扩展|
`movzbw`       |            |
`movzbl`       |            |(会把高4字节也清零)       |
`movzbq`       |            |
`movzwl`       |            |
`movzwq`       |            |
`MOVS S, R`    |符号扩展传送|传送符号扩展的到寄存器    |
`movsbw`       |            |
`movsbl`       |            |
`movsbq`       |            |
`movswl`       |            |
`movswq`       |            |
`movslq`       |            |
`cltq`         |            |将%eax符号扩展到%rax     |
`pushq S`      |入栈出栈    |subq $8, %rsp;  movq %rax, (%rsp) | 
`popq  D`      |            |movq (%rsp), %rax;  addq $8, %rsp |
`leaq S, D`    |运算        |加载有效地址(常用于指针), leaq (%rdx, %rdi, 4), %rax; <br/>即%rax=%rax+%rdi*4 |
`INC D`        |            |D += 1, incq 16(%rax)     |
`DEC D`        |            |D -= 1                    |
`NEG D`        |            |D = -D                    |
`NOT D`        |            |D = ~D                    |
`ADD S, D`     |            |                          |
`SUB S, D`     |            |                          |
`XOR S, D`     |            |D ^= S                    |
`OR  S, D`     |            |D |= S                    |
`AND S, D`     |            |D &= S                    |
`SAL k, D`     |            |D <<= k(逻辑左移)         |
`SHL k, D`     |            |D <<= k(算术左移)         |
`SAR k, D`     |            |D >>= k                   |
`SHR k, D`     |            |D >>= k(符号位填充)       |
`imulq  S`     |乘除        |(64bit乘法,单操作数)结果为 S*%rax; 高位存放于rdx, 低位存放在rax |
`mulq  S`      |            |                                                    |
`mul S, D`     |            |(双操作数)D *= S; 结果存放在D中                     |
`imul S, D`    |            |(双操作数)D *= S (无符号)                                |
`clto`         |            |对rax的符号位进行扩展，扩展的结果存放在rdx中             |
`cqto`         |            |对rax的符号位进行扩展. 扩展的符号位放在rdx中             |
`idivq  S`     |            |(64bit除法) 结果为 %rax除以S; 商存放在rax, 余数存放在rdx |
`divq  S`      |            |                                                         |
`div S, D`     |            |                                                         |
`idiv S, D`    |            |                                                         |
`cmp S1, S2`   |比较        |用S2 - S1的结果来设置标志寄存器ZF(S1=S2,ZF=1; else ZF=0) |
`test S1, S2`  |            |用S1 & S2的结果来设置标志寄存器ZF |
`sete   D`     |            |D=ZF (结果相等, ZF=1) |
`setne  D`     |            |D=~ZF                 |
`sets   D`     |            |D=SF (负数,SF=1)      |
`setns  D`     |            |D=~SF                 |
`setg   D`     |            |有符号>               |
`setge  D`     |            |有符号>=              |
`setl   D`     |            |有符号<               |
`setle  D`     |            |有符号<=              |
`seta   D`     |            |无符号>               |
`setae  D`     |            |无符号>=              |
`setb   D`     |            |无符号<               |
`setbe  D`     |            |无符号<=              |
`jmp Label`    |            |跳转                  |
`jmp *Opera`   |            |                      |
`je  Label`    |            |ZF=0 跳转             |
`jne Label`    |            |                      |
`js  Label`    |            |SF=1 负数跳转         |
`jns Label`    |            |                      |
`jg  Label`    |            |有符号>               |
`jge Label`    |            |                      |
`jl  Label`    |            |                      |
`jle Label`    |            |                      |
`ja  Label`    |            |无符号>               |
`jae Label`    |            |                      |
`jb  Label`    |            |                      |
`jbe Label`    |            |                      |
`cmove  S, R`  |条件传送    |根据前一条语句(cmp, test)结果，决定本语句. 相等(ZF=1), 把S的值赋给R |
`cmovne S, R`  |            |不相等                |
`cmovs  S, R`  |            |负数                  |
`cmovns S, R`  |            |非负数                |
`cmovg  S, R`  |            |有符号>; g/ge/l/le    |
`cmova  S, R`  |            |无符号>; a/ae/b/be    |

switch语句，GCC根据case的数量、case值的稀疏程度来翻译switch...case...语句.
当case的数量多于4个且值的跨度范围较小时，使用跳转表.

(6) 函数调用与恢复

栈往下增长。上为大地址，下为小地址。
函数调用过程: 多余参数入栈(若参数个数>6); 返回地址入栈;
> 在代码中，经常可用看到 movq 16(%rsp), %rax，表明把第二个参数赋给rax <br/>
> (%rsp)，函数返回地址 <br/>
> 8(%rsp)，入栈的第一个参数（假设参数为8字节类型）<br/>
> 16(%rsp)，入栈的第二个参数 <br/>

有些寄存器是被调用者可以修改的，有些是被调用者不能修改的。被调用者要想使用这些寄存器，需要先把这些寄存器的值入栈，最后再出栈恢复。


#### 3.2 浮点数寄存器

SSE指令要求数据块16bit对齐，即SSE单元和内存之间传输数据的指令要求内存地址必须是16的整数倍。<br/>
x86-64处理器实现了AVX多媒体指令（SSE指令的超集），支持AVX的指令并没有强制性的对齐要求。<br/>
代码优化建议，64bit系统8字节对齐、32bit系统4字节对齐。<br/>

(1) SIMD指令寄存器长度变化表

指令名称 | 寄存器名称 | 寄存器长度|  寄存器个数  |      编译命令       |
---------|-----------:|----------:|:------------:|---------------------|
MMX      |  mm        |  64 bit   |              |
SSE      | xmm        | 128 bit   |              |
AVX      | ymm        | 256 bit   |16(ymm0-ymm15)| -mavx2, 生成AVX2代码

AVX寄存器名称|长度(bit)|       作用及要求       |
-------------|:-------:|------------------------|
ymm0         |  256    | 1st FP arg返回值
ymm1 - ymm7  |  256    | 2nd FP参数 - 8th FP参数
ymm8 - ymm15 |  256    | 调用者保存

> ymm0-ymm7，可以用来依次传递8个浮点数参数，也可以用栈传递额外的参数 <br/>
> 使用ymm0传递返回值 <br/>
> 所有的ymm都是调用者保存 <br/>

可以把xmm寄存器(128bit)看成一个数组，4个32bit数组[x32,x32,x32,x32]或2个64bit数组[x64,x64]。

AVX(Advanced vector extension, 高级向量扩展)是SIMD的最新版本，每个寄存器256bit，可以一次执行8个四字节或4个八子节的计算。

假设%ymm0包含8个float数据(a0, ..., a8), %rcx包含8个float数据的内存地址。`vmulps (%rcx), %ymm0, %ymm1`, 一次从内存中读取8个值，并行的执行8个乘法。
然后把计算的结果保存在%ymm1中。

(2) 指令缩写对照表

缩写| 含义
----|-----------------|
ss  | 单精度浮点数    |
sd  | 双精度浮点数    |
si  | 整数(默认32bit) |
q   | 64bit           |
cvt | convert         |

(3) 浮点数传送指令表

指令        |   源  |  目的 |            描述                |     寄存器和内存     |
------------|:-----:|:-----:|--------------------------------|----------------------|
vmovss      | M32   | X     | 传送单精度                     | M32: 32bit内存       |
vmovss      | X     | M32   | 传送单精度                     | X: 寄存器xmm         |
vmovsd      | M64   | X     | 传送双精度                     | M64: 64bit内存       |
vmovsd      | X     | M64   | 传送双精度                     |
vmovaps     | X     | X     | 传送对齐的封装好的单精度       | a:aligned，16bit对齐 |
vmovapd     | X     | X     | 传送对其的封装好的双精度       |

(4) 浮点数转换成整数指令表

指令        |   源  | 目的|           描述                      |           速查
------------|:-----:|:---:|-------------------------------------|------------------------|
vcvttss2si  | X/M32 | R32 | 用截断的方法把单精度数转成整数      | 32(float)  -> 32(int)
vcvttsd2si  | X/M64 | R32 | 用截断的方法把双精度数转成整数      | 64(double) -> 32(int)
vcvttss2siq | X/M32 | R64 | 用截断的方法把单精度数转成64bit整数 | 32(float)  -> 64(int64)
vcvttsd2siq | X/M64 | R64 | 用截断的方法把双精度数转成64bit整数 | 64(double) -> 64(int64)

R32: 32bit通用寄存器<br/>
M32: 32bit内存<br/>

整数转换成浮点指令表

指令        |    源1  | 源2  | 目的|        描述           |           速查
------------|:-------:|:----:|:---:|-----------------------|------------------------------|
vcvtsi2ss   | M32/R32 |  X   |  X  | 把32bit整数转成单精度 | 32(int32) -> 32(float)       |
vcvtsi2sd   | M32/R32 |  X   |  X  | 把32bit整数转成双精度 | 32(int32) -> 64(double)      |
vcvtsi2ssq  | M64/R64 |  X   |  X  | 把64bit整数转成单精度 | 64(int64/long) -> 32(float)  |
vcvtsi2sdq  | M64/R64 |  X   |  X  | 把64bit整数转成双精度 | 64(int64/long) -> 64(double) |

(5) 浮点数运算指令表

指令(单精度)|指令(双精度)|效果             |描述
------------|------------|-----------------|------------
vaddss      |vaddsd      |D <- S2 + S1     |加法
vsubss      |vsubsd      |D <- S2 - S1     |减法
vmulss      |vmulsd      |D <- S2 * S1     |乘法
vdivss      |vdivsd      |D <- S2 / S1     |除法
vmaxss      |vmaxsd      |D <- max(S2, S1) |求最大值
vminss      |vminsd      |D <- min(S2, S1) |求最小值     
sqrtss      |sqrtsd      |D <- 开方(S1)    |浮点数平方根

`vdivsd %xmm1, %xmm2, %xmm2`，含义为xmm2 = xmm2 / xmm1;

(6) 浮点数其它指令

指令(单精度)|指令(双精度)|效果         |描述
------------|------------|-------------|-------
vxorps      |vxorpd      |D <- S2 ^ S1 |按位异或
vandps      |vandpd      |D <- S2 & S1 |按位与
ucomiss     |ucomisd     |CF/ZF/PF     |比较指令,根据S2-S1结果设置标志位

