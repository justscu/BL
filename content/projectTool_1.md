### 1 gcc命令

GCC是一套由GUN开发的支持多种编程语言的编译器。支持C、C++、GO等。

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
```
GCC默认动态库优先于静态库（先找.so，再找.a），若在编译的时候想优先使用静态库，需要加上`-static`。




### 2 CMake

cmake是跨平台的编译工具，用于生成Makefile文件。cmake依赖CMakeList.txt文件。

需要注意的是，`TARGET_LINK_LIBRARIES`后面的依赖，是有先后顺序的，如`TARGET_LINK_LIBRARIES(pushTest libgtest.a pthread)`, libgtest.a依赖pthread库，顺序写反了，会导致link失败。

#### 2.1 生成可执行文件
```sh
cmake_minimum_required(VERSION 2.8)

# 工程名字，PushProxyProject
PROJECT(PushProxyProject)

# 添加编译选项
ADD_DEFINITIONS(-g -W -Wall -DTIXML_USE_STL -Wno-deprecated -DTHREADED ${CMAKE_CXX_FLAGS})

# 添加源文件路径
AUX_SOURCE_DIRECTORY(./ SRC_LIST)
AUX_SOURCE_DIRECTORY(./zk/ SRC_LIST)
# 也可以使用这种方法添加源文件
ADD_SUBDIRECTORY(network)

# 添加头文件路径
INCLUDE_DIRECTORIES(
./
../commlib/json/
../commlib/apr-1.4.6/include/apr-1/
../commlib/leveldb-1.9.0/
../commlib/leveldb-1.9.0/include/
../commlib/zookeeper-3.4.5/include/zookeeper/
)

# 设置输出文件路径
SET(EXECUTABLE_OUTPUT_PATH ../bin)

# 添加库文件路径
LINK_DIRECTORIES(
/usr/local/lib/
../commlib/activemq-cpp-3.5.0/lib/
../commlib/apr-1.4.6/lib/
../commlib/zookeeper-3.4.5/lib/
../commlib/leveldb-1.9.0/
)

# 生成可执行文件PushProxy，依赖于SRC_LIST文件
ADD_EXECUTABLE(PushProxy ${SRC_LIST})

# 添加库文件，表明PushProxy依赖后面的pthread, json ...等文件
TARGET_LINK_LIBRARIES(PushProxy pthread json activemq-cpp apr-1 zookeeper_mt leveldb)
```

#### 2.2 生成库文件
```sh
cmake_minimum_required(VERSION 2.8)

# 工程名字Project2
PROJECT(Project2)

# 添加子目录 ./src/network
ADD_SUBDIRECTORY(./src/network)

# 添加子目录 xml
AUX_SOURCE_DIRECTORY(./xml SRC_LIST)

# 使用-fPIC，生成动态库(生成位置无关的代码)
SET(CMAKE_CXX_FLAGS "-fPIC")

# 设置输出文件路径
SET(LIBRARY_OUTPUT_PATH ../lib)

# 添加编译选项
ADD_DEFINITIONS(-g -W -Wall -DAC_HAS_TRACE -DTIXML_USE_STL ${CMAKE_CXX_FLAGS})

# 头文件路径
INCLUDE_DIRECTORIES(
./
../include/)

# link文件路径
LINK_DIRECTORIES()

####### (1)只生成静态库文件 libkvdb.a #######
ADD_LIBRARY(kvdb STATIC ${SRC_LIST})

####### (2)只生成动态库文件 libkvdb.so #######
ADD_LIBRARY(kvdb SHARED ${SRC_LIST})

####### (3)同时构建动态库和静态库 #######
# 当同时写上下面两句时，不会生成静态库，因为target不能够重名[即kvdb]
ADD_LIBRARY(kvdb STATIC ${SRC_LIST})
ADD_LIBRARY(kvdb SHARED ${SRC_LIST})
# 可以采用下面的方法，先生成 libkvdb_2.so，然后将其改名为 libkvdb.so
ADD_LIBRARY(kvdb STATIC ${SRC_LIST})
ADD_LIBRARY(kvdb_2 SHARED ${SRC_LIST})
SET_TARGET_PROPERTIES(kvdb_2 PROPERTIES OUTPUT_NAME "kvdb") #修改输出名字

####### (4)为生成的动态库添加版本 #######
ADD_LIBRARY(kvdb_2 SHARED ${SRC_LIST})
SET_TARGET_PROPERTIES(kvdb_2 PROPERTIES OUTPUT_NAME "kvdb") #修改输出名字
SET_TARGET_PROPERTIES(kvdb_2 PROPERTIES VERSION 1.2 SOVERSION 1)
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

### 3 configure

configure是一个shell脚本，可以根据系统参数和环境配置，生成Makefile文件。
可以在执行`./configure`命令时，传递不同的参数，可以生成不同的Makefile文件。
`./configure --help`，可以查看支持哪些参数。

如：./configure CXXFLAGS="-I/home/ll/u01/libunwind-0.99/include -fpermissive -g" LDFLAGS="-L/home/ll/u01/libunwind-0.99/lib" LIBS="-lunwind" --prefix=/home/ll/u01/google-perftools-1.10 --host=x86_64

下面是nginx源码中的configure脚本。
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
