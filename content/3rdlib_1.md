fmtlib

### 编译

```sh
git clone https://github.com/fmtlib/fmt.git fmt
# 选择tag 8.1.1
cd fmt & git tag & git checkout -b 8.1.1 
mkdir debug & cd debug
cake .. & make -j
# 编译完毕后，生成“libfmt.a”静态库
```

### 使用
将"include/fmt/format.h, include/fmt/core.h, debug/libfmt.a"拷贝到需要使用的目录，如"third/fmt/" <br/>

在头文件中，增加
```cpp
#include <fmt/format.h>
#include <fmt/core.h>
```

### fmt语法

基础用法示例
```cpp
#include <fmt/core.h>
#include <fmt/format.h>

using namespace fmt::literals;

// (1) 结果为: "The answer is 4"
fmt::print("The answer is {} \n", 4);

// (2) 结果为: "string:ABC, char:F, Int:8, Double:5.12"
fmt::print("string:{}, char:{}, Int:{}, Double:{} \n", "ABC", 'F', 8, 5.12);

// (3) 使用索引，从0开始
//     结果为: "ABC Hello ABC"
fmt::print("{1} {0} {1} \n", "Hello", "ABC");

// (4) 使用命名的{}来格式化, 需要 “using namespace fmt::literals;”
//     结果为"SSE:600008"
fmt::print("{exchange}:{code} \n", "exchange"_a = "SSE", "code"_a = "600008");

// (5) 需要长度的格式化
//     结果为"price:6.236"
fmt::print("price:{:.3f} \n", 6.235689);

```
