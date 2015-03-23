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
