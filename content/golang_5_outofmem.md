本文分析了由内存不足导致程序panic的原因，及应对方法。

#### 内存不足

下面的ftp程序，在经过一段时间运行后，会panic，提示`fatal error: out of memory`。
```go
package main

import (
		"fmt"
		"time"
		"ftp"
)

func main() {
	ftp := &ftpOpe{IP:"192.168.1.5", Usr:"anonymous", Pwd:"anonymous"}
	for {
		err := ftp.Conn()
		if err == nil {
			break
		}
		fmt.Printf("ftp.Conn error[%s] \n", err.Error())
		time.Sleep(time.Second*5)
	}
	defer ftp.Close()
	
	for {
		name := "a/b/c/d.pdf"
		pdfContent, err := ftp.GetFile(name)
		if err != nil {
			fmt.Errorf("len[%d] [%s] err[%s] \n", len(pdfContent), name, err.Error())
		} else {
			fmt.Printf("[%s] success \n", name)
		}
//		runtime.GC()
	}
}
```

经过一段时间运行，程序会挂掉，panic信息为:
```sh
runtime: memory allocated by OS (0x809f5000) not in usable range [0x9731a000,0x1731a000)
runtime: memory allocated by OS (0x854bd000) not in usable range [0x9731a000,0x1731a000)
runtime: out of memory: cannot allocate 131072-byte block (536870912 in use)
fatal error: out of memory

goroutine 37 [running]:
runtime.throw(0x83c4ee5)
    /home/ll/go/src/pkg/runtime/panic.c:520 +0x71 fp=0x854fabd4 sp=0x854fabc8
largealloc(0x1, 0x854fac2c)
    /home/ll/go/src/pkg/runtime/malloc.goc:226 +0xaf fp=0x854fabf8 sp=0x854fabd4
runtime.mallocgc(0x1fe00, 0x8201181, 0x1)
    /home/ll/go/src/pkg/runtime/malloc.goc:169 +0xbc fp=0x854fac2c sp=0x854fabf8
cnew(0x8201180, 0x1fe00, 0x1)
    /home/ll/go/src/pkg/runtime/malloc.goc:836 +0xad fp=0x854fac3c sp=0x854fac2c
runtime.cnewarray(0x8201180, 0x1fe00)
    /home/ll/go/src/pkg/runtime/malloc.goc:849 +0x3f fp=0x854fac4c sp=0x854fac3c
makeslice1(0x81f8fc0, 0x1fe00, 0x1fe00, 0x854fac8c)
    /home/ll/go/src/pkg/runtime/slice.goc:55 +0x4b fp=0x854fac58 sp=0x854fac4c
runtime.makeslice(0x81f8fc0, 0x1fe00, 0x0, 0x1fe00, 0x0, 0x0, 0x1fe00, 0x1fe00)
    /home/ll/go/src/pkg/runtime/slice.goc:36 +0xb2 fp=0x854fac78 sp=0x854fac58
bytes.makeSlice(0x1fe00, 0x0, 0x0, 0x0)
    /home/ll/go/src/pkg/bytes/buffer.go:191 +0x72 fp=0x854fac9c sp=0x854fac78
bytes.(*Buffer).ReadFrom(0xa55d0e40, 0x871ef818, 0xa5b750c0, 0xfe00, 0x0, 0x0, 0x0)
    /home/ll/go/src/pkg/bytes/buffer.go:163 +0xbe fp=0x854fad18 sp=0x854fac9c
io/ioutil.readAll(0x871ef818, 0xa5b750c0, 0x200, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
    /home/ll/go/src/pkg/io/ioutil/ioutil.go:33 +0x160 fp=0x854fad84 sp=0x854fad18
io/ioutil.ReadAll(0x871ef818, 0xa5b750c0, 0x0, 0x0, 0x0, 0x0, 0x0)
    /home/ll/go/src/pkg/io/ioutil/ioutil.go:42 +0x6e fp=0x854fadac sp=0x854fad84
xxx/ftpOpe.GetFile(0x9739a3e0, 0xe, 0x9739a3f9, 0x3, 0x9739a409, 0x3, 0x97469a45, 0x1, 
0x974640c0, 0xa5ba93e0, ...)
    /home/ll/goprojects/src/xxx/ftpOpe/ftpope.go:84 +0x125 fp=0x854fade4 sp=0x854fadac
```

根据panic时，打印出的栈信息，挨个查看函数的实现:
```go
// ftpOpe/ftpope.go中的函数
func (f ftpOpe) GetFile(srcFile string) (content []byte, err error) {
	// create file
	r, err := f.conn.Retr(srcFile)
	if err != nil {
		return nil, err
	} 

	defer r.Close()
	buf, err := ioutil.ReadAll(r)
	if err != nil {
		return nil, err
	}
	
	return buf, err
}
```

