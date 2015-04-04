## Google Protocol Buffer

[google protocol buffer](https://code.google.com/p/protobuf/)是一种高效的结构化存储数据格式，用来将结构化数据串行化和反串行化。
常用于分布式应用之间的数据通信或者异构环境下的数据交换，串行化后的数据简单、高效、体积小。

### 1 C++版编译器
安装方法
```sh
git clone https://github.com/google/protobuf.git
cd protobuf/
./autogen.sh
./configure
make
make check
sudo make install

# 或者使用apt进行安装
sudo apt-get install protobuf-compiler 
```

新建pb文件
```cpp
// addressbook.proto文件
package tutorial;   // package的名字叫做tutorial

message Person {
   required string name = 1;
   required int32 id = 2;
   optional string email = 3;

   enum PhoneType {
     MOBILE = 0;
     HOME = 1;
     WORK = 2;
   }

   message PhoneNumber {
     required string number = 1;
     optional PhoneType type = 2 [default = HOME];
   }

   repeated PhoneNumber phone = 4;
}

message AddressBook {
   repeated Person person = 1;
}
```

生成xx.pb.cc和xx.pb.h文件
```sh
# 运行命令
protoc --cpp_out=./ addressbook.proto
# 若成功的话，会在./目录下，生成address.pb.cc和address.pb.h文件
```

常用的命令
```sh
protoc --proto_path=src/ --cpp_out=build/gen xx1.proto xx2.proto
# protoc 为工具命令
# proto_path 为import文件的路径
# cpp_out 表明要生成 xx.pb.cc和xx.pb.h文件
# xx1.proto为源文件名
```

import关键字
```sh
# 在xx.proto文件中，可以使用import关键字引入在其他.proto文件中定义的消息。
import common.header;
message Msg{ 
    required common.info_header header = 1; #common.info_header定义在common.header包内
    required string yourPrivateData = 2; 
}
```

串行化和反串行化
```cpp
// 该函数以字符串的形式返回串行化的结果，如msg_content.SerializeAsString()；在发送消息前，使用该函数
string MessageLite::SerializeAsString() const；

// 该函数将反串行化的结果存放到data中；收到消息时，使用该函数
bool MessageLite::ParseFromString(const string& data)；
```

### 2 Golang版编译器
安装方法
```sh
# 第一种方法：使用源码安装
cd $GOPATH/src/ & mkdir p & cd p/
hg clone https://code.google.com/p/goprotobuf/
# 下载的文件，主要包含2个主要部分：
#（A）goprotobuf/proto,运行时库，用来marshaling & unmarshaling，即编解码
#（B）goprotobuf/protoc-gen-go，protocol编译器的插件，用于生成go源文件

cd goprotobuf/proto & go build & go install
# 会生成$GOPATH/pkg/linux_386/code.google.com/p/goprotobuf/proto.a文件

cd goprotobuf/protoc-gen-go & go build & go install
# 在$GOPATH/pkg/linux_386/code.google.com/p/goprotobuf/protoc-gen-go/下，
# 会生成descriptor.a、generator.a、plugin.a文件
# 在$GOROOT/bin下会生成protoc-gen-go可执行文件

# 第二种方法：使用go get安装
go get -u code.google.com/p/goprotobuf/{proto,protoc-gen-go}
```

生成xx.pb.go文件
```sh
protoc --proto_path=src/ --go_out=build/gen  xx1.proto xx2.proto
# protoc 为工具命令
# proto_path 为import文件的路径
# go_out 表明要生成 xx.pb.go文件
# xx1.proto为源文件名
```

串行化和反串行化
```go
import proto "code.google.com/p/goprotobuf/proto"

msg := &tutorial.Person {
    Name: proto.String("dzh"),
    Id: proto.Int32(110)
    Email:proto.String("justscu@163.com")
}
buffer, err := proto.Marshal(msg) // 串行化

msg2 := &tutorial.Person {}
err = proto.Unmarshal(buffer, msg2) // 反串行化
```

### 3 常见错误及解决方法
#### 3.1 libprotoc.so.9错误
在运行protoc命令时，报下面错误 
**protoc: error while loading shared libraries: libprotoc.so.9: cannot open shared object file: No such file or directory**

很明显是没有找到libprotoc.so.9文件。使用查找命令`whereis libprotoc.so.9`:
```sh
libprotoc.so: /usr/local/lib/libprotoc.so /usr/local/lib/libprotoc.so.9
```
在/usr/local/lib目录下，有该.so文件。将/usr/local/lib目录加入到.so文件的搜索路径下。然后执行`sudo ldconfig`。
原因: 进行默认安装C++版本编译器时，libprotoc.so.9安装在`/usr//usr/local/lib/`下，但该路径并不是.so文件的默认查找路径。

#### 3.2 在Golang环境中使用生成的xx.pb.go文件
还是以上面的"addressbook.proto"文件为例，生成的文件为"addressbook.pb.go"。
在addressbook.pb.go中，有句`package example`，表明是导出包，包的名字是`example`。

创建src/example目录，并将"addressbook.pb.go"文件拷贝到该目录下。
在主文件中（main.go），用`import "example"`语句进行导入。

### 4 Makefile的写法
```sh
# To build a package from test.proto and some other Go files, write a Makefile like this:

include $(GOROOT)/src/Make.$(GOARCH)

TARG=path/to/example
GOFILES=\
test.pb.go\
other.go

include $(GOROOT)/src/Make.pkg
include $(GOROOT)/src/pkg/code.google.com/p/goprotobuf/Make.protobuf
```

### 5 protocol buffer编码规则
pb采用kv对（key-value pairs）的方式对消息进行编码。对于optional字段，当没有赋值时，串行化的序列中，没有该kv对。

#### 5.1 Varint编码
pb对无符号数，采用Varint编码。

pb采用7位编码，最高位是标志位。当最高位为1时，表示后续字节是该字段的，当最高位为0时，表示该字段的最后一个字节。同时，pb使用little-edition编码（即先存低位）。

但这样编码，存在一个问题，当数字不大时，使用的字节比较少（最少1个字节）；当数字很大时，使用的字节比较多（最多5字节）；固定编码，都需要四个字节。（统计发现，大数出现的几率低） 
[定长编码和变长编码代码](https://github.com/justscu/varintcode)
![network_3_1]()

#### 5.2 Zig-Zag编码
pb对有符号数，先采用Zig-Zag编码，然后再采用Varint编码。
若负数也采用Varint编码的话，会占用5个字节，pb对负数，采用Zig-Zag编码方式。Zig-Zag编码用无符号数来表示有符号数字，正数和负数交错。

就是把（-1）**1**000 0001变成0000 001**1**，注意最后一个1是符号位。那么如果是个int32的话，**1**000 0000 0000 0000 0000 0000 0000 0001 就变成了000 0000 0000 0000 0000 0000 0000 0001**1**。 

用位运算来表示把一个负数转换成zig-zag编码，int32是：`(n << 1) ^ (n >> 31)`；int64是：`(n << 1) ^ (n >> 63) `

#### 5.3 Varchar编码
对字符串，使用Varchar编码（类似于数据库编码方式）。用Varint表示字符串的长度，其后跟具体的字符串。

### 6 优点
- 比XML 更小、更快、更简单。串行化后以二进制方式存储。
- 当增减结构中的字段时，可以很好的向后兼容。
- 可以支持嵌套的Message。

### 7 不足
- 功能简单，无法表示复杂的概念
