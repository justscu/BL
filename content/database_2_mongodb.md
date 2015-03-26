## mongodb

mongodb是开源的NoSql型数据库，在Mongo DB中每一条记录都是一个Document对象。

下载源码: git clone https://github.com/mongodb/mongo

编译方法：https://github.com/mongodb/mongo/blob/master/docs/building.md

**scons**是一个Python写的自动化构建工具，类似make，用来编译程序。make靠的是Makefile文件，scons靠的是SConstruct文件。在编译的时候，用scons命令即可。

scons安装
```sh
wget http://prdownloads.sourceforge.net/scons/scons-2.3.4.tar.gz
tar -zxvf scons-2.3.4.tar.gz 
cd scons-2.3.4
python setup.py install
```

编译mongo命令：scons
创建目录： mkdir -p /data/db, 
运行：./mongod run &，默认端口为27017
	　./mongod run --dbpath=/xxx/path --port=33065
关闭：./mongod --shutdown
若出现下面的警告，使用`numactl --interleave=all ./mongod run &`来启动。
```sh
** WARNING: You are running on a NUMA machine.
**          We suggest launching mongod like this to avoid performance problems:
**          numactl --interleave=all mongod [other options]
```