```go
// io/ioutil/ioutil.go 文件
var ErrTooLarge = errors.New("bytes.Buffer: too large")

// readAll reads from r until an error or EOF and returns the data it read
// from the internal buffer allocated with a specified capacity.
func readAll(r io.Reader, capacity int64) (b []byte, err error) {
	buf := bytes.NewBuffer(make([]byte, 0, capacity))
	// If the buffer overflows, we will get bytes.ErrTooLarge.
	// Return that as an error. Any other panic remains.
	defer func() {
		e := recover()
		if e == nil {
			return
		}
		if panicErr, ok := e.(error); ok && panicErr == bytes.ErrTooLarge {
			err = panicErr
		} else {
			panic(e)
		}
	}()
	_, err = buf.ReadFrom(r)  // 调用 bytes/buffer.go:ReadFrom函数
	return buf.Bytes(), err
}

// ReadAll reads from r until an error or EOF and returns the data it read.
// A successful call returns err == nil, not err == EOF. Because ReadAll is
// defined to read from src until EOF, it does not treat an EOF from Read
// as an error to be reported.
func ReadAll(r io.Reader) ([]byte, error) {
	return readAll(r, bytes.MinRead)
}
```

```go
// bytes/buffer.go: ReadFrom

// ReadFrom reads data from r until EOF and appends it to the buffer, growing
// the buffer as needed. The return value n is the number of bytes read. Any
// error except io.EOF encountered during the read is also returned. If the
// buffer becomes too large, ReadFrom will panic with ErrTooLarge.
func (b *Buffer) ReadFrom(r io.Reader) (n int64, err error) {
	b.lastRead = opInvalid
	// If buffer is empty, reset to recover space.
	if b.off >= len(b.buf) {
		b.Truncate(0)
	}
	for {
		if free := cap(b.buf) - len(b.buf); free < MinRead {
			// not enough space at end
			newBuf := b.buf
			if b.off+free < MinRead {
				// not enough space using beginning of buffer;
				// double buffer capacity
				newBuf = makeSlice(2*cap(b.buf) + MinRead) // 调用　makeSlice
			}
			copy(newBuf, b.buf[b.off:])
			b.buf = newBuf[:len(b.buf)-b.off]
			b.off = 0
		}
		m, e := r.Read(b.buf[len(b.buf):cap(b.buf)])
		b.buf = b.buf[0 : len(b.buf)+m]
		n += int64(m)
		if e == io.EOF {
			break
		}
		if e != nil {
			return n, e
		}
	}
	return n, nil // err is EOF, so return nil explicitly
}

// makeSlice allocates a slice of size n. If the allocation fails, it panics
// with ErrTooLarge.
func makeSlice(n int) []byte {
	// If the make fails, give a known error.
	defer func() {
		if recover() != nil {
			panic(ErrTooLarge)
		}
	}()
	return make([]byte, n)  // 调用runtime/malloc.goc:runtime·cnewarray函数
}
```

```go
// runtime/malloc.goc文件

static void*
cnew(Type *typ, intgo n, int32 objtyp)
{
	if((objtyp&(PtrSize-1)) != objtyp)
		runtime·throw("runtime: invalid objtyp");
	if(n < 0 || (typ->size > 0 && n > MaxMem/typ->size))
		runtime·panicstring("runtime: allocation size out of range");
	return runtime·mallocgc(typ->size*n, (uintptr)typ | objtyp, 
		typ->kind&KindNoPointers ? FlagNoScan : 0);
}

// same as runtime·new, but callable from C
void*
runtime·cnew(Type *typ)
{
	return cnew(typ, 1, TypeInfo_SingleObject);
}

void*
runtime·cnewarray(Type *typ, intgo n)
{
	return cnew(typ, n, TypeInfo_Array); //　调用上面的runtime·cnew
}
```

当main中的for{}不断循环时，程序不停的调用`ioutil.Readall`中的函数
```go
func ReadAll(r io.Reader) ([]byte, error)
```
该函数不停的进行内存分配。
`a/b/c/d.pdf`为47.21MB大小，经过20多次循环后，程序就抛出异常了，提示`fatal error: out of memory`。

本来20次循环，程序需要47.21M*20=944.2M的空间，但通过top可以看到实际的内存已经达到了1.8G，这与`bytes/buffer.go: ReadFrom`函数的分配策略有关，`newBuf = makeSlice(2*cap(b.buf) + MinRead)`，每次都是x*2+512字节的速度进行分配。这会消耗过多的内存。
内存只分配不回收，导致了程序因内存不足而panic。

#### 垃圾回收
go语言有GC，可以对用户分配的内存进行回收。按道理说，GC起作用的话，上面的程序应该不会出现内存不足的问题。于是，在main函数的for循环中，主动调用GC命令`runtime.GC()`，之后长时间运行程序，内存的使用比较稳定，且没有出现`fatal error: out of memory`。
看来是go的GC机制的问题。

