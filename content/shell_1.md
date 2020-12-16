### 1 开始
```sh
#! /bin/sh
或 
#! /bin/bash
添加调试选项
#! /bin/bash -xv
-x：在执行时显示参数和命令
-v：在命令进行读取时显示输入
```

### 2 输入/输出
```sh
read var # 从命令行读入字符，放入var中，以回车结束 
read -n 20 var # 从命令行读入20个字符，放入var中 
read -p "Please Input: " var # 输出提示信息，并读入用户的输入 
read -t timeout var # 带超时的读入

var=abcdef    # 值中间不含空格，不需要用引号 
var="abc def" # 值中间含空格，需要用引号 

echo "${var}" # 输出 abcdef，双引号，会求值 
echo '${var}' # 输出 ${var}，单引号，不会求值 
echo ${#var}  # 求var的长度 

# printf 打印，完全按照c的格式进行打印 
printf "%-5s %-6d %-5.4f \n" DZH 601519 19.635276

kill -0 pid   # 不向进程pid发送信号，但若pid存在，返回0；否则返回1
echo $?       # 返回上次命令的执行结果：0成功；非0失败
```

### 3 变量
```sh
var="aaaa bb"   # 等号左右不能有空格，有空格就变成比较了 
var = "aaaa bb" # 比较命令
```

### 4 环境变量
```sh
PATH="${PATH}:/home/ll/zk/" # 将 /home/ll/zk/加入到环境变量 
export PATH
# 上面两条等价于 
export PATH="${PATH}:/home/ll/zk/"

在 ~/.bashrc中，定义了提示的颜色，cat ~/.bashrc | grep PS1
```

### 5 数学运算符 let、$[ ]、$(( ))、``、bc
```sh
n1=4
n2=5

let ret1=${n1}+n2 # 加号两端不能有空格， let 关键字
echo ${ret1} # 9

ret2=$[ ${n1} + n2 ] 或者 ret2=$(( ${n1} + n2 ))
echo ${ret2}

let n1++
let n2--

n3=54
ret3=`echo "${n3}*1.06" | bc` # bc是一个数学计算集合
echo ${ret3}
```

### 6 重定向 tee
```sh
# 将stderr重定向到stderr.log中，将stdout重定向到stdout.log中 
cmd 2> stderr.log 1>stdout.log 
# fd在右边使用时，需要加上& 
cmd 2>&1 output.log # 将stderr重定向到stdout中，并一起输出到output.log中
# 等价于 
cmd &> output.log
# 将stderr的信息输入到/dev/null中
cmd 2> /dev/null 

# tee命令将cmd的输出写一份到file1中，同时提供一份拷贝输出到file2中
cmd | tee file1 file2 
# 将"ls -al"的内容拷贝一份到a.log中，同时提供一份拷贝，作为"wc -l"的输入
ls -al | tee a.log | wc -l 
# tee -a，追加内容到a.log
ls -al | tee a.log | wc -l 
# 将文件file的内容(重定向)作为cmd命令的输入
cmd < file
```

### 7 重定向脚本内部的文本块
```sh
# 下面的脚本，将"cat <<EOF> logfile"和"EOF"之间的内容，定向输出到logfile中
#! /bin/bash
cat <<EOF> logfile
LOG FILE HEADER
This is a test log file
Func: system statistic
EOF
```

### 8 自定义文件描述符 exec
```sh
echo "This is a test file" > test.log
# 创建一个文件描述符fd=3，并读取test.log文件中的内容到fd=3
exec 3< test.log 
# 将fd=3中的内容作为cat的输入，即执行的结果为：This is a test file
cat <&3 

# 创建一个文件描述符fd=4，并将fd=4的内容重定向到out.log文件
exec 4>out.log 
# 将内容输出到fd=4的文件描述符
echo "This is a test" >&4 
# 执行的结果为：This is a test
cat out.log
```

### 9 用整数作为index的数组
```sh
array_int=(0 1 2 3 4 5 6)
echo ${array_int[2]} # 输出 2

array_str[0]="test0"
array_str[1]="test1"
array_str[2]="test2"
array_str[3]="test3"
index=2
echo ${array_str[${index}]} # 输出test2
echo ${array_str[*]} # 输入数组的内容
echo ${array_str[@]} # 输入数组的内容
echo ${#array_str[*]} # 输入数组长度
echo ${#array_str[@]} # 输入数组长度
```

### 10 用string作为index的数组
```sh
#! /bin/bash
# 使用declare将 ass_array声明为关联数组
declare -A ass_array
ass_array=([k1]=v1 [k2]=v2)
# 或者
ass_array[k3]=v3
ass_array[k4]=v4
# echo ${ass_array[@]}
# 遍历
for k in ${ass_array[@]}
do
    echo ${k}
# echo ${ass_array[${k}]}
done
# 使用index
echo ${ass_array[k3]} # 输出v3
```

### 11 别名 alias
```sh
# alias作用是暂时的，当shell关闭后，将不再起作用
alias install='sudo apt-get install' 

# 为了使alias长期有效，可以将其放入到 ~/.bashrc中 
unalias，删除别名
alias rm='cp $@ ~/rmback/; rm $@'
unalias rm
```

