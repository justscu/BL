### 常用方法
```sh
// 返回错误
import ("errors")

func f() error {
    return errors.New("return error")
}
```

### 不要使用fmt.Sprintf进行类型转换
函数声明 
- func Sprintf(format string, a ...interface{}) string
- func FormatUint(i uint64, base int) string

fmt.Sprintf()第二个参数是interface{}，内部用到了反射，会有更大的开销。
strconv.FormatUint()内部进行查表操作，速度更快。

```sh
var i uint = 105
s1 := fmt.Sprintf("%d", i)
s2 := strconv.FormatUint(i, 10)
```

### go条件编译
可以使用编译标签(build tag)来进行条件编译。

`redisOpe.go`文件
```go

// +build !redisProxy

package redisOpe

func Get_Ope() string {
	return "redisOpe.go"
}
```

`redisOpe_redisProxy.go`文件
```go

// +build redisProxy

package redisOpe

func Get_Ope() string {
	return "redisOpe_redisProxy"
}
```
在Makefile中，当用`go build -o rePro`时，会编译`redisOpe.go`文件；
当用`go build -o rePro -tags 'redisProxy'`时，会编译`redisOpe_redisProxy.go`文件。
注意："// +build !redisProxy"行是必须的，而且前后都需要空行，否则go编译器会将其当做注视而不是编译标签。
