## ubuntu下配置golang开发环境

### 1 相关文档
- [golang中国](http://www.golangtc.com/)
- [studyGolang](http://studygolang.com/)
- [开发环境相关配置](http://golang.org/doc/code.html)
- [学习文档](https://github.com/astaxie/build-web-application-with-golang/blob/master/ebook/preface.md)


### 2 查看OS的位数
- uname -a，若显示x86，则是32bit的；若显示x86_64，则是64bit的。
- getconf -a | grep LONG_BIT，若返回32，则是32bit的；若返回64，则是64bit的。
- arch命令

```sh
ll@ll-rw:~$ uname -a
Linux ll-rw 3.13.0-35-generic #62-Ubuntu SMP Fri Aug 15 01:58:01 UTC 2014 i686 i686 i686 GNU/Linux
#出现i686，表明是32bit的

ll@ll-rw:~$ getconf -a | grep LONG_BIT
LONG_BIT 32

[ll@localhost src]$ arch
x86_64  #表明是64bit的
```


### 3 安装golang编译器
(1)下载依赖 
`cd /home/ll & sudo apt-get install bison gawk gcc libc6-dev make; sudo apt-get install mercurial`

(2)下载go解析器源码
`hg clone -r release https://go.googlecode.com/hg/ go`

(3)运行脚本
`cd go/src;  ./all.rc`

(4)当看见下面这段话，表面安装成功
```sh
　ALL TESTS PASSED
---
Installed Go for linux/386 in /home/ll/go
Installed commands in /home/ll/go/bin
*** You need to add /home/ll/go/bin to your PATH.
```

(5)修改环境变量，即` ~/.bashrc`文件
```sh
export GOROOT=/home/ll/go
export GOARCH=386
export GOOS=linux
export GOBIN=$GOROOT/bin/
export GOTOOLS=$GOROOT/pkg/tool/
export PATH=$PATH:/home/ll/go/bin:$GOTOOLS
```
记住，之后要**source ~/.bashrc**。

(6)随便进入一个目录，运行**go env**，看看安装有没有成功

(7)自己建一个test.go文件，在里面写入
```sh
package main
import "fmt"
func main() {
	fmt.Printf("hello, world\n")
}
```
然后运行*go run test.go*；若输出"hello, world"，就表明go的编译器安装成功了。


### 4 下载代码提示工具gocode

(1)cd /home/ll/ & mkdir goprojects

(2)cd /home/ll/goprojects

(3)修改*.bashrc*文件，增加`GOPATH`环境变量
```sh
export GOPATH=/home/ll/goprojects
export PATH=$PATH:/home/ll/go/bin:$GOTOOLS:$GOPATH/bin
```
记住，之后要**source ~/.bashrc**。

(4)**go get -u github.com/nsf/gocode**，下载gocode插件。若下载成功，可以看到/home/ll/go/bin下面多了一个gocode文件。


### 5 配置eclipse的开发环境(goclipse)

(1)安装goclipse插件，地址为：http://goclipse.googlecode.com/svn/trunk/goclipse-update-site/

(2)打开eclipse->Window->Perferences->go，
```sh
GOROOT: /home/ll/go
GOPATH: /home/ll/goprojects
Go Tools会自动补全的。
Gocode path: /home/ll/go/bin/gocode. 
```

(3)当修改库文件时，eclipse是不会跟着编译的，可以使用"Project->Clean..."。

### 6 相关配置 

#### 6.1 go工程

每个go工程，有个workspace。其下包括src、pkg、bin共3个目录。
- `src`为源码
- `pkg`为编译后生成的库文件（不可直接执行的）
- `bin`为编译后生成的可执行文件

#### 6.2 GOPATH

The **GOPATH** environment variable specifies the location of your workspace. It is likely the only environment variable you will need to set when developing Go code.

To get started, create a workspace directory and set GOPATH accordingly. Your workspace can be located wherever you like, Note that this must not be the same path as your Go installation.

The GOPATH environment variable lists places to look for Go code. GOPATH must be set to get, build and install packages outside the standard Go tree.

If DIR is a directory listed in the GOPATH, a package with source in DIR/src/foo/bar can be imported as "foo/bar" and has its compiled form installed to "DIR/pkg/GOOS_GOARCH/foo/bar.a".

Go searches each directory listed in GOPATH to find source code, but new packages are always downloaded into the first directory in the list. 

```sh
mkdir $HOME/goprojects
export GOPATH=$HOME/goprojects
export PATH=$PATH:$GOPATH/bin

import ("fmt")      // fmt是golang的标准库，所以去"$GOROOT"下查找
import ("kv/dcvs")  // kv/dcvs是非标准库，所以去"$GOPAHT/src"下查找
```

#### 6.3 package的路径

从外部下载的包，应当放到$GOPATH/src目录下面。一般的格式为: `mkdir -p $GOPATH/src/github.com/packge_from_internet`, -p表示一次创建多级目录。假设新的新包放在"github.com/package_from_internet"处

#### 6.4 开发自己的可执行程序

假设自己写的程序，放在"github.com/myproc/"目录下，`mkdir -p $GOPATH/src/github.com/myproc/`。
然后在myproc目录下，创建hello.go文件，并进行开发。
编译命令为：
```sh
go install github.com/myproc/hello
或者
cd github.com/myproc/ & go instal
```
若生成可执行文件，则在"$GOPATH/bin"下可以找到。

#### 6.5 开发自己的库文件

假设自己写的库程序，放在"github.com/libproc/"目录下，mkdir -p $GOPATH/src/github.com/libproc/。
然后在libproc目录下，创建proc.go文件，并进行开发。
```sh
go build github.com/libproc/proc & go install github.com/libproc/proc
或者
cd github.com/libproc/ & go build & go install
```
若生成库文件，在$GOPATH/pkg下可以找到。


#### 6.6 测试文件的写法

假设测试文件为：$GOPATH/src/github.com/user/newmath/sqrt_test.go
```sh
// sqrt_test.go文件内容 // (1)文件名必须以 _test.go结尾
package newmath
import "testing"  // （2）必须包含该包
func Test_Sqrt(t *testing.T) { // （3）格式声明必须是 func TestXXX(t *testing.T)
	const in, out = 4, 2
	if x := Sqrt(in); x != out {
		t.Errorf("Sqrt(%v) = %v, want %v", in, x, out)
	}
}

// 运行：go test github.com/user/newmath 或者 cd $GOPATH/src/github.com/user/newmath & go test
```

#### 6.7 使用remote packages

获取remote package命令：**go get url**, 如 `go get code.google.com/p/go.example/hello`，假设下载[code.google.com/p/go.example/hello]文件。

go get命令会自动下载、编译、安装。go get will fetch, build, and install it automatically。

#### 6.8 帮助

(1)在命令行，输入`godoc -http=:6060`，启动doc服务器；在浏览器中输入`http://localhost:6060/`，就可以进行浏览了。

(2)也可以在命令行直接输入要查找的内容，如**godoc fmt Sprintf**，表示要查询fmt.Sprintf命令。