### 12 时间
```sh
date
date +%s # 1418783537, 取秒数
date +%Y%m%d #取年月日
date -d "7 day ago" +%Y%m%d #取7天前的日期

# 删除7天前的文件或文件夹
function rm_fold()
{
    root_path=$1
    t=`date -d "7 day ago" +%Y%m%d`                                                                                                                                                                                

    files=`ls ${root_path}`
    for file in ${files[@]}
    do
        if [[ ${t} -ge ${file} ]]; 
        then
            rm -rf ${root_path}/${file}
        fi
    done
}

rm_fold  /home/ll/xxx/
```

### 13 函数/参数/递归
```sh
function fname() # 不写function也可以
{
    echo $1, $2 # 访问参数1和参数2
    echo "$@"   # 以列表形式一次打印所有参数
    echo "$*"   # 类似于$@，但参数作为单个实体
    return 0    # 返回 0
}

fname                 # 无参数调用函数
fname arg1 arg2 arg3  # 带参数调用函数

$0 脚本名字
$1 第一个参数
"$@"，被扩展为 "$1" "$2" "$3" ...
"$*"，被扩展为 "$1c$2c$3"，其中c为IFS的第一个字符

F()
{
    echo $1
    sleep 2
    F $[$1+1]  #函数递归
}

F 9 # 函数调用
```

### 14 命令的输出 $( ) 、``
```sh
cmd_output=$(ls -al | cat -n) # 用 $() 作为命令输出
echo ${cmd_output}

cmd_output=`ls -al | cat -n`  # 用反引号作为命令输出
echo ${cmd_output}

ls -al
echo $? # 输出cmd命令的返回值
```

### 15 子shell

(1)使用()来定义一个子shell，子shell不会对当前shell有任何影响
```sh
cd /home/ll
(cd /usr/bin; pwd) # 启动一个子进程，不对父进程有影响
pwd                # 输出 /home/ll
```

(2) 要保留子shell中的空格和换行符，请使用""双引号
```sh
$ cat a.txt
1, 0,
2,
3
$ out=$(cat a.txt)
$ echo ${out}    # 未使用""双引号的输出
1, 0, 2, 3
$ echo "${out}"  # 使用""双引号的输出，保留空格和换行
1, 0,
2,
3
```

### 16 IFS（Internal Field Separtor），内部字段分隔符
```sh
#! /bin/bash
data="name;sex;rollno;location"
oldIFS=${IFS} # 保留原来的IFS
IFS=;         #将";"作为新的IFS
for item in ${data}
do
    echo ${item} # 输出的内容为:name sex rollno location
done

IFS=$oldIFS      # 恢复原来的IFS

#########
line="root:x:0:0:root:/root:/bin/bash"
oldIFS=${IFS}
IFS=":"
count=0
for item in ${line}
do
    [ ${count} -eq 0 ] && user=${item}
    [ ${count} -eq 6 ] && shell=${item}
    let count++
done
IFS=${oldIFS}

echo ${user}\'s shell is ${shell}
```

### 17 循环
```sh
# 第一种)
for var in list;
do
    commands;
done

# 第二种)
for((i=0;i<10;i++))
{
    commands;
}

# 第三种)
while condition
do
    commands;
done

# 第四种)
x=0
until [ ${x} -eq 9 ];
do
    let x++; echo ${x}
done
```

### 18 比较
```sh
# 第一种)
if condition; then
    commands;
fi

# 第二种)
if condition1; then
    commands;
elif condition2; then
    commands;
else
    commands;
fi

# 第三种)
if [ condition ] && action; then
fi

if [ condition ] || action; then
fi

算术比较
if [ ${var} -eq 0 ]; then
fi

-eq 等于；-ne不等于；-gt大于；-lt小于；-ge大于或等于；-le小于或等于
文件比较
if [ -f ${file} ]; then
fi

-f文件存在；-d是目录
字符串比较
if [[ ${str1} = ${str2} ]]; then
fi

=相等；==相等；!=不等；>大于；<小于；
[[ -z ${str} ]] 字符串为空，则返回true
[[ -n ${str} ]] 字符串非空，则返回true
注意：比较符号的两边是有空格的，等号的两边没有空格:
${v1} = "abc" # 比较
${v1}="abc" # 赋值

# 第四种)
if [[ -n ${str1} ]] && [[ -z ${str2} ]]; then
    commands;
fi

# 第五种)
if test -n ${str1} && test -z ${str2}; then
    commands;
fi
```

### 19 case
```sh
case "$1" in  # 判断参数1
     start)
         echo " Start LVS of RealServer"
         /sbin/ifconfig eno1:2 ${VIP} broadcast ${VIP} netmask 255.255.255.0 up
         /sbin/route add default gw ${GW}
     ;;

     stop)
         echo "Stop LVS of RealServer"
         /sbin/ifconfig eno1:2 down
         /sbin/route del default gw ${GW}
     ;;

     *)
         echo "Usage: $0 {start|stop}"
         exit 1
esac
```
