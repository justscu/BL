### CH9 虚拟内存

![内存系统](https://github.com/justscu/BL/blob/master/pics/CSAPP-9-1-intel-i7-内存系统.png)

CPU通过使用虚拟内存的方式来访问主存（物理内存）. CPU -> 虚拟地址 -> MMU(地址翻译) -> 物理地址 -> 主存.


可以使用`cat /proc/cpuinfo`查看CPU的支持的实际内存与虚拟内存情况。
```sh
cat /proc/cpuinfo
address sizes: 39 bits physical, 48 bits virtual
// 表示CPU支持512GB物理内存，256TB虚拟内存空间
```
在使用分页机制后，256TB的虚拟地址空间被分成大小固定的页（有3种大小: 4KB，2MB，1GB），每一页或者被映射到物理内存，或者没有被映射任何东西。

`段错误`(Segmentation fault)，进程访问一个没有权限访问的地址时，CPU就会触发一个保护故障，并将控制传递给一个内核中的异常处理程序。<br/>
页表是一个`页表条目`(PTE, Page Table Entry)的数组，PTE表中有专门的许可位，用来标识权限信息。如SUP表示该页面需要超级权限的用户或内核才能访问。WRITE表明写权限。

CPU获取一个虚拟内存地址的内容的过程
> (1)CPU生成一个虚拟地址A，并将A传递给MMU; <br/>
> (2)MMU生成PTE的地址，并从高速缓存/主存请求得到它; <br/>
> (3)高速缓存/主存向MMU返回PET; <br/>
> (4)MMU计算物理地址，并把物理地址传递给高速缓存/主存; <br/>
> (5)高速缓存/主存返回所请求的数据给处理器. <br/>

![地址翻译](https://github.com/justscu/BL/blob/master/pics/CSAPP-9-2-intel-i7-地址翻译.png)
