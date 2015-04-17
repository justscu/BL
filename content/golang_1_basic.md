## golang基础知识汇总

### 1 约定 

- 基本写法

```go
// (1)package 写法, packname为包名
package packname

// (2)导入外部库
import(
    "fmt"                // fmt是标准库，去 "$GOROOT" 下查找
    "net"
    "app/selfwritelib"   // 非标准库，去 "$GOPATH/src/app/selfwritelib" 查找
)

// (3)每个go最先执行的是init，而不是main。init可以没有，但main必须有
func init() {

}

// (4)在go中，首写字母为大写的函数或变量，是要被导出的。如
var IoNumber int
func WriteIObuffer() error {
    // ... ...
}
```

- go需要显示的类型转换，不支持隐式转换。

```go
var i int = 5
var f float64 = float64(i)

// interface 的强制转换
type T struct{
    K string
    V string
}

func f(i interface{}) {
    i.(T).K //转换
    i.(T).V
}
```

- go有指针，但没有指针运算

```go
type Vertex struct {
    x int
    y int
}

func f() {
    v := Vertex{1, 2}
    p = &Vertex{} //默认初始化为{0,0}，p为 *Vertex指针
    p = &v
    p++ //error，不支持，不能通过++,--改变指针的指向
}
```

- defer，defer是一个栈，先进后出

```go
// defer语句注册一个函数，当defer所在的函数返回时，调用defer注册的函数。
func f() (result int) {
    defer func() {
        result++
    }()

    return 1
}

func main() {
    fmt.Println(f()) // 打印的值为2，因为先返回1，后调用defer.
}
```

### 2 变量

函数外的每个语句，必须以关键字`var/func/type`等开始。

简洁赋值语句`:=`只能出现在函数内部，不能出现在函数外。

### 3 数组array

长度是数组的一部分，[3]int和[4]int是不同的数组，数组的长度在初始化时就确定了，不能动态修改。
可以使用下标对数组进行操作。

```go
package main
import "fmt"
import "reflect"

var a = [3]int{1, 2, 3}  // 一维数组
var b = [2][4]int{[4]int{1,2,3,4}, [4]int{5,6,7,8}} // 二维数组
var c = [2][4]int{{1,2,3,4},{5,6,7,8}} // 二维数组

func main() {
	m := [...]interface{} {  // m为数组，若声明为 m := []interface{}，则m为切片
        [3]int{1, 2, 3}, // 数组，长度为3
        [...]int{1, 2, 3}, // 数组，长度由编译器自己算
        []int{1, 2, 3}, //　切片
        string("aaaaa"),
        string("cccccccc"),
    }
    
    // 数组遍历, k为数组的id，从0开始, v为具体值
    for k, v := range m {
    	fmt.Printf("k[%d] v[%s] \n", k, v)
    }
    
    // 若只关心数组中内容，可以
    for _, v := range m {
    	fmt.Printf("v[%s] \n", v)
    }
    
    /* 结果
		v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		v[aaaaa] 
		v[cccccccc] 
    */
    
    // 只关心数组id
    for k := range m {
    	fmt.Printf("k[%d] ", k) // 结果: k[0] k[1] k[2] k[3] k[4] 
    }
	/* 结果
		k[0] k[1] k[2] k[3] k[4]
	*/
    
	// 求长度
    fmt.Printf("len[%d] \n", len(m))
	/* 结果
		len[5]
	*/
	
	// 查看元素类型
    for _, v := range m {
        rv := reflect.ValueOf(v)
        fmt.Println(rv.Kind())
    }
    /* 结果
    	array
		array
		slice
		string
		string
    */
}

```

### 4 切片slice

slice有cap和len两个属性

#### 4.1 创建

