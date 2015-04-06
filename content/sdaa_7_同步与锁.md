## 系统设计与性能优化之七：同步与锁

### 1 基本知识
当多个线程（包括进程、主机）共享同一份资源时，需要考虑资源互斥与同步的问题。

所有的问题，缘起于一份资源，N个线程都想操作它。当然，N个线程读同一份资源，是没有问题的，而且读到的内容也是相同的；但当有线程读、有线程写同一份资源时，就会有问题。

每个线程有独立的栈空间，互不干扰。每个进程的所有线程，共享堆空间，所以当我们的程序有new出的空间、static类型变量、全局变量时，就需要考虑竞争问题，看看是否有多个线程竞争这些资源。

当多个线程共享相同的内存时，需要确保每个线程看到的数据是一致的。（1）若都去读这个数据，是没有问题的；（2）若写的时间大于一个存储器的访问周期，就可能会有问题；（3）当存储器读与存储器写这两个周期交叉时，潜在的不一致就会出现。

常用的同步方式有：**互斥量、锁(读写锁、互斥锁)、文件锁、条件变量、信号量**等。 

### 2 加锁时需要考虑的问题
加锁，可以解决同一进程内的多个线程对同一资源的争夺。可以使用锁来避免多个线程的资源共享问题。
在使用锁时，要注意**锁的粒度**：
* （1）锁太粗，会出现多个线程等待相同的锁，降低程序的并发能力；
* （2）锁太细，过多的锁开销，会降低程序的性能，而且使代码变得复杂。
在Linux系统中，常见的锁有mutex，rwlock, cond，是非递归的。即*同一个线程对锁lock两次，会死锁*。 

### 3 锁的持续性
互斥锁、读写锁、条件变量常用于同一进程内的线程间同步，若将它们放在共享内存区，也能用于进程间同步。
若进程在持有互斥锁时终止，**内核不会负责自动释放持有的锁**。内核自动清理的唯一类型的锁是fcntl锁。

若被锁住的互斥锁的持有进程或线程终止，会造成这个互斥锁无法解锁，因而死锁。线程可以安装线程清理程序，用来在被取消时能释放持有的锁。但这种释放可能会导致共享对象的状态被部分更新，造成不一致。