`GODEBUG="gctrace=1" ./main 2> app.log`命令可以将每次的GC操作都记录到app.log中。GOGCTRACE是环境变量，go在每次GC时，都将信息输出到标准错误中，所以将其重定向到gc.log中。

不调用runtime.GC()时，输出到gc.log中的日志
```
gc4(1): 2+1+556+1 us, 0 -> 0 MB, 2416 (2625-209) objects, 40/0/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc5(1): 2+1+694+1 us, 0 -> 0 MB, 2269 (3495-1226) objects, 56/10/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc6(1): 1+13+646+1 us, 0 -> 0 MB, 1476 (3497-2021) objects, 63/50/10 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc7(1): 2+1+602+1 us, 0 -> 2 MB, 1474 (3499-2025) objects, 64/49/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc8(1): 2+1+596+1 us, 1 -> 3 MB, 1474 (3500-2026) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc9(1): 2+1+643+1 us, 3 -> 7 MB, 1474 (3501-2027) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc10(1): 2+1+681+2 us, 6 -> 14 MB, 1474 (3502-2028) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc11(1): 2+1+587+1 us, 12 -> 28 MB, 1474 (3503-2029) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc12(1): 3+15+639+2 us, 24 -> 56 MB, 1474 (3504-2030) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc13(1): 2+1+651+2 us, 48 -> 112 MB, 1474 (3505-2031) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc14(1): 2+1+640+2 us, 96 -> 224 MB, 1535 (3568-2033) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc15(1): 15+2+661+2 us, 180 -> 416 MB, 1597 (3694-2097) objects, 71/65/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc16(1): 2+1+684+2 us, 326 -> 672 MB, 1733 (3955-2222) objects, 85/80/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
```

调用runtime.GC()时，输出到gc.log中的日志
```
gc7(1): 2+1+576+1 us, 0 -> 2 MB, 1468 (3492-2024) objects, 64/49/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc8(1): 2+1+588+1 us, 1 -> 3 MB, 1468 (3493-2025) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc9(1): 2+1+647+1 us, 3 -> 7 MB, 1468 (3494-2026) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc10(1): 2+1+698+1 us, 6 -> 14 MB, 1468 (3495-2027) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc11(1): 3+1+686+1 us, 12 -> 28 MB, 1468 (3496-2028) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc12(1): 2+1+645+2 us, 24 -> 56 MB, 1468 (3497-2029) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc13(1): 2+1+641+2 us, 48 -> 112 MB, 1468 (3498-2030) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc14(1): 2+1+580+1 us, 96 -> 96 MB, 1474 (3505-2031) objects, 64/50/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc15(1): 2+1+629+2 us, 84 -> 192 MB, 1516 (3564-2048) objects, 64/0/49 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc16(1): 2+1+605+1 us, 160 -> 160 MB, 1474 (3571-2097) objects, 70/64/0 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc17(1): 2+1+590+1 us, 146 -> 192 MB, 1522 (3637-2115) objects, 70/0/50 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc18(1): 2+1+583+1 us, 146 -> 192 MB, 1522 (3703-2181) objects, 70/0/64 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc19(1): 2+1+608+1 us, 146 -> 192 MB, 1522 (3769-2247) objects, 70/0/64 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc20(1): 2+1+586+1 us, 146 -> 192 MB, 1526 (3839-2313) objects, 70/0/64 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc21(1): 2+1+591+1 us, 146 -> 192 MB, 1526 (3905-2379) objects, 71/0/65 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc22(1): 2+1+526+1 us, 146 -> 192 MB, 1526 (3971-2445) objects, 71/0/65 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
gc23(1): 2+1+597+1 us, 146 -> 192 MB, 1526 (4037-2511) objects, 71/0/65 sweeps, 0(0) handoff, 0(0) steal, 0/0/0 yields
```

*	`gc23(1)`是第23次进行GC，1个go程在执行，go程数对应GOMAXPROCS环境变量；
*	`2+1+597+1 us`表示本次GC动作消耗的时间，数值越大，说明GC消耗的时间越长；在执行GC期间，程序是要暂停的；
*	`146 -> 192 MB`表示内存的变化，从146M增加到192M，可以看到调用`runtime.GC()`后，内存一直比较稳定；
*	`1526 (4037-2511) objects`表示GC后，对象从4037个减少到2511个；

#### 解决方法
当不知道有多少数据需要读取时，使用`ioutil.Readall`中的`func ReadAll(r io.Reader) ([]byte, error)`比较方便，但会浪费内存。

当明确知道有多少数据需要读取时，最好使用`io.ReadFull`函数:
```go
body := make([]byte, 0, lenth)
_, err := io.ReadFull(res.Body, body)
```
