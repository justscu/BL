
基本操作
====

查看gcc版本: `gcc -v`;
查看glibc版本: `ldd --version`;
查看需要的动态库: `ldd xx`

查看编译选项:
```
[ll@] readelf -p .comment xxx_bin
# 包含额外的.a文件，分开编译的，可见多个GCC信息
String dump of section '.comment':
  [     0]  GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-39)
  [    2d]  GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)
  [    5a]  GCC: (GNU) 10.3.1

[ll@] readelf -wi xxx_bin | grep -E "(DW_AT_producer|DW_AT_name|DW_AT_comp_dir)"
```

需要加载哪些动态库: `ldd xxx_bin`;
系统中包含哪些动态库: `ldconfig -p` 


gcc命令
====

GCC是一套由GUN开发的支持多种编程语言的编译器。支持C、C++、GO等.

`o`区分大小写, Optimization(优化) output(输出文件).

```sh
-o，生成目标文件main，如 gcc main.c -o main
-Wall，打开所有警告，如 gcc -Wall main.c -o main
-Werror，将警告当做错误来处理
-E，将预编译结果，输出到main.i文件中(去掉注释、宏展开、include替换)，如 gcc -E main.c > main.i
-S，将汇编的结果，输出到main.s文件中，如 gcc -S main.c > main.s
-C，只编译，不链接，生成main.o文件，如 gcc -C main.c
-I，表示头文件的路径，如gcc foo.c -I /home/ll/work/include/ -o foo
-L，表示库文件的路径，如gcc foo.c -L /home/ll/commlib/ -ljson -o foo
-l，表示使用库，使用libjson.so文件，如 gcc main.c -o main -ljson
-Ox，代码优化，其中-O0，表示不优化；-O1到-O3代表不同级别的优化
-V，打印编译过程中的调试信息，如 gcc -V

-ansi，支持ISO C89风格
-funsigned-char，将char类型解释为unsigned char类型。通过该编译选项，char会被当做unsigned char处理
-fsigned-char，将unsigned char类型解释为char类型

-D，编译时，声明了宏。如-DCCAV，相当于在代码中有这样一句：#define CCAV
-g，增加调试信息
-ggdb3，在gdb调试时，可以查看宏的信息

-fPIC，创建动态库时，创建无关联的地址信息代码
	gcc -C -fPIC Cfile.c
	gcc -shared -o libCfile.so Cfile.o

-std=c++11，采用C++11标准进行编译。gcc版本要>=4.8.1。若系统中同时有多个gcc版本，可以指定特定版本。vim ~/.bashrc，加入:
	export CC=/xxx/bin/gcc
	export CPP=/xxx/bin/cpp
	export CXX=/xxx/bin/c++
	
```
GCC默认动态库优先于静态库（先找.so，再找.a），若在编译的时候想优先使用静态库，需要加上`-static`。


makefile
====

1 编译规则
=====
- 若工程没有编译过，则编译所有cpp文件，并链接生成目标文件；
- 若改变了某些cpp文件，则只编译这些改变过的cpp文件，并链接生成目标文件；
- 若改变了某些.h文件，则编译包含这些.h文件的cpp文件，并链接生成目标文件；

2 格式
=====
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

3 Makefile的命名规则
=====
可以使用makefile，Makefile，GUNmakefile，Make.Linux，Make.Solaris，xxx.mk等。

make命令会在当前目录下依次查找`GUNmakefile->makefile -> Makefile`，也可以使用`make -f Make.Linux`来指定相关的文件。