![sdaa_7_1](https://github.com/justscu/BL/blob/master/pics/sdaa_7_1.png)

### 4 原子读写函数pread/pwrite
- ssize_t pread(int fd, void *buf, size_t count, off_t offset);
- ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
相当于`lseek+read/write`函数。但是原子操作的，即在read/write完成前，无法中断其lseek和read/write操作。
原子操作是指由多个操作（占用多个CPU时序）组成的操作。要么执行完所有的操作，要么一步也不执行；不可能只执行所有步骤中的一个子集。 

### 5 锁带来的问题
优先级反转
- （1）threadA, threadB, threadC...多个线程，都去竞争同一把锁。
- （2）当threadA先执行，获取了这把锁，执行被锁的代码。这时，threadA的时间片完了. 
- （3）threadB分到了时间片，threadB开始被调度，但threadB获取不了锁，于是，threadB开始等待(阻塞)，直到被分给的时间片完。
- （4）threadC, threadD...分到了时间片，需要等待锁(阻塞)...
- （5）然后threadA又分到了时间片，开始执行... ...，执行完被锁住的代码后，释放锁... ...
- （6）threadB获取时间片，获取到锁，开始执行。
 这就会出现低优先级的线程先运行（threadB的优先级比threadA高，但threadA先运行）。 


### 6 无锁lock-free
利用compare and swap (CAS)实现无锁队列，
无锁队列，主要使用函数__sync_bool_compare_and_swap，当没有获取到资源时，该函数理立即返回，而不是等待。
```cpp
unsigned int mtx_trylock(unsigned int* mtx)
{
    return ( *mtx == 0 && (__sync_bool_compare_and_swap(mtx, 0, 1)) );
}

static const int ncpu = 4; //假设为4核CPU
void mtx_lock(unsigned int* mtx)
{
    for(;;)
    {
        if(*mtx == 0 && (__sync_bool_compare_and_swap(mtx, 0, 1)))
        {
            return; //获得锁
        }
        if(ncpu > 1)
        {
            for(unsigned int pause_time = 1; pause_time <= 16; pause_time <<= 1)
            {
                for(unsigned int i = 0; i < pause_time; ++i)
                {
                    __asm__ ("pause"); //没有获得锁，先pause会
                }
                if(*mtx == 0 && (__sync_bool_compare_and_swap(mtx, 0, 1)))
                {
                    return; //获得锁
                }
            }
        }
        
        sched_yield(); // 让出CPU，让其他线程先运行
    }
}

void mtx_unlock(unsigned int* mtx)
{
    __sync_bool_compare_and_swap(mtx, 1, 0); // 释放锁
}
```
用于加减的函数`__sync_fetch_and_add`。

### 7 信号回调函数
虽然加锁使得同一份资源在多个线程间共享成为可能。但当有信号回调函数操作共享资源的时候，就得小心了。
```cpp
void signal_cb_fn(void) //信号回调函数
{
    static int a = 0;
    a++;
    return;
}
```
可能在a++还没有完成的时候，信号再次发生。加锁也不能够解决这个问题，因为同一线程对同一锁lock两次，可能会死锁(非递归锁)。
当然，对于在栈上开辟空间的变量，是不存在这个问题的。所以在信号回调函数中，尽量避免使用在堆上开辟空间的变量。

### 8 互斥量(mutual exclusion)

互斥量可用于进程/线程间的同步，用于保护临界区(critical region)。任一时刻，只有一个进程/线程在执行其中的代码。
```cpp
// 互斥量, 同一个线程对同一个mutex加锁2次，会死锁。
class ThreadMutexLock
{
public:
    ThreadMutexLock() {
        int iRet = pthread_mutex_init(&m_mutex, NULL);
        if (iRet != 0)
        {
            std::cout << "pthread_mutex_init: " << strerror(errno) << std::endl;
            assert(0);
        }
    }
    virtual ~ThreadMutexLock() {
        int iRet = pthread_mutex_destroy(&m_mutex);
        if (iRet != 0)
        {
            std::cout << "pthread_mutex_destroy: " << strerror(errno) << std::endl;
            assert(0);
        }
    }
    // 同一个线程，对同一个锁 加锁2次，也会死锁
    int Lock(bool bBlock = true) {
        if (bBlock)
            return pthread_mutex_lock(&m_mutex);
        else
        {//either acquires the lock or fails and returns immediately
            return pthread_mutex_trylock(&m_mutex);
        }
    }
    virtual int UnLock() {
        return pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};
```
进程间使用互斥量：需要开辟一块共享内存给互斥量，同时设置PTHREAD_PROCESS_SHARED属性。
```cpp
pthread_mutexattr_t mattr;
pthread_mutexattr_init(&mattr);
pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

pthread_mutex_t* mptr;
mptr = xxx; //在共享内存区，开辟一块空间
pthread_mutex_init(mptr, &mattr);
```

### 9 读写锁
读写锁适合读多写少的时候。读共享，写独占。
```cpp
// 线程读写锁，读共享；写独占
class ThreadRWLock
{
public:
    ThreadRWLock() {
        int iRet = pthread_rwlock_init(&m_rwlock, NULL);
        if (iRet != 0)
        {
            std::cout << "pthread_rwlock_init: " << strerror(errno) << std::endl;
            assert(0);
        }
    }
    virtual ~ThreadRWLock() {
        int iRet = pthread_rwlock_destroy(&m_rwlock);
        if (iRet != 0)
        {
            std::cout << "pthread_rwlock_destroy: " << strerror(errno)
             << std::endl;
            assert(0);
        }
    }
    // 读共享 return, 0,success.
    int ReadLock(bool bBlock = true) {
        if (bBlock)
            return pthread_rwlock_rdlock(&m_rwlock);
        else
        {//either acquires the lock or fails and returns immediately
            return pthread_rwlock_tryrdlock(&m_rwlock);
        }
    }
    // 写独占， return, 0,success.，其他返回失败
    // 同一个线程，对同一个锁 加锁2次，会死锁
    int WriteLock(bool bBlock = true) {
        if (bBlock)
            return pthread_rwlock_wrlock(&m_rwlock);
        else
        {//either acquires the lock or fails and returns immediately
            return pthread_rwlock_trywrlock(&m_rwlock);
        }
    }
    virtual int UnLock() {
        return pthread_rwlock_unlock(&m_rwlock);
    }
private:
    pthread_rwlock_t m_rwlock;
};
```
进程间使用读写锁：需要开辟一块共享内存给互斥量，同时设置进程间共享属性PTHREAD_PROCESS_SHARED。
```cpp
pthread_rwlockattr_t rwattr;
pthread_rwlockattr_setpshared(&rwattr, PTHREAD_PROCESS_SHARED);

pthread_rwlock_t* rwptr;
wrptr = xxx; //在共享内存区，开辟一块空间
pthread_rwlock_init(rwptr, &rwattr);
```

### 10 条件变量
互斥锁用于上锁，条件变量用于等待。每个条件变量总有一个互斥锁与其相关联。
条件变量（cond）需要和mutex一起使用。mutex是用来锁cond的，使用时，最难理解的是pthread_cond_wait。
当向一个条件变量发送信号时，若此时没有线程在等待条件变量，该信号将消失[不同于信号量semaphore]。
```cpp
// 利用pthread_cond实现一个同步队列
// (1)mutex是用来锁cond的，(2)pthread_cond_wait，是先解锁后加锁的过程。
template<class T>
class SyncQueue // 并发阻塞队列
{
public:
    SyncQueue() {
        int iRet = pthread_mutex_init(&m_mutex, NULL);
        if (iRet != 0)
        {
            std::cout << "pthread_mutex_init: " << strerror(errno) << std::endl;
            assert(0);
            return;
        }
        iRet = pthread_cond_init(&m_cond, NULL);
        if (iRet != 0)
        {
            std::cout << "pthread_cond_init: " << strerror(errno) << std::endl;
            assert(0);
        }
    }
    virtual ~SyncQueue() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    void Push(const T & t) {
        pthread_mutex_lock(&m_mutex);
        const bool bEmpty = m_list.empty();
        m_list.push_back(t);
        pthread_mutex_unlock(&m_mutex);
        if (bEmpty)
            pthread_cond_broadcast(&m_cond);
    }
    bool Pop(T& t) {
        pthread_mutex_lock(&m_mutex);
        while (m_list.empty())
        { // (1)把m_cond的线程放到等待条件发生的队列中，然后解锁，并等待条件的发生。
            // (2)若条件发生时，加锁，同时返回。
            pthread_cond_wait(&m_cond, &m_mutex); //只有为空，才需要等.
        }
        t = m_list.front();
        m_list.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return true;
    }
    bool IsEmpty() {
        pthread_mutex_lock(&m_mutex);
        bool bEmpty = m_list.empty();
        pthread_mutex_unlock(&m_mutex);
        return bEmpty;
    }
private:
    std::list<T> m_list;
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};
```

### 11 记录锁（文件锁)
当多个进程竞争同一个文件资源的时候，可以使用记录锁。使用struct flock和函数fcntl。
- （1）同一个进程可以对文件多次加锁，加锁的区域会合并，最终解锁一次即可。
- （2）【锁的进程属性】进程终止时，进程占用的文件锁全部释放。fork的子进程，不继承父进程已经获取的锁。
- （3）【锁的文件属性】fd关闭时，该进程加在该fd上的锁，全部被释放。
```cpp
class ProcessFileLock
{
public:
    ProcessFileLock(const char* path)
    {
        m_fd = open(path, O_RDWR|O_CREAT, 0666);
        if(m_fd < 0)
        {
            printf("open file[%s] error[%s] \n", path, strerror(errno));
            return;
        }
    }
    ~ProcessFileLock(){
        if(m_fd > 0)
            close(m_fd);
    }
    // @return: 0, success; else, failed, and errno was set.
    int ReadLock(bool bBlock = true)
    {
        struct flock lock;
        lock.l_type = F_RDLCK; //读，共享锁
        lock.l_start = 0; // 偏移0字节
        lock.l_whence = SEEK_SET; //从文件头开始
        lock.l_len = 0; //锁整个文件
        int iRet = -1;
        if(bBlock)
            iRet = fcntl(m_fd, F_SETLKW, &lock);// 阻塞锁
        else
            iRet = fcntl(m_fd, F_SETLK, &lock);// 非阻塞锁
        return iRet;
    }
    // @return: 0, success; else, failed, and errno was set.
    int WriteLock()
    {
        struct flock lock;
        lock.l_type = F_WRLCK;//写，独占锁
        lock.l_start = 0; // 偏移0字节
        lock.l_whence = SEEK_SET; //从文件头开始
        lock.l_len = 0; //锁整个文件
        int iRet = -1;
        if(bBlock)
            iRet = fcntl(m_fd, F_SETLKW, &lock);// 阻塞锁
        else
            iRet = fcntl(m_fd, F_SETLK, &lock);// 非阻塞锁
        return iRet;
    }
    // @return: 0, success; else, failed, and errno was set.
    int UnLock()
    {
        struct flock lock;
        lock.l_type = F_UNLCK;//解锁
        lock.l_start = 0; // 偏移0字节
        lock.l_whence = SEEK_SET; //从文件头开始
        lock.l_len = 0; //锁整个文件
        int iRet = fcntl(m_fd, F_SETLK, &lock);
    }
private:
    int m_fd;
};
```

### 12 Posix信号量
信号量可用于进程/线程间同步。fork调用后，在父进程中打开的信号量，在子进程中仍然打开。
* （1）有名信号量。多个进程/线程间共享，靠的是信号量的名字。随内核持续，创建/销毁函数
```cpp
const char* pSemName = "/sem_test";
sem_t* pSem = sem_open(pSemName, oflag, mode, value);
sem_close(pSem);
sem_unlink(pSemName);
```

* （2）基于内存信号量。需要在共享内存区为信号量分配空间。创建/销毁函数：
```cpp
sem_t* pSem = xxx; //共享内存上开辟一块空间
sem_init(pSem, shared, value);
sem_destroy(pSem);
```
若基于内存的信号量，是单个进程内的多个线程共享(shared=0)，那么该信号量随进程持续。当进程退出时，信号量消失。
若基于内存的信号量，是多个进程间共享(shared=1)，那么该信号量必须放到共享内存区域，只要该共享内存区域还存在，则该信号量就继续存在。

* （3）其它常用函数
```cpp
sem_wait, sem_trywait
sem_post, sem_getvalue
```