- 通过声明创建，
`p := []int{2,3,4,5,6}`，[]T是一个元素类型为T的slice
- 通过make创建，
`a := make([]int, 0, 5)`，int类型的切片，len(a)=0, cap(a)=5
- 通过赋值创建，
```go
s := [4]int{0, 1, 2, 3}   //　s为数组
t := s[1:3] // 1, 2　t为slice
m := s[1:]  // 1, 2, 3
n := s[:3]  // 0, 1, 2
```
注意：make的第二个参数，决定了append从切片的什么位置开始插入。

#### 4.2 加入元素&遍历
使用**append**向切片中加入元素，若空间不够，会自动开辟新的空间，返回值指向新的切片。
append的原型为：`append(s[ ]T, vs ...T) [ ]T`，
使用`for range`进行遍历。

```go
// (1) 插入元素
func f1() {
	a := make([]int, 2, 3) // len=2,cap=3，区别与make([]int, 0, 3)
	a = append(a, 5) // 若使用 a[3] = 5，程序会崩溃
    a = append(a, 6) //切片会自动增长
    a = append(a, 7)
    //使用range进行遍历
    for k, v := range a { 
        fmt.Printf("k[%d] v[%d] \n", k, v)
    }
    /*结果
    	k[0] v[0] 
		k[1] v[0] 
		k[2] v[5] //　新append的5，从第三个元素开始
		k[3] v[6] 
		k[4] v[7] 
    */
    
	// 切片重新开辟空间
    a = make([]int, 0, 3) // len=0,cap=3
	a = append(a, 5) // 若使用 a[3] = 5，程序会崩溃
    a = append(a, 6) //切片会自动增长
    a = append(a, 7)
    //使用range进行遍历
    for k, v := range a { 
        fmt.Printf("k[%d] v[%d] \n", k, v)
    }
    /* 结果
    	k[0] v[5] 
		k[1] v[6] 
		k[2] v[7]
    */
}

// (2)遍历
func f2() {
	m := []interface{} {  // m为切片，若声明为 m := [...]interface{}，则m为数组
        [3]int{1, 2, 3}, // 数组，长度为3
        [...]int{1, 2, 3}, // 数组，长度由编译器自己算
        []int{1, 2, 3}, //　切片
        string("aaaaa"),
        []string{"S1", "S2", "S3"}, // slice
    }
    
    // 遍历切片
    for k, v := range m {
    	fmt.Printf("k[%d] v[%s] \n", k, v)
    }
	/* 结果
		k[0] v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		k[1] v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		k[2] v[[%!s(int=1) %!s(int=2) %!s(int=3)]] 
		k[3] v[aaaaa] 
		k[4] v[[S1 S2 S3]] 
	*/
	
	for k := range m {
		fmt.Printf("k[%d] ", k)
	}
	/* 结果
		k[0] k[1] k[2] k[3] k[4]
	*/
	
	// 求长度
	fmt.Printf("len[%d] \n", len(m)) // 结果: len[5]
    
    // 查看元素类型
    for _, v := range m {
        rv := reflect.ValueOf(v)
        fmt.Println(rv.Kind())
    }
    /* 结果
    	array
		array
		slice
		string
		slice
    */
}
```

#### 4.3 拷贝和赋值

使用**copy**命令进行拷贝
```go
make([]string, 0, 10) // string类型的切片，长度0，容量10.
copy(dst, src)
var a = [...]int{0,1,2,3,4,5,6,7} // int类型的数组
var s = make([]int, 0, 6)  // int类型的slice
var b = make([]byte, 0, 5) // byte类型的slice

var b[]byte // byte类型切片
b = append(b, "hi"...) //append可能会重新分配底层数组来容纳新的单元

//添加b
a = append(a, b)
//复制
b = make([]T, len(a))
copy(b,a)
//删除 [i:j]
a = append(a[:i], a[j:]...)
//删除第i个元素
a = append(a[:i], a[i+1:]...)
//扩展j个空元素
a = append(a, make([]T, j)...)
//插入j个空元素
a = append(a[:i], append(make([]T, j), a[i:]...)...)
//插入元素x
a = append(a[:i], append([]T{x}, a[i:]...)...)
//插入切片b
a = append(a[:i], append(b, a[i:]...)...)
//弹出最后一个元素
x, a = a[len(a)-1], a[:len(a)-1]
//压入x
a = append(a, x)
```

