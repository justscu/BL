## mongodb

Mongo是开源的NoSql型数据库，在Mongo DB中每一条记录都是一个Document对象。文档(Document)是Mongo中数据的基本单元，类似于关系型数据库中的行(row)。每个文档都有一个特殊的键(_id)，它在文档所处的集合中是唯一的。
集合就是一组文档，集合可以看做没有模式的表。
多个集合组成数据库。
Mongo区分类型和大小写。

- 下载源码: `git clone https://github.com/mongodb/mongo`
- 编译方法：https://github.com/mongodb/mongo/blob/master/docs/building.md

**scons**是一个Python写的自动化构建工具，类似make，用来编译程序。make靠的是Makefile文件，scons靠的是SConstruct文件。在编译的时候，用scons命令即可。scons安装:
```sh
wget http://prdownloads.sourceforge.net/scons/scons-2.3.4.tar.gz
tar -zxvf scons-2.3.4.tar.gz 
cd scons-2.3.4
python setup.py install
```

- 编译: `scons all`，编译所有程序
- 安装: `scons --prefix=/opt/mongo install`
- 创建目录: `mkdir -p /data/db`
- 运行： `./mongod run &`，默认端口为27017;　或`./mongod run --dbpath=/xxx/path --port=33065`从33065端口启动程序
- 关闭： `./mongod --shutdown`

若出现下面的警告，可使用`numactl --interleave=all ./mongod run &`来启动。
```sh
** WARNING: You are running on a NUMA machine.
**          We suggest launching mongod like this to avoid performance problems:
**          numactl --interleave=all mongod [other options]
```


