#### 1 同select比较    
对于`select`，"linux/posix_types.h"头文件有这样的宏：`#define __FD_SETSIZE  1024`，
`select`能监听的最大fd数量，在内核编译的时候就确定了的。若想加大fd的数量，需要调整编译参数，重新编译内核。

`epoll_wait`直接返回被触发的fd或对应的一块buffer，不需要遍历所有的fd。
epoll的实现中，用户空间跟内核空间共享内存(mmap)，避免拷贝。

#### 2 相关函数和结构
```cpp
#include <sys/epoll.h>
typedef union epoll_data { //注意是union
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event {
	uint32_t events; /* Epoll events */
	epoll_data_t data; /* User data variable */
};

// size: 该epoll上监控socket fd的最大个数
int epoll_create(int size);

/*  通过epoll_ctl，将要监控的fd加入到epoll中　
	op: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
	fd: 要加入到epoll监控的fd
	event.events: EPOLLIN, EPOLLOUT, EPOLLERR, EPOLLET(默认为LT), EPOLLPRI, EPOLLHUP等
*/
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

/*  等待epoll被触发，返回值为此次被触发的fd个数
	events: 预先分配的，用来存放被触发的信息
	maxevents: 本次被触发的最大个数
	timeout: epoll_wait被block的最大时间, -1永远阻塞；0不阻塞
*/
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

int close(int fd);
```

#### 3 基本操作
(1)加入到epoll监控
```cpp
struct epoll_event ee;
ee.events = EPOLLIN|EPOLLOUT|EPOLLET; //不设置EPOLLET的话，为LT模式
ee.data.ptr = (void *) buffer;//传入buffer地址
epoll_ctl(epfd, EPOLL_CTL_ADD, socketfd, &ee);
```
(2)从epoll监控中删除
```cpp
struct epoll_event ee;
ee.events = 0;
ee.data.ptr = NULL;
epoll_ctl(epfd, EPOLL_CTL_DEL, socketfd, &ee);
```
(3)等待事件的触发
```cpp
for (;;){
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1){    
        if (errno == EINTR)
            continue; // 中断
        perror("epoll_pwait");
        exit (EXIT_FAILURE);
    }

    for (n = 0; n < nfds; ++n){
        // events[n].events; //是什么事件 EPOLLIN/EPOLLOUT/EPOLLERR/EPOLLPRI
        // events[n].data.fd;//被触发的fd(在epoll_ctrl时传入的)
        // events[n].data.ptr;//跟fd相关联的buffer(在epoll_ctrl时传入的)
        if( events[n].events & EPOLLIN ) { }
        else if( events[n].events & EPOLLOUT ) { }
        else if( events[n].events & EPOLLERR ) { }
        else {}
    }
}
```

#### 4 读和写
epoll有2种模式，水平触发（LT）&边缘触发（ET）。

##### 4.1 LT模式下的读和写
* 读：若接收缓冲区中有数据，就会一直触发EPOLLIN。所以不用担心是否读干净缓冲区中的数据，若没有读完的话，EPOLL会一直被触发。
* 写：若发送缓冲区中还有空间的话，会一直触发EPOLLOUT（只有在发送缓冲区满的时候，才不会被触发）。所以这个很烦，本来没有数据需要发送，但EPOLLOUT仍然被触发。解决方法：
	* (1)epoll_ctl(, EPOLL_CTL_ADD, fd)，将该fd加入到epoll中；
	* (2)该fd的EPOLLOUT被触发，发送数据（一般在回调函数中完成）；
	* (3)epoll_ctl(epfd, EPOLL_CTL_DEL, fd),将该fd移出epoll。加入又移出的，很麻烦，又有开销。若数据比较少的时候，直接调用send发送好了。

##### 4.2 ET模式下的读和写
ET模式下，是不能用阻塞式的函数的。
* 读：fd由不可读->可读（接收缓冲区没有数据->有数据）时，才会触发EPOLLIN。若没有读干净，read返回，缓冲区仍然有数据时，不会再次触发。
```cpp
n=read(fd, buf, buf_size);
if(n<0){
    if (errno == EAGAIN)
        continue;
    else
        return -1; //errno, should close the fd
}
else if(n < buf_size) {
    // 读完了缓冲区中的数据，可以返回
    return n;
}
else if(n == buf_size) {
    // 缓冲区中可能还有数据，继续读
    continue;
}
```
* 写：fd由不可写->可写（发送缓冲区满->有空间）时，才会触发EPOLLOUT。

#### 5 需要注意的信号
服务器程序，一般需要处理以下信号。
* SIGPIPE：Broken pipe: write to pipe with no readers，对socket的两端A和B，若B已经关闭。当A第一次写时，内核会返回RST给应用程序；当A第二次写，内核会返回SIGPIPE信号给应用程序。SIGPIPE默认动作是关闭应用程序，生成core文件，所以A端需要处理SIGPIPE信号。可以忽略该信号，signal(SIGPIPE, SIG_IGN)。
* SIGINT：当用户按ctl+c时，会产生SIGINT信号。可以用来终止进程。signal(SIGINT, proc_exit);
* SIGKILL/SIGSTOP：不能被捕捉或忽略的信号。提供了一种可以杀死进程的方法。signal(SIGKILL/SIGSTOP, proc_exit);
* SIGTERM：Termination signal，由kill命名发出的系统默认终止信号。signal(SIGTERM, proc_exit);

operation |  block                              | nonblock
----------|-------------------------------------|-------------------------------------------------------------------------------|
read      |收buff有数据就立即返回;buff为空时阻塞|立即返回;有数据时返回读到的长度;无数据返回-1(errno=EAGAIN/EWOULDBLOCK)
write     |写完整个要发送的数据，才会返回       |立即返回写成功的字节数;若一个字节都没有写成功，返回-1(errno=EAGAIN/EWOULDBLOCK)