#### 4.4 切片和数组的不同

数组在编译的时候，就确定了长度，且长度不能改变。
`slice`实际上是对源数组的引用，内部也是用数组实现的，是个长度可变的数组。

```go
a := [...]int{1, 2, 3, 4}  // 数组，长度由编译器来计算
sa := a[0:]    // slice
sa[2] = 10     // 数组a的值，也会跟着变
sa = append(sa, 100) // 数组a的值，也会跟着变

sb := a[0:]
sb = append(sb, 101, 102, 103, 104)
// slice在引用的时候，可能会发生意外。即当slice的长度增大时（append），
// 可能会重新分配内存，这时sb的地址和a的地址不同。
```

数组
- (1)赋值：数组是值类型的，将一个数组赋值给另一个数组时，会拷贝所有的元素。
- (2)值传递：数组作为参数时，传递的是数组的一份拷贝，而不是数组的地址/指针[与c不同]。
- (3)大小：数组的大小是数组的一部分，[10]int和[20]int是不同的，且长度在声明的时候就确定了，不能更改。

slice
- (1)赋值：slice是引用类型的，将一个slice赋值给另一个slice时，两个slice共用相同的内存，改变一个slice的值会影响到另一个slice。
- (2)引用传递：slice作为参数时，传递的是引用。
- (3)大小：slice的大小可以动态调整。


### 5 map

- map使用前，必须使用make来创建
- 可以使用下标法进行赋值
- 可以直接判断元素是否存在
- 使用**for range**迭代时，可以安全的删除和插入map

```go
type Ver struct {
    x, y int
}

var m1 map[string]Ver //声明一个map，key为string类型，value为Ver类型
func f() {
    m1 = make(map[string]Ver) //map使用前，必须使用make而不是new来创建
    m1["dzh"] = Ver{103, 601516} //赋值方法1
    var m2 = map[string]Ver { //赋值方法2
        "Bell": Ver{3,5},
        "Dzh": Ver{6,4},
    }

    m3 := map[string]Ver { //赋值方法3
        "Bell": Ver{3,5},
        "Dzh": Ver{6,4},
    }

    m3["ccav"] = Ver{3, 5} //添加元素
    delete(m3, "Dzh") //删除元素
    if v, ok := m3["Dzh"]; ok { //判断元素是否存在
        fmt.Println(v)
    }
}

m := map[int] string {1:"One", 2:"Two" }
for k, v := range m {
    v // **v只是一个拷贝，不能通过v来改变m中的值**
    m[3] = "Three"
    delete(m, 1)
}
```


### 6 函数 & 闭包

#### 6.1 基本函数和闭包

```go
// 函数
func main() {
    p1 := func(x, y int) float64 { //p1为函数
        return math.Sqrt(x*x+y*y)
    }

    fmt.Println(p1(3,4)) //调用p1函数
}
//-------------------------------------------------------------------
// 闭包
// func(a, b int) float64是一个函数的声明
func adder(m float64) func(a, b int) float64 {
    sum := 0.0
    return func(a, b int) float64 {
        sum += (m * 10) + math.Sqrt(float64(a*a+b*b))
        return sum
    }
}

func main() {
    pos20 := adder(20) // 返回函数
    fmt.Println(pos20(3, 4))

    pos30 := adder(30)  // 返回函数
    fmt.Println(pos30(6,8))
}
//---------------------------------------------------------------------

// 用闭包演示 fibonacci
package main
import "fmt"
// fibonacci 函数会返回一个返回 int 的函数。
func fibonacci() func() int {
    a := 0
    b := 0
    return func() int {
        if a == 0 && b == 0 {
            a = 1
            b = 0
            return 0
        }

        c := a+b
        a = b
        b = c
        return c
    }
}

func main() {
    f := fibonacci()
    for i := 0; i < 10; i++ {
        fmt.Printf("%d ", f()) // 0 1 1 2 3 5 8 13 21 34
    }

    fmt.Printf("%d ", f()) // 55
}
```

