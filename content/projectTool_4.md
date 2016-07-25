## 在动态库中使用全局变量和静态变量

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
结论：全局变量被2个不同的x.so使用，调用了2次构造/析构函数。但打印出来的变量地址一致。

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

### 

代码位置`src/dym.test/`目录。

### 3 使用编译选项
Linux下GCC有个`-fvisibility`选项来控制函数和变量的visibility属性。
-fvisibility=default|internal|hidden|protected。

函数的visibility属性默认为public，在编译选项中加入`-fvisibility=hidden`，会将所有默认的public属性改为hidden。
设置了`-fvisibility=hidden`后，只有设置了`__attribute__((visibility("default")))`的函数才对外可见，效果等同于windows下的`__declspec(dllexport)`。

```cpp
__attribute__((visibility("hidden"))) Data data;
__attribute__((visibility("hidden"))) Data* get_data() { return &data; }
```
给staticx加上`-fvisibility=hidden`编译选项后的效果：
```
ctor() this[0x7f22bf0fd058]
ctor() this[0x7f22bf2ff048]
data[1]
data[1]
dtor() this[0x7f22bf2ff048]
dtor() this[0x7f22bf0fd058]
```

使用`readelf -s xx.so`命令查看xx.so文件属性。defult即为可见，hidden即为不可见。

注意，也可以对全局变量、静态变量、以及使用了这些变量的函数，使用`__attribute__((visibility("hidden"))) `属性，从而使其它x.so文件不可见。
