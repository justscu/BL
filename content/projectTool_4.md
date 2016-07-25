RedHat7.2，macOS10.11对多个动态库中有相同的`全局变量，静态变量`的处理方式不一致。

staticx(全局变量) -> dym1 -> exe,
staticx(全局变量) -> dym2 -> exe

staticx文件内容，编译后，生成静态库 libstaticx.a
```cpp
// static.h文件内容
class Data {                                                                    
public:
    Data();
    ~Data();
    void print();

private:
    int data_;
};

extern Data g_data;

// static.cpp文件内容
#include <stdio.h>                                                              
#include "static.h"

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

class Data g_data;
```

动态库dym1文件内容，编译后，生成 libdym1.so
```cpp
// dym1.h文件内容
#include <iostream>

extern "C" {
	void dym1_f1();
}

// dym1.cpp文件内容
#include "dym1.h"
#include "../../static/static.h"

void dym1_f1() {
	g_data.print();
}
```

动态库dym2文件内容，编译后，生成 libdym2.so
``` cpp
// dym2.h文件内容
#include <iostream>

extern "C" {
	void dym2_f1();
}

// dym2.cpp文件内容
#include "dym2.h"
#include "../../static/static.h"

void dym2_f1() {
	g_data.print();
}
```

可执行文件exe文件内容，编译后，生成 exe
```cpp
// 可执行文件 main.cpp文件内容
#include "../../dym1/dym1.h"
#include "../../dym2/dym2.h"

#include <dlfcn.h>


int test1() {
	dym1_f1();
	dym2_f1();
	return 0;
}

typedef void (*FUNC)();

int test2() {
//	void* dym1_h = dlopen("/lib/libdym1.dylib", RTLD_LAZY|RTLD_GLOBAL);
//	void* dym2_h = dlopen("/lib/libdym2.dylib", RTLD_LAZY|RTLD_GLOBAL);
	
	void* dym1_h = dlopen("/lib/libdym1.dylib", RTLD_LAZY|RTLD_LOCAL);
	void* dym2_h = dlopen("/lib/libdym2.dylib", RTLD_LAZY|RTLD_LOCAL);

	FUNC f1 = (FUNC)dlsym(dym1_h, "dym1_f1");
	FUNC f2 = (FUNC)dlsym(dym2_h, "dym2_f1");

	f1();
	f2();

	dlclose(dym1_h);
	dlclose(dym2_h);
	return 0;
}

int main() {
	return test1();
}
```

linux下之行的结果为
```
ctor() this[0x7f47d3a4e068]
ctor() this[0x7f47d3a4e068]
data[1]
data[2]
dtor() this[0x7f47d3a4e068]
dtor() this[0x7f47d3a4e068]
```
macOS下执行的结果为
```
ctor() this[0x10be6a0d0]
ctor() this[0x10be740d0]
data[1]
data[1]
dtor() this[0x10be740d0]
dtor() this[0x10be6a0d0]
```

结论
> (1) 在linux平台下，不同的.so文件中包含相同的全局变量、静态变量时，在链接生成可执行文件的时候，先加载的变量会被后加载的变量覆盖掉。
>     但是构造函数、析构函数仍然会被调用2次。
>
> (2) 在macOS平台下，不同的.so文件中包含相同的全局变量、静态变量时，不会被覆盖。
> 
> (3) 在Linux平台编写.so文件时，要避免使用全局变量、静态变量。