#### 6.2 不定参数

`arg ...int`，表示函数的参数个数不定；且`...int`这种参数只能作为函数的最后一个

```go
// (1)使用 s... interface{} 作为不定长度参数
func re7(a int, s... interface{}) {
	for k, v := range s {
		fmt.Println("k[", k, "] ", "v[", v, "]")
	}
}

// (2)使用 arg... int 作为不定长度参数
func re8(a int, arg ...int) {
    for k, v := range arg {
        fmt.Println("k[", k, "] ", "v[", v, "]")
    }
}

func main() {
	m := [ ]interface{} {  // m为数组，若声明为 m := []interface{}，则m为切片
        [3]int{1, 2, 3}, // 数组，长度为3
        [...]int{1, 2, 3}, // 数组，长度由编译器自己算
        []int{1, 2, 3}, //　切片
        string("aaaaa"),
        []string{"a1", "a2", "a3"},
    }
	
	re7(len(m), m)
	/* 结果
		k[ 0 ]  v[ [[1 2 3] [1 2 3] [1 2 3] aaaaa [a1 a2 a3]] ]
	*/
	
	re7(len(m), m...)
	/* 结果
		k[ 0 ]  v[ [1 2 3] ]
		k[ 1 ]  v[ [1 2 3] ]
		k[ 2 ]  v[ [1 2 3] ]
		k[ 3 ]  v[ aaaaa ]
		k[ 4 ]  v[ [a1 a2 a3] ]
	*/
	
	re8(len(m), m[2].([]int)...)
	/* 结果
		k[ 0 ]  v[ 1 ]
		k[ 1 ]  v[ 2 ]
		k[ 2 ]  v[ 3 ]
	*/
}
```

#### 6.3 参数传递

函数参数传递，都是以拷贝的形式进行的。若想在函数中改变值，需要使用指针。

**Go语言中string，slice，map这三种类型的实现机制类似指针，所以可以直接传递，而不用取地址后传递指针**。（注：若函数需改变slice的长度，则仍需要取地址传递指针）

```go
func add_1(a *int) {
    *a = *a +1
    return
}
```
#### 6.4 使用局部变量

在闭包中使用局部变量时，应该用参数的形式传入
```go
func S1() {
	for i := 0; i < 5; i++ {
		go func() {
			fmt.Printf("%d, ", i)
		} ()
	}
}
/* 输出:
	5, 5, 5, 5, 5, 
*/

func S2() {
	for i := 0; i < 5; i++ {
		go func(i int) {
			fmt.Printf("%d, ", i)
		} (i)
	}
}
/* 输出:
	0, 1, 2, 3, 4, 
*/
```

### 7 方法

golang没有类，但可以在struct的基础上定义方法。

```go
type Ver struct {
    x, y int
}

// 用拷贝,不改变值
func (v Ver) Abs() float64 {
    return math.Sqrt(float64(v.x*v.x + v.y*v.y))
}
// 用指针,改变值
func (v *Ver) Setx(a int) {
    v.x = a // 改变x的值
}

func main() {
    v := &Ver{6,8} //指针
    fmt.Println(v.Abs()) // 10
}
```

### 8 interface

```go
type ITEM struct {
    k int64
    v string
}

// interface类型转换
func achange(v interface{}) {
    k := v.(ITEM).k
    v := v.(ITEM).v
}
```

### 9 goroutine
- 非抢占式的，需要自己让出yield
- go程调度发生在操作系统调用时。当发生系统调用时，会使正在执行的go程让出CPU给其他go程
- go程相互通信，靠的是channel。channel类型的运算有三种：**send/receive/select**

