
#### 环境变量
可以在命令行中设置环境变量
```sh
# 查看环境变量(在命令行中)
env　| grep "LD_LIBRARY_PATH"
echo $LD_LIBRARY_PATH

# 添加环境变量
	# [临时有效](在命令行中)
export $LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/new/path/to/lib
	# [长期有效] 修改`~/.bashrc`文件，在末尾添加
export $LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/new/path/to/lib;  source ~/.bashrc

# 取消环境变量[unset](在命令行中)
unset $LD_LIBRARY_PATH

# 只读环境变量
readonly $LD_LIBRARY_PATH
```

或者在文件中修改环境变量，然后用`source`命令
```sh
# env.rc文件

export PKG_CONFIG_PATH=/home/ll/project/commlib/bin/zeromq-4.0.4/lib/pkgconfig/
export ZK_LIST="10.15.43.49:2181"
# unset ZK_LIST
```
然后调用`source env.rc`命令使其生效。

#### 头文件路径顺序
gcc/g++头文件的路径顺序
*	(1)`gcc -I xxx/path`中包含的头文件路径
*	(2)gcc环境变量来搜索头文件路径`C_INCLUDE_PATH、CPLUS_INCLUDE_PATH、OBJC_INCLUDE_PATH`
*	(3)gcc库文件的路径: "\`gcc -print-prog-name=cc1plus\` -v" 或 "\`g++ -print-prog-name=cc1plus\` -v"中的路径 

#### lib库路径顺序
gcc/g++ lib库搜索路径
*	(1)`gcc -L xxx/path`指定的路径
*	(2)环境变量`LD_LIBRARY_PATH`指定的动态库搜索路径，`LD_LIBRARY_PATH`用来处理非标准路经的共享库
*	(3)配置文件`/etc/ld.so.conf`中指定的动态库搜索路径
*	(4)默认的动态库搜索路径`/lib`
*	(5)默认的动态库搜索路径`/usr/lib`

当有x.a和x.so文件出现在同一目录时，优先使用x.so文件，可以使用`-static`来优先使用静态库。

#### 添加lib库路径的方法
linux系统把`/lib`、`/usr/lib`作为库的默认搜索路径。当x.a或x.so在这2个库中时，不需要设置搜索路径，就可以直接使用。

当有新的x.a/x.so加入(以"libactivemq-cpp.so"为例)，且不在上述3个库中，可以在`/etc/ld.so.conf.d`目录中添加库的搜索路径：
*	(1)在`/etc/ld.so.conf.d/`目录下添加"libactivemq-cpp.so.conf"文件
*	(2)在"libactivemq-cpp.so.con"f中加上库的绝对路径"/home/ll/u01/activemq-cpp/lib/"
*	(3)执行`ldconfig`
*	(4)｀ldconfig -p | grep libactivemq-cpp.so｀，查看是否添加成功

也可以采用下面方法(假设"pushserver"为可执行程序，用到了activemq的库)：
```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ll/u01/activemq-cpp/lib/; ./pushserver
# 或
env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ll/u01/activemq-cpp/lib/ ./pushserver
```
