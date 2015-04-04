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
```pb
# addressbook.proto文件
package tutorial;  #package的名字叫做tutorial

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
string MessageLite::SerializeAsString() const；
// 该函数以字符串的形式返回串行化的结果，如msg_content.SerializeAsString()；在发送消息前，使用该函数 
bool MessageLite::ParseFromString(const string& data)；
// 该函数将反串行化的结果存放到data中；收到消息时，使用该函数
```

### 2 Golang版编译器

### 3 常见错误及解决方法

### 4 Makefile的写法

### 5 protocol buffer编码规则