4 包含其他Makefile文件 -- include
=====
```sh
include foo.make a.mk ./zk/*.mk
```
注意：include前不能有Tab; ./zk/*.mk表示zk下的.mk文件; 支持通配符

5 通配规则
=====
```sh
BINDIR=~/goproject/bin 
ETCDIR=~/goproject/etc
rm -rf *.o #表示要删除所有的.o文件
```

6 嵌套Makefile
=====
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

7 变量
=====
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

8 条件判断
=====
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

9 函数
=====
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

10 make命令
=====

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

一般支持下面几个命令：
```sh
make all
make clean
make install
make tar
```
```sh
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

11 自动化变量
=====

```sh
$@，规则中的目标文件集
$%
$<
```

12 示例
=====

测试目录是否存在，不存在就创建一个, -d 表示directory
```sh
test -d '/home/ll/commlib/nginx-1.4.4' || mkdir -p '/home/ll/commlib/nginx-1.4.4'

# 测试文件是否存在，不存在就创建一个
test -f '/home/ll/commlib/u01/conf/mime.types' \
|| cp conf/mime.types '/home/ll/commlib/u01/conf/mime.types'
test ! -f '/home/ll/commlib/u01/conf/nginx.conf' \
|| mv '/home/ll/commlib/u01/conf/nginx.conf'  '/home/ll/commlib/u01/conf/nginx.conf.old'

# 生成一个ssl.h文件，依赖objs/Makefile
/home/ll/commlib/u01/openssl-1.0.1c/.openssl/include/openssl/ssl.h: objs/Makefile
cd /home/ll/commlib/u01/openssl-1.0.1c/ \
&& $(MAKE) clean \ #执行 /home/ll/commlib/u01/openssl-1.0.1c/下的Makefile
&& ./config --prefix=/home/ll/commlib/u01/openssl-1.0.1c/.openssl no-shared no-threads \
&& $(MAKE) \
&& $(MAKE) install LIBDIR=lib

```

cmake
====
`cmake`是跨平台的编译工具，根据`CMakeLists.txt`生成`Makefile`文件.

- 基础配置
```
    cmake_minimum_required(VERSION 2.8), 需要最低的cmake版本
    project(PushProxyProject), 定义项目名称
```

- 变量定义

cmake包含一些内置的变量，如
```
    project_source_dir, 工程的根目录
    project_binary_dir, 运行cmake的目录
    project_name，通过project(NAME)设置的名字
    cmake_current_source_dir, 当前处理CMakeLists.txt所在的路径
    cmake_current_binary_dir, target编译目录
    cmake_current_list_dir, CMakeLists.txt的完整路径
    executable_output_path, 重新定义目标文件的存放目录
    library_output_path, 重新定义lib文件的存放目录
```
定义变量
```
    set(SRC_LIST main.cpp abc.cpp), 使用set定义变量SRC_LIST
    set(SRC_LIST ${SRC_LIST} test.cpp), 使用set向变量SRC_LIST中追加test.cpp文件
```

- 编译选项与定义
```
    set(CMAKE_CXX_STANDARD 17), 指定C++标准
    add_compile_options(-O3 -Wall -std=c++11), 设置全局编译优化和警告
    add_compile_definitions(USE_DB)，定义宏，等价于ADD_DEFINITIONS
    add_definitions(USE_DB), 定义宏
```

- 头文件与库文件管理
```
    include_directories(xx/include), 添加头文件搜索路径
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/third/include/              
    )
    link_directories(xx/lib), 添加库文件搜索路径
```

- 源文件管理
```
    set(SRC_FILES main.cpp utils.cpp), 手动列出源文件
    file(GLOB SRC_FILES "src/*.cpp"), 自动扫描特定目录下的所有源文件
    aux_source_directory(/path/src SRC_FILES), 将`/path/src`目录下所有的cpp文件，加入到变量SRC_FILES中
```

- 构建目标
```
    add_executable(PushProxy ${SRC_FILES} main.cpp), 编译生成可执行程序PushProxy
    add_library(push static ${SRC_FILES}), 编译生成静态库 push.a
    add_library(push shared ${SRC_FILES}), 编译生成动态库 push.so
```

- 连接依赖
```
    target_link_libraries(PushProxy libgtest.a pthread), 依赖libgtest.a和libpthread.so, 依赖有先后顺序.
                                    libgtest.a依赖pthread库, 顺序写反了会导致link失败
```

- 示例: 可执行文件
```sh
# 指定cmake的最小版本
cmake_minimum_required(VERSION 2.8)

# 工程名字
project(PushProxyProject)

# 添加编译选项
add_compile_options(-g -Wall -Wno-deprecated)
# 添加宏定义
add_compile_definitions(TIXML_USE_STL THREADED)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/commlib/json/
    ${PROJECT_SOURCE_DIR}/commlib/apr-1.4.6/include/apr-1/
    ${PROJECT_SOURCE_DIR}/commlib/leveldb-1.9.0/include/
    ${PROJECT_SOURCE_DIR}/commlib/zookeeper-3.4.5/include/zookeeper/
)

# 添加源文件
file(GLOB SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/zk/*.cpp
)

# 也可以使用这种方法添加源文件
ADD_SUBDIRECTORY(network)



# 设置输出文件路径
set(EXECUTABLE_OUTPUT_PATH ../bin)

# 添加库文件路径
link_directories(
    /usr/local/lib/
    ${PROJECT_SOURCE_DIR}/commlib/activemq-cpp-3.5.0/lib/
    ${PROJECT_SOURCE_DIR}/commlib/apr-1.4.6/lib/
    ${PROJECT_SOURCE_DIR}/commlib/zookeeper-3.4.5/lib/
    ${PROJECT_SOURCE_DIR}/commlib/leveldb-1.9.0/
)

# 生成可执行文件PushProxy，依赖于SRC_LIST文件
add_executable(PushProxy ${SRC_LIST} main.cpp)

# 添加库文件，表明PushProxy依赖后面的pthread, json ...等文件
target_link_libraries(PushProxy pthread json activemq-cpp apr-1 zookeeper_mt leveldb)

```

- 示例: 库文件
```sh
cmake_minimum_required(VERSION 2.8)

# 工程名字
project(Project2)

# 打印消息
message(${PROJECT_SOURCE_DIR})
set(CMAKE_VERBOSE_MAKEFILEON ON)

# 添加编译选项
add_compile_options(-g -O3 -Wall -std=c++11 -fPIC -Wno-deprecated)
# 添加宏定义
add_compile_definitions(TIXML_USE_STL THREADED)

# 添加子目录 ./src/network
# ADD_SUBDIRECTORY(./src/network)

# 添加 .h
include_directories(
    ${PROJECT_SOURCE_DIR}/fmt/include/
    ${PROJECT_SOURCE_DIR}/utils/
    ${PROJECT_SOURCE_DIR}/log/
    ${PROJECT_SOURCE_DIR}/tinyini/
)

# 添加 .cpp
aux_source_directory(${PROJECT_SOURCE_DIR}/utils/   SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/log/     SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/tinyini/ SRC_LIST)

#拷贝头文件到指定目录
file(COPY ${PROJECT_SOURCE_DIR}/utils/   DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")
file(COPY ${PROJECT_SOURCE_DIR}/log/     DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")
file(COPY ${PROJECT_SOURCE_DIR}/tinyini/ DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")

# 添加link文件路径
link_directories(
    /usr/local/lib/
    ${PROJECT_SOURCE_DIR}/fmt/debug/ #使用fmt库
)

# 设置输出文件路径
set(LIBRARY_OUTPUT_PATH ../lib)


####### (1)只生成静态库文件 libkvdb.a #######
ADD_LIBRARY(kvdb STATIC ${SRC_LIST})
target_link_libraries(kvdb
"-Wl, --whole-archive" # 之后的库使用--whole-archive
fmt
"-Wl, --no-whole-archive" # 之后的库不使用--whole-archive
)

####### (2)只生成动态库文件 libkvdb.so #######
add_library(kvdb SHARED fmt ${SRC_LIST})

####### (3)同时构建动态库和静态库 #######
# 当同时写上下面两句时，不会生成静态库，因为target不能够重名[即kvdb]
add_library(kvdb STATIC ${SRC_LIST})
add_library(kvdb SHARED ${SRC_LIST})
# 可以采用下面的方法，先生成 libkvdb_2.so，然后将其改名为 libkvdb.so
add_library(kvdb STATIC ${SRC_LIST})
add_library(kvdb_2 SHARED ${SRC_LIST})
set_target_properties(kvdb_2 PROPERTIES OUTPUT_NAME "kvdb") #修改输出名字

####### (4)为生成的动态库添加版本 #######
add_library(kvdb_2 SHARED ${SRC_LIST})
set_target_properties(kvdb_2 PROPERTIES OUTPUT_NAME "kvdb") #修改输出名字
set_target_properties(kvdb_2 PROPERTIES VERSION 1.2 SOVERSION 1)

```

最终生成的结果为
```sh
-rw-rw-r-- 1 ll ll 3728154 10-15 16:04 lib/libkvdbl.a
lrwxrwxrwx 1 ll ll 15 10-15 16:06 lib/libkvdb.so -> libkvdb.so.1
lrwxrwxrwx 1 ll ll 17 10-15 16:06 lib/libkvdb.so.1 -> libkvdb.so.1.2
-rwxrwxr-x 1 ll ll 2352419 10-15 16:06 lib/libkvdb.so.1.2
```

.a/.so区别
* (1).a文件只是简单多个.o文件的集合。在使用静态库时，链接器从.a文件中复制函数和数据，并将其拷贝到最终的可执行程序(拷贝可执行代码和符号表)。
使用.a文件，可执行程序会比较大。但在发行的时候，只需要发行可执行程序即可。当.a文件需要更新时，需要重新编译发行可执行程序。
* (2)在使用动态库时，链接器只拷贝了符号表，没有拷贝可执行代码。发布的可执行文件较小，但需要同时发布.so文件。在启动可执行文件时，需要加载.so文件，速度会慢一些。
* (3)在编译可执行文件时，默认使用.so文件。`-static`强制使用.a文件，`-shared`强制使用.so文件。
* (4)在生成.so文件时，最好加上gcc的`-fPIC`编译参数，这样会生成位置无关的代码(Position-Independent Code)。生成的代码中，没有绝对地址，全部使用相对地址，故而代码可以被加载器加载到内存的任意位置，都可以正确的执行。
总是用`-fPIC`来生成x.so，没有必要使用`-fPIC`来生成x.a。
* (5)若在生成.so文件时不使用`-fPIC`, 则可执行文件在加载.so中的函数时，需要重新定位并修改.so中代码段的内容。这会造成每个使用该.so文件的程序都需要有一份该.so文件的拷贝（拷贝到什么位置取决于将该.so文件映射到内存的什么位置），这就不是共享内存了。
* (6) 使用`flie`命令，可以查看文件的格式(.a, .so, or exe)。



configure
====

`configure`是一个shell脚本，可以根据系统参数和环境配置，生成Makefile文件。
可以在执行`./configure`命令时，传递不同的参数，可以生成不同的Makefile文件。
`./configure --help`，可以查看支持哪些参数。

如：./configure CXXFLAGS="-I/home/ll/u01/libunwind-0.99/include -fpermissive -g" LDFLAGS="-L/home/ll/u01/libunwind-0.99/lib" LIBS="-lunwind" --prefix=/home/ll/u01/google-perftools-1.10 --host=x86_64

下面是`nginx`源码中的`configure`脚本。
```sh
#!/bin/sh     #这是一个脚本

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


LC_ALL=C
export LC_ALL

. auto/options    # .命令是linux内部命令，从指定的文件中读入所有命令语句并在当前进程中执行
. auto/init
. auto/sources

test -d $NGX_OBJS || mkdir $NGX_OBJS   # NGX_OBJS=objs，在auto/options文件中定义的；不存在的话，创建该文件

echo > $NGX_AUTO_HEADERS_H    # NGX_AUTO_HEADERS_H=$NGX_OBJS/ngx_auto_headers.h，在auto/init中定义
echo > $NGX_AUTOCONF_ERR

# NGX_AUTO_CONFIG_H=$NGX_OBJS/ngx_auto_config.h，在auto/init中定义 
echo "#define NGX_CONFIGURE \"$NGX_CONFIGURE\"" > $NGX_AUTO_CONFIG_H


# 若用户设置NGX_DEBUG=YES，则读入 auto/have中所有语句，并在当前进程中执行
if [ $NGX_DEBUG = YES ]; then
    have=NGX_DEBUG . auto/have
fi


if test -z "$NGX_PLATFORM"; then  # test -z，判断 $NGX_PLATFORM 是否为空
    echo "checking for OS"

    NGX_SYSTEM=`uname -s 2>/dev/null`
    NGX_RELEASE=`uname -r 2>/dev/null`
    NGX_MACHINE=`uname -m 2>/dev/null`

    echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

    NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";

    case "$NGX_SYSTEM" in
        MINGW32_*)
            NGX_PLATFORM=win32
        ;;
    esac

else
    echo "building for $NGX_PLATFORM"
    NGX_SYSTEM=$NGX_PLATFORM
fi

. auto/cc/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/headers
fi

. auto/os/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/unix
fi

. auto/modules
. auto/lib/conf

case ".$NGX_PREFIX" in
    .)
        NGX_PREFIX=${NGX_PREFIX:-/usr/local/nginx}
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;

    .!)
        NGX_PREFIX=
    ;;

    *)
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;
esac

if [ ".$NGX_CONF_PREFIX" != "." ]; then
    have=NGX_CONF_PREFIX value="\"$NGX_CONF_PREFIX/\"" . auto/define
fi

have=NGX_SBIN_PATH value="\"$NGX_SBIN_PATH\"" . auto/define
have=NGX_CONF_PATH value="\"$NGX_CONF_PATH\"" . auto/define
have=NGX_PID_PATH value="\"$NGX_PID_PATH\"" . auto/define
have=NGX_LOCK_PATH value="\"$NGX_LOCK_PATH\"" . auto/define
have=NGX_ERROR_LOG_PATH value="\"$NGX_ERROR_LOG_PATH\"" . auto/define

have=NGX_HTTP_LOG_PATH value="\"$NGX_HTTP_LOG_PATH\"" . auto/define
have=NGX_HTTP_CLIENT_TEMP_PATH value="\"$NGX_HTTP_CLIENT_TEMP_PATH\""
. auto/define
have=NGX_HTTP_PROXY_TEMP_PATH value="\"$NGX_HTTP_PROXY_TEMP_PATH\""
. auto/define
have=NGX_HTTP_FASTCGI_TEMP_PATH value="\"$NGX_HTTP_FASTCGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_UWSGI_TEMP_PATH value="\"$NGX_HTTP_UWSGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_SCGI_TEMP_PATH value="\"$NGX_HTTP_SCGI_TEMP_PATH\""
. auto/define

. auto/make   #生成 Makefile文件
. auto/lib/make
. auto/install

# STUB
. auto/stubs

have=NGX_USER value="\"$NGX_USER\"" . auto/define
have=NGX_GROUP value="\"$NGX_GROUP\"" . auto/define

. auto/summary
```
