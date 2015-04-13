## <center> pprof </center>

golang自带**pprof**工具，可以做CPU、内存、程序阻塞的profiling。

### 1 环境要求
要求golang在1.4或以上版本，查看命令：`go version`

[帮助文档](https://github.com/hyper-carrot/go_command_tutorial/blob/master/0.12.md)

### 2 修改代码
```go
import "os"
import "runtime/pprof"  // 引用pprof package

func main() () {
	f, _ := os.Create("profile_file")  
    pprof.StartCPUProfile(f)      // 开始cpu profile，结果写到文件f中  
    defer pprof.StopCPUProfile()  // 结束profile
	... ...
}
```
运行程序，会生成profile_file文件，该文件记录了程序的性能数据等信息。

### 3 分析
运行`go tool pprof -dot -output=./x.dot ./bin/svc.news  ./profile_file`，会生成x.dot文件。
运行`xdot x.dot`，可以看到分析的结果图表。

利用交互式的命令进行查看：`go tool pprof bin/svc.news  profile_file`
```sh
(pprof) top
(pprof) list Insert # Insert为函数名字，支持regex格式
(pprof) tree        # 显示调用关系
(pprof) disasm
```
