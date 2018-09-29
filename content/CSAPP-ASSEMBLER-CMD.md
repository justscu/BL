## 汇编基础

### 1 寄存器

#### 通用寄存器

x86-64共16个寄存器，都是64bit的，遵循的规则:
（1）生成1字节和2字节数字的指令会保持其余的字节内容不变；（2）生成4字节数字的指令会把高4字节置0.

 名称 |     作用    | 备注 |
-----:|------------:|-----:|
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


####  标志寄存器

标志位 |      名称    |   值为 1 时   |
------:|-------------:|---------------|
   CF  |     进位标志 | 发生进位      |
   PF  |     奇偶标志 | 偶数          |
   AF  | 辅助进位标志 | 发生进位      |
   ZF  |       零标志 | 比较结果相等  |
   SF  |     符号标志 | 负数          |
   OF  |     溢出标志 | 发生溢出      |


#### 寻址方式

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


#### x86-64指令位宽

C语言  |指令 | 位宽(bit) | 示例 |           其它     |
------:|-----|----------:|------|--------------------|
int8   | b   |         8 | movb |
int16  | w   |        16 | movw |
int32  | l   |        32 | movl | 4字节的指令，会把寄存器高4字节清0
int64  | q   |        64 | movq |
long   | q   |        64 |      |
char*  | q   |        64 |      |
float  | s   |        32 |      |
double | l   |        64 |      |


### 2 指令

指令         |类型        |含义                      |
-------------|------------|--------------------------| 
MOV S, D     |传送        |目的为内存地址或寄存器    |
movb         |            |
movw         |            |
movl         |            |传送32bit
movq         |            |
movabsq I, R |            |传送绝对的64bit到寄存器R  |
MOVZ S, R    |零扩展传送  |目的为寄存器，以0扩展|
movzbw       |            |
movzbl       |            |(会把高4字节也清零)       |
movzbq       |            |
movzwl       |            |
movzwq       |            |
MOVS S, R    |符号扩展传送|传送符号扩展的到寄存器    |
movsbw       |            |
movsbl       |            |
movsbq       |            |
movswl       |            |
movswq       |            |
movslq       |            |
cltq         |            |将%eax符号扩展到%rax     |
pushq S      |入栈出栈    |subq $8, %rsp;  movq %rax, (%rsp) | 
popq  D      |            |movq (%rsp), %rax;  addq $8, %rsp |                                   |
leaq S, D    |运算        |加载有效地址, leaq (%rdx, %rdi, 4), %rax; <br/>即%rax=%rax+%rdi*4 |
INC D        |            |D += 1, incq 16(%rax)     |
DEC D        |            |D -= 1                    |
NEG D        |            |D = -D                    |
NOT D        |            |D = ~D                    |
ADD S, D     |            |                          |
SUB S, D     |            |                          |
XOR S, D     |            |D ^= S                    |
OR  S, D     |            |D |= S                    |
AND S, D     |            |D &= S                    |
SAL k, D     |            |D <<= k(逻辑左移)         |
SHL k, D     |            |D <<= k(算术左移)         |
SAR k, D     |            |D >>= k                   |
SHR k, D     |            |D >>= k(符号位填充)       |
imulq  S     |乘除        |(64bit乘法,单操作数)结果为 S*%rax; 高位存放于rdx, 低位存放在rax |
mulq  S      |            |                                                    |
mul S, D     |            |(双操作数)D *= S; 结果存放在D中                     |
imul S, D    |            |(双操作数)D *= S (无符号)                                |
clto         |            |对rax的符号位进行扩展，扩展的结果存放在rdx中             |
cqto         |            |对rax的符号位进行扩展. 扩展的符号位放在rdx中             |
idivq  S     |            |(64bit除法) 结果为 %rax除以S; 商存放在rax, 余数存放在rdx |
divq  S      |            |                                                         |
div S, D     |            |                                                         |
idiv S, D    |            |                                                         |
cmp S1, S2   |比较        |用S2 - S1的结果来设置标志寄存器ZF(S1=S2,ZF=1; else ZF=0) |
test S1, S2  |            |用S1 & S2的结果来设置标志寄存器ZF |
sete   D     |            |D=ZF (结果相等, ZF=1) |
setne  D     |            |D=~ZF                 |
sets   D     |            |D=SF (负数,SF=1)      |
setns  D     |            |D=~SF                 |
setg   D     |            |有符号>               |
setge  D     |            |有符号>=              |
setl   D     |            |有符号<               |
setle  D     |            |有符号<=              |
seta   D     |            |无符号>               |
setae  D     |            |无符号>=              |
setb   D     |            |无符号<               |
setbe  D     |            |无符号<=              |
jmp Label    |            |跳转                  |
jmp *Opera   |            |                      |
je  Label    |            |ZF=0 跳转             |
jne Label    |            |                      |
js  Label    |            |SF=1 负数跳转         |
jns Label    |            |                      |
jg  Label    |            |有符号>               |
jge Label    |            |                      |
jl  Label    |            |                      |
jle Label    |            |                      |
ja  Label    |            |无符号>               |
jae Label    |            |                      |
jb  Label    |            |                      |
jbe Label    |            |                      |
cmove  S, R  |条件传送    |根据前一条语句(cmp, test)结果，决定本语句. 相等(ZF=1), 把S的值赋给R |
cmovne S, R  |            |不相等                |
cmovs  S, R  |            |负数                  |
cmovns S, R  |            |非负数                |
cmovg  S, R  |            |有符号>; g/ge/l/le    |
cmova  S, R  |            |无符号>; a/ae/b/be    |
