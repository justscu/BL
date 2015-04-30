## <center> pprof </center>

golang自带**pprof**工具，可以做CPU、内存、程序阻塞的profiling。

### 1 环境要求
最好golang在1.4或以上版本，查看命令：`go version`

[帮助文档](https://github.com/hyper-carrot/go_command_tutorial/blob/master/0.12.md)


### 2 CPU prof
#### 2.1 修改代码
```go
import "os"
import "runtime/pprof"  // 引用pprof package

func main() () {
	f, _ := os.Create("profile_file")  
    pprof.StartCPUProfile(f)      // 开始cpu profile，结果写到文件f中  
    defer pprof.StopCPUProfile()  // 结束profile
   // ... 程序其它代码
}
```
运行程序，会生成profile_file文件，该文件记录了程序的性能数据等信息。

#### 2.2 分析profile文件
运行`go tool pprof -dot -output=./x.dot ./bin/svc.news  ./profile_file`，会生成x.dot文件。
运行`xdot x.dot`，可以看到分析的结果图表。

利用交互式的命令进行查看：`go tool pprof bin/svc.news  profile_file`
```sh
(pprof) top
(pprof) list Insert # Insert为函数名字，支持regex格式
(pprof) tree        # 显示调用关系
(pprof) disasm
```

### 3 heap prof
Go语言运行时系统会对用户程序运行期间的所有的堆内存分配进行记录。
#### 3.1 修改代码
```go
import "os"
import "fmt"
import "time"
import "runtime/pprof"

func heap_snap() {
	i := 0
	for {
		i++
		time.Sleep(time.Second*10)
		str := fmt.Sprintf("/tmp/heap.prof_%d", i)
		f, err := os.Create(str)
		if err != nil {
			fmt.Println("Create:", err.Error())
			continue
		}
		err = pprof.WriteHeapProfile(f)　// 写heap信息到文件
		if err != nil {
			fmt.Println("WriteHeapProfile: ", err.Error())
		}
		f.Close()
	}
}

func main() {
	go heap_snap()
	// ... 程序其它代码
}
```
运行程序，每隔10s会生成一个内存使用情况的heap.prof_x文件，该文件记录了程序的内存信息。

#### 3.2 分析
分析使用的命令，与2.2节一致。
可以生成多个x.dot文件，然后比对发现内存增长点最多的函数。

### 4 通过网页查看profiles信息

在使用网页查看信息时，需要专门启动一个服务端口。
```go
import (
	"net/http"
	_ "net/http/pprof"
	"log"
)

func main() {
	go func() {
		log.Println(http.ListenAndServe("0.0.0.0:6060", nil)) // 本例启动6060端口
	}()

	//	... 程序其它代码
}
```
浏览器中输入`http://localhost:6060/debug/pprof/`，可以看到profiles信息。
