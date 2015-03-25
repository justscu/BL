
### 1 Makefile
#### 1.1 编译规则
- 若工程没有编译过，则编译所有cpp文件，并链接生成目标文件；
- 若改变了某些cpp文件，则只编译这些改变过的cpp文件，并链接生成目标文件；
- 若改变了某些.h文件，则编译包含这些.h文件的cpp文件，并链接生成目标文件；

#### 1.2 格式
```sh
目标：依赖文件
    gcc命令
    shell命令
```
```sh
#声明变量objs
objs=main.o a.o b.o c.o 
pushProxy: ${objs} #生成目标pushProxy
    mkdir ./bin    #执行shell命令
    cc -o push ${objs}  #执行gcc命令

.PHONY: clean  # .PHONY表明clean是个伪目标
clean:
    -rm pushProxy #rm前的-号，表示出错了，也继续往下执行
    -rm ${objs}
```
make pushProxy，make clean。若make不带参数，默认执行第一个；
Makefile中，必须以Tab键开始；
"#"后跟注释，可以使用"\#"进行转义。

#### 1.3 Makefile的命名规则
可以使用makefile，Makefile，GUNmakefile，Make.Linux，Make.Solaris，xxx.mk等。

make命令会在当前目录下依次查找`GUNmakefile->makefile -> Makefile`，也可以使用`make -f Make.Linux`来指定相关的文件。

