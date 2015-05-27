1.程序有什么问题
```go
func main() {
    select {
    
    }
}
```
没有case和default，程序将阻塞。
fatal error: all goroutines are asleep - deadlock!


2.程序的输出
```go
func main() {
    f()
    fmt.Println("1.")
}

func f() {
    defer func() {
        if r := recover(); r != nil {
            fmt.Println("2.", r)
        }
		fmt.Println("3.")
    }()

    fmt.Println("4.")
    g()
    fmt.Println("5.")
}

func g() {
    panic("6.")
}
```
panic和recover机制。


3.程序的输出
```go
func main() {
	var fs = [2]func(k int){ }

	for i := 0; i < 2; i++ {
		defer fmt.Printf("a.i = %d \n", i)

		defer func() {
			fmt.Printf("b.i = %d \n", i) 
		}()

		fs[i] = func(k int) {
			fmt.Printf("c.%d = %d \n", k, i)
		}
	}

	for k, v := range fs {
		v(k)
	}
}
```
defer里面的变量是申明的时候就copy的，不会随着后面的函数逻辑改变而改变，除非你用指针类型。

```go
func deferFunc() int {
    index := 0

    fc := func() {
        fmt.Println(index, "f1")
        index++

        defer func() {
            fmt.Println(index, "f2")
            index++
        }()
    }

    defer func() {
        fmt.Println(index, "f3")
        index++
    }()

    defer fc()

    return func() int {
        fmt.Println(index, "f4")
        index++
        return index
    }()
}

func main() {
	deferFunc()
}
```
defer:LIFO机制。
closure:闭包共享局部变量。


4.程序的输出
```go
type User struct {
	Id   int
	Name string
}

type Tester interface {
	Test()
}

func (this *User) Test() {
	fmt.Println(&this)
}

func t11() Tester {
	var x *User = nil
	return x
}

func main() {
	t := t11()
	if t == nil {
		fmt.Println("nil")
	} else {
		fmt.Println("not nil")
	}
}
```
`t=t11()`返回的是一个interface{}，值为nil。


5.解释

5.1 make和new、slice和array、make(chan int, 1)和make(chan int)的区别。

	make用于slice/map/chan分配内存，返回的是对象T；new返回的是指针*T。
```go
a := new(map[int]string) // 没有分配内存
*a = make(map[int]string, 0, 1024)
```
	slice是切片，内部用array实现，slice包含类型/len/cap三元素。slice的长度可动态变化，array的长度不能改变。
```go
a := []int{1, 2, ,3 ,4}   // slice
b := [...]int{1, 2, 3, 4} // array，自动计算长度
c := [4]int{1, 2, 3, 4}   // array
```
	`make(chan int)`不带缓冲区，不能往里面写数据；`make(chan int, 1)`缓冲区的长度为1。

5.2 select中的case可以是什么类型的? select读多个chan时，是顺序的，还是随机的?

	select case must be receive, send or assign recv.　因为锁的原因，select中的case是随机的。

5.3 如何在select多个channel的时候，关闭所有chan后再退出select?

	应该由writer来close一个chan，而不是reader。往一个关闭的chan写数据，会panic。
```go
var i := 0
for {
	select {
		case c: <- ch1:
			if c == nil {
				i++
			}
		case <- ch2:
			if c == nil {
				i++
			}
		case <- ch3:
			if c == nil {
				i++
			}
		default:
			if i == 3 {
				break // 退出
			}
	}
}
```

5.4 如果一个线程写chan，另外一个线程读chan，如何保证在close(chan)后，立刻停止读取呢？丢弃后面写入的数据。

	
5.5 golang使用的GC算法（Mark And Sweep,标记和扫描）

