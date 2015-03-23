golang自带**pprof**工具，可以做CPU、内存的profiling。

### 1 环境要求
要求golang在1.4或以上版本，查看命令：`go version`

### 2 修改代码
```sh
import "os"
import "runtime/pprof"  // 引用pprof package

func main() () {
	f, _ := os.Create("profile_file")  
    pprof.StartCPUProfile(f)      // 开始cpu profile，结果写到文件f中  
    defer pprof.StopCPUProfile()  // 结束profile
	... ...
}
```
运行程序，会生成profile_file文件，该文件中记录了程序的性能数据等信息。

### 3 分析
运行`go tool pprof -dot -output=./x.dot ./bin/svc.news  ./profile_file`，会生成x.dot文件。
运行xdot x.dot，可以看到分析的结果图标。

利用交互式的命令进行查看：`go tool pprof bin/svc.news  profile_file`