```go
var chin chan int //声明一个chin，用于收发int型
chin<-0 //把0发送给chin
<-chin //从chin中读取
chin = make(chan int) //必须make后才能进行操作
chin = make(chan int, 1024) //不标明个数(1024)，默认为0
```

- select语句的每个case分支，只能是发送和接收语句

### 10 channel

- 收&发

```go
func send(strCh chan <- string) {
	var i int = 0
	for {
		i += 2
		iStr := fmt.Sprintf("%d", i)
		
		time.Sleep(time.Second * 1)
		if i <= 20 {
			strCh <- iStr // 发
		} else {
			close(strCh)  // close the channel
			break
		}
	}
}

func recv(strCh <- chan string) {
	for {
		str := <- strCh  // 收
		if len(str) != 0 {
			fmt.Printf("recv [%s] \n", str)
		} else {
			// 收到长度为０的消息，表面channel被close掉了
			fmt.Printf("recv close strCh info \n")
			break
		}
	}
}

func main() {
	strCh := make(chan string, 8) // chan的长度为8
	go recv(strCh)
	send(strCh)
	//　等待所有信息都输出
	time.Sleep(time.Second * 1) 
}
```

- 和map与slice一样，channel使用前必须创建
`ch := make(chan int)`，不带长度时，默认长度为0

- 接收者可以通过赋值语句的第二个参数来测试channel是否被关闭

```go
if v, ok := <- ch; ok == false {  //判断ok的返回值
    fmt.Println("被关闭了")
}
```

- 只有发送者才可以关闭channel，接收者不能关闭channel。向一个已经关闭的channel发送数据时，会panic。
- 当channle关闭后，下面的for循环会被退出。

```go
for i := range ch { //循环会不停的从ch中读取数据，直到ch被关闭

}
```

- 当调用close(ch)时，所有处于`<-ch`的routine都能够收到消息，可以利用该属性让所有的routine退出。



### 11 select

	检查每个case语句;
	如果有任意一个chan是send or recv read，那么就执行该block;
	如果多个case是ready的，那么随机找1个并执行该block;
	如果都没有ready，那么就block and wait;
	如果有default block，而且其他的case都没有ready，就执行该default block;

- select会被阻塞，直到case分支中有条件满足。
```go
func f() {
    for {
        select { //select会被阻塞，直到case中的条件满足
            case x <- ch1: //从ch1通道中读取数据
                fmt.Println(x)
            case y <- ch2: //从ch2通道中读取数据
                fmt.Println(y)
        }
    }
}
```

- 当select中的其他条件分支都没有准备好的时候，default分支会被执行。
```go
func f() {
    for {
        select {
            case x <- ch1:
                fmt.Println(x)
            case y <- ch2:
                fmt.Println(y)
            default:
                time.Sleep(time.Millisecond) //当其他分支没有被触发时，总被满足
        }
    }
}
```

### 12 make和new

`new`和`make`是go内建的内存分配函数，用来分配内存。
make只用来创建slice，map和channel，且返回一个初始化的(而不是置零)类型为T的值（而不是`*T`）。make的返回值不是指针。
```go
sa := make([]int, 0, 256) // 返回int类型的切片
sa = append(sa, 10)
sa = append(sa, 20)

ma := make(map[int]string, 256) // 返回一个map
ma[5] = "55"
ma[0] = "00"

ca := make(chan int, 256) // 返回 ch
ca <- 2
ca <- 0
```

new返回的是指针
```go
na := new([]int) // 返回指向int型切片的指针，即 *[]int
// na并没有指向一块内存，不能够直接使用，可以使用make分配内存
*na = make([]int, 0, 256)
```

比较通用的做法
```go
type T struct {
	i int
	b bool
	s string
}

nt := new(T)  // nt为指针，即 *T
fmt.Println(nt) // 输出：&{0 false }

pt := &T{i: 5, b: true, s: "aaa"}
fmt.Println(pt) // 输出：&{5 true aaa}
```
