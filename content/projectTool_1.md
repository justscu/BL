## gcc命令

GCC是一套由GUN开发的支持多种编程语言的编译器。支持C、C++、GO等。

```sh
-o，生成目标文件main，如 gcc main.c -o main
-Wall，打开所有警告，如 gcc -Wall main.c -o main
-Werror，将警告当做错误来处理
-E，将预编译结果，输出到main.i文件中(去掉注释、宏展开、include替换)，如 gcc -E main.c > main.i
-S，将汇编的结果，输出到main.s文件中，如 gcc -S main.c > main.s
-C，只编译，不链接，生成main.o文件，如 gcc -C main.c
-I，表示头文件的路径，如gcc foo.c -I /home/ll/work/include/ -o foo
-L，表示库文件的路径，如gcc foo.c -L /home/ll/commlib/ -ljson -o foo
-l，表示使用库，使用libjson.so文件，如 gcc main.c -o main -ljson
-Ox，代码优化，其中-O0，表示不优化；-O1到-O3代表不同级别的优化
-V，打印编译过程中的调试信息，如 gcc -V

-ansi，支持ISO C89风格
-funsigned-char，将char类型解释为unsigned char类型。通过该编译选项，char会被当做unsigned char处理
-fsigned-char，将unsigned char类型解释为char类型

-D，编译时，声明了宏。如-DCCAV，相当于在代码中有这样一句：#define CCAV
-g，增加调试信息
-ggdb3，在gdb调试时，可以查看宏的信息

-fPIC，创建动态库时，创建无关联的地址信息代码
	gcc -C -fPIC Cfile.c
	gcc -shared -o libCfile.so Cfile.o
```
GCC默认动态库优先于静态库（先找.so，再找.a），若在编译的时候想优先使用静态库，需要加上-static
