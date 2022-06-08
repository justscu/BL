
[fmtlib](#fmtlib)


## fmtlib

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

格式语法: [Format String Syntax](https://fmt.dev/latest/syntax.html)
```
replacement_field ::=  "{" [arg_id] [":" (format_spec | chrono_format_spec)] "}"
arg_id            ::=  integer | identifier
integer           ::=  digit+
digit             ::=  "0"..."9"
identifier        ::=  id_start id_continue*
id_start          ::=  "a"..."z" | "A"..."Z" | "_"
id_continue       ::=  id_start | digit

# 
format_spec ::=  [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
fill        ::=  <a character other than '{' or '}'>
align       ::=  "<" | ">" | "^"
sign        ::=  "+" | "-" | " "
width       ::=  integer | "{" [arg_id] "}"
precision   ::=  integer | "{" [arg_id] "}"
type        ::=  "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" | "g" |
                 "G" | "o" | "p" | "s" | "x" | "X"

#
chrono_format_spec ::=  [[fill]align][width]["." precision][chrono_specs]
chrono_specs       ::=  [chrono_specs] conversion_spec | chrono_specs literal_char
conversion_spec    ::=  "%" [modifier] chrono_type
literal_char       ::=  <a character other than '{', '}' or '%'>
modifier           ::=  "E" | "O"
chrono_type        ::=  "a" | "A" | "b" | "B" | "c" | "C" | "d" | "D" | "e" | "F" |
                        "g" | "G" | "h" | "H" | "I" | "j" | "m" | "M" | "n" | "p" |
                        "q" | "Q" | "r" | "R" | "S" | "t" | "T" | "u" | "U" | "V" |
                        "w" | "W" | "x" | "X" | "y" | "Y" | "z" | "Z" | "%"
```

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
//     结果为"price:   6.236"
fmt::print("price:{:>8.3f} \n", 6.235689); // 右对齐，保留3位小数，总长度为8

// (6) 对齐
fmt::print("{:>10}", 245); // 右对齐，长度为10
fmt::print("{:<10}", 245); // 左对齐
fmt::print("{:*^10}", 245); // 中间对齐，用*填充

```




