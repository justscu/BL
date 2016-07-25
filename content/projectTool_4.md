
### 1 全局变量
在一个staticx.a文件中声明一个全局变量，由2个不同的动态库dym1.so, dym2.so中去使用这个全局变量。
这个全局变量会构造2次，但打印发现这个全局变量的地址一致。

```cpp
// staticx.h
class Data {
public:
	Data();
	~Data();
	void print();

private:
	int data_;
};

Data* get_data();

// staticx.cpp
#include <stdio.h>
#include "staticx.h"

Data::Data() {
    data_ = 0;
	fprintf(stdout, "ctor() this[%p]\n", this);
}
Data::~Data() {
	fprintf(stdout, "dtor() this[%p]\n", this);
}

void Data::print() {
	data_++;
	fprintf(stdout, "data[%d]\n", data_);
}


Data data; // 全局变量
Data* get_data() { return &data; }
```

libstaticx.a -> libdym1.so -> exe;

libstaticx.a -> libdym2.so -> exe;

运行结果
```
ctor() this[0x7fb9b2ccb060]
ctor() this[0x7fb9b2ccb060]
data[1]
data[2]
dtor() this[0x7fb9b2ccb060]
dtor() this[0x7fb9b2ccb060]
```
结论：全局变量被2个不同的x.so使用，调用了2次构造/析构函数。但两次构造/析构函数的地址一致。

### 2 静态变量
在一个staticx.cpp文件中声明一个static类型的局部变量，由2个不同的dym1.so, dym2.so库中去引用这个局部变量。
该static局部变量只会被构造一次。
```cpp
// 部分代码
Data* get_data() {
    static Data data; //静态局部变量
    return &data;
}
```

运行结果
```
ctor() this[0x7fcee4a8c090]
data[1]
data[2]
dtor() this[0x7fcee4a8c090]
```
结论：静态局部变量，被2个不同的x.so文件使用，调用构造/析构函数一次。

代码位置`src/dym.test`