#### 1.4 包含其他Makefile文件 -- include
```sh
include foo.make a.mk ./zk/*.mk
```
注意：include前不能有Tab; ./zk/*.mk表示zk下的.mk文件; 支持通配符

#### 1.5 通配规则
```sh
BINDIR=~/goproject/bin 
ETCDIR=~/goproject/etc
rm -rf *.o #表示要删除所有的.o文件
```

#### 1.6 嵌套Makefile
假设有个子目录叫做zkdir，zkdir下有个Makefile文件，则总控的Makefile可以这样写：
```sh
zkdir:
    cd zkdir && $(MAKE)
或者写成
zkdir:
    $(MAKE) -C zkdir
# 注意：总控Makefile文件中的变量，是可以传递到子Makefile中的

# 如nginx中的总控Makefile写法
build:
    $(MAKE) -f objs/Makefile
    $(MAKE) -f objs/Makefile manpage

install:
    $(MAKE) -f objs/Makefile install

# 在nginx/objs下有个Makefile文件
```

#### 1.7 变量
Makefile中可以声明变量，且变量是大小写敏感的。
```sh
#大小写敏感
BINDIR = ~/goproject/bin
BinDir = ~/xxx

rm $(BINDIR)
# 或者
rm ${BINDIR}
# 真实的$字符，用$$

EXEDir := ~/xxx/cc   # := 表示前面的变量，不能使用后面声明的变量
CurDIr := $(shell pwd) #当前路径
ZKDir ?= /home/ll/zookeeper #若没有声明ZKDir，则此处声明ZKDir

#等价于

ifeq($(origin ZKDir), undefined)
	ZKDir = /home/ll/zookeeper
endif

#追加变量
objs := main.o a.o b.o
objs += c.o   # 用 +=
objs := $(objs) d.o # 用 :=
```

#### 1.8 条件判断
```sh
ifeq...endif；ifeq...else...endif；
ifneq...endif；ifneq...else...endif；
ifdef...endif；ifdef...else...endif；
ifndef...endif；ifndef...else...endif；

libs1=-lgun
libs2=-lmvs

ifeq($(CC), gcc)
    $(CC) -o foo $(objs) $(libs1) #是gcc，使用libs1
else
    $(CC) -o foo $(objs) $(libs2) #非gcc，使用libs2
endif
```

#### 1.9 函数
格式：$(funcname arg1, arg2, ..., argn)

```sh
#把字符串text中的from替换为to
$(subst <from>, <to>, <text>)
text:=hello,
from:=l
to:=d
change = $(subst $(from), $(to), $(text))
处理完毕，change为heddo,wordd!

#模式匹配替换，将text中符合patten的格式，替换为replacement
$(patsubst <patten>, <replacement>, <text>) 
src = a.c b.c c.c
patten=%.c
rep=%.o
dst = $(patsubst $(patten), $(rep), $(text))
替换后，dst为a.o b.o c.o
等价于 $(text: <patten>=<replacement>)

#去掉string开头和结尾的空格
$(trip <string>)
src = a b c
dst = $(trip $(src))
最后dst=a b c

#在in中查找find，若找到返回find，否则返回空
$(findstring <find>, <in>) 
f=ve
in=level
$(findstring $(f), $(in))
能找到，返回ve

#查找text中符合pattenN的单词，并返回
$(filter <patten1 patten2 ... pattenN>, <text>) 
patt=%.c %.h
src=a.c b.c c.c a.pp b.pp a.h b.hh a.o b.o
dst=$(filter $(patt) $(src))
结果为a.c b.c c.c a.h

#反过滤，保留不符合pattenN的单词
$(filter-out <patten1 ... pattenN> <text>) 

#给list中的单词排序，并返回排序后的单词
$(sort <list>) 

#返回text中的第n个单词，从1开始计数
$(word <n>, <text>) 
src=in a world, you can free
dst=$(word 4, $(src))
结果为you

#返回text中，[start, end]的单词
$(wordlist <start>,<end>,<text>) 
dst=$(wordlist 2, 4, $(src))
结果为a world, you


#返回text中单词个数
$(words <text>)

#返回text中第一个单词
$(firstword <text>) 

src=you can sleep, after 5 hours later.
$(word $(words $(src)), $(src)) #返回最后一个单词later.

$(dir <names...>) #返回目录，若没有目录，则返回./
$(notdir <names...>) #返回文件名
src=src/zookeeper.c main.cpp
dst1=$(dir $(src)) #返回src/ ./
dst2=$(notdir $(src)) $返回 zookeeper.c main.cpp

$(suffix <names...>) #返回name中每个单词的后缀
$(basename <names...>) #返回name中每个单词的前缀
src=src/zookeeper.c main.cpp hacks
dst1=$(suffix $(src)) #返回.c .cpp
dst2=$(basename $(src)) #返回src/zookeeper main hacks

$(addsuffix <suffix>, <names...>) #给name中每个单词添加后缀
$(addprefix <prefix>, <names...>) #给name中每个单词添加前缀
src=src/zookeeper.c main.cpp hacks
dst1=$(addsuffix .tmp, src)
结果为src/zookeeper.c.tmp main.cpp.tmp hacks.tmp
dst1=$(addprefix /home/ll/, src)
结果为/home/ll/src/zookeeper.c /home/ll/main.cpp /home/ll/hacks

$(join <list1>, <list2>) #返回连接后的字符串
$(join a1 b1, a2 b2 c2) #返回a1a2 b1b2 c2
$(join a1 b1 c1, a2 b2) #返回a1a2 b1b2 c1
$(join a1 b1, a2 b2)    #返回a1a2 b1b2

$(foreach <var>, <list>, <text>)
把list中的单词逐一取出，放到var所指定的变量中，再执行text所包含的表达式。每一次循环，text会返回一个字符串。
names:=a b c d
files:=$(foreach n, $(names), $(n).o)
逐一取出names中的单词，放到变量n中。$(n).o每次计算出一个值，最后的结果为：a.o b.o c.o d.o。
var是个临时变量，出了foreach，就出了作用域。

#若condition成立，返回then-part计算的结果，否则返回空字符串
$(if <condition>, <then-part>) 

#若condition成立，返回then-part计算的结果，否则返回else-part计算的结果
$(if <condition>, <then-part>, <else-part>) 

$(origin <var>)  #返回var变量在什么地方定义的
返回值包括：
    undefined, 没有定义过
    default, 默认定义的
    environment, 环境变量
    file, 定义在Makefile中
    command line, 被命令行定义的
    override, 被override指示符重新定义的
    automatic，自动化变量

$(shell cmd)  # 执行shell命令
$(err <text>) # 输出错误信息text，Makefile停止执行
$(warning <text>) # 只输出一段警告
```

#### 1.10 make命令

包含多个目标的写法
```sh
.PHONY:all
all: prog1 prog2 prog3

prog1: c1.cpp c1.h
    cc -o prog1 c1.cpp c1.h
prog2: c2.cpp c2.h
    cc -o prog2 c2.cpp c2.h
prog3: c2.cpp c2.h c3.cpp c3.h
    cc -o prog2 c2.cpp c2.h c3.cpp c3.h
```
`make all`编译所有的目标;
`make prog2`编译生成prog2

一般支持下面几个命令
```sh
make all
make clean
make install
make tar

make -f Makefile.Linux # -f指定文件
make -B #认为所有的文件都被改动过，需要重新编译
# 指定包含makefile的搜索目标
make -I <dir1 dir2 ...> 或者 make --include-dir=<dir1 dir2 ...>
make -j <jobs> # 同时执行的jobs数。如make -j 4

# 默认命令变量
CC ，C语言编译程序，默认为"cc"
CXX，C++语言编译程序，默认为"g++"
RM，删除文件，默认为"rm -f"

# 默认参数变量
CFLAGS，C语言编译器参数
CPPFLAGS，C预处理器参数
CXXFLAGS，C++语言编译器参数
```

#### 1.11 自动化变量
```sh
$@，规则中的目标文件集
$%
$<
```

#### 1.12 示例
测试目录是否存在，不存在就创建一个, -d 表示directory
```sh
test -d '/home/ll/commlib/nginx-1.4.4' || mkdir -p '/home/ll/commlib/nginx-1.4.4'

# 测试文件是否存在，不存在就创建一个
test -f '/home/ll/commlib/u01/conf/mime.types' || cp conf/mime.types '/home/ll/commlib/u01/conf/mime.types'
test ! -f '/home/ll/commlib/u01/conf/nginx.conf' || mv '/home/ll/commlib/u01/conf/nginx.conf'  '/home/ll/commlib/u01/conf/nginx.conf.old'

# 生成一个ssl.h文件，依赖objs/Makefile
/home/ll/commlib/u01/openssl-1.0.1c/.openssl/include/openssl/ssl.h: objs/Makefile
cd /home/ll/commlib/u01/openssl-1.0.1c/ \
&& $(MAKE) clean \ #执行 /home/ll/commlib/u01/openssl-1.0.1c/下的Makefile
&& ./config --prefix=/home/ll/commlib/u01/openssl-1.0.1c/.openssl no-shared no-threads \
&& $(MAKE) \
&& $(MAKE) install LIBDIR=lib

```

