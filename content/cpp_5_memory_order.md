原子操作 <br/>
编译乱序/CPU执行乱序 <br/>
volatile <br/>
spin_lock <br/>
std::atomic <br/>
C++11 memory order <br/>

`原子操作`，CPU保证没有线程能够观察到原子操作的中间状态。即某个操作要么全做，要么全不做，不会有线程观察到一半的状态。如fetch_add(Read-Modify-Write), CAS(compare_exchange_strong)。

CPU为了提高流水线的运行效率，会做出：（1）对无依赖的前后指令做适当的乱序和调度；（2）对控制依赖的指令做分支预测；（3）对读取内存等耗时操作，做提前预读... 这些问题，需要考虑：（1）CPU指令的执行顺序；（2）CPU读/写；（3）缓存；（4）不同CPU核之间缓存同步。

`锁(Lock)`使得程序性能变差的原因：A线程得到锁后，可能被OS调度出去；其它线程在等待锁的过程中，就会陷入阻塞，严重影响程序性能。</br>
`无锁（Lock-free）`，没有显式使用锁的代码，不一定是Lock-free的。Lock-free的实现，需要依赖`原子操作`和`memory-order`。

`顺序一致性(Sequential Consistency)`，即保证所有线程看到的是一样的内存修改顺序，同时这个顺序和源码顺序一致。


### cpp11 6种memory_order

|                name |                info |
|---------------------|---------------------|
|memory_order_relaxed | 不保证执行顺序
|memory_order_release | 本线程中，所有之前的写操作完成后才能执行本条原子操作 
|memory_order_acquire | 本线程中，所有后续的读/写操作必须在本条原子操作完成后执行
|memory_order_consume | 本线程中，所有后续的有关本原子类型的操作，必须在本条原子操作完成后执行
|memory_order_acq_rel | 同时包含memory_order_acquire和memory_order_release操作
|memory_order_seq_cst | Sequentially Consistent，所有线程看到的是一样的内存修改顺序，同时这个顺序和源码顺序一致

cpp11中的memory order，是语言层面的。编译器会根据不同的cpu平台选择合适的手段来保持同步。这样可以达到跨平台的目的。程序员可以不用考虑cpu等硬件平台的memory同步。<br/>
std::memory_order specifies how memory accesses, including regular, non-atomic memory accesses, are to be ordered around an atomic opertion. 指定了（包括普通的非原子内存访问在内的）原子操作前后的内存访问方式。即`原子变量`除了有原子操作的特性外，还会限制其前后的非原子变量的内存操作。

`memory_order_relaxed`，宽松内存序。该原子操作仅仅保证自身的原子性，不对其前后变量的读写产生影响。如果不需要其它的内存同步，选用该类型可以获取最好性能（如原子计数器）。

```cpp
namespace Test_memory_order_relaxed {

void thread1(int32_t &a, std::atomic<bool> &ready) {
    a = 5; // step 1
    ready.store(true, std::memory_order_relaxed); // step 2
}

void thread2(int32_t &a, std::atomic<bool> &ready) {
    while (!ready.load(std::memory_order_relaxed)) { ; } // step 3

    if (a != 5) { // step 4
        assert(0);
    }
}

void test() {
    for (uint64_t i = 0; i < 100*10000ul; ++i) {
        int32_t                a{0};
        std::atomic<bool>  ready{false};

        std::thread *t1 = new (std::nothrow) std::thread(thread1, std::ref(a), std::ref(ready));
        std::thread *t2 = new (std::nothrow) std::thread(thread2, std::ref(a), std::ref(ready));
        t1->join();
        t2->join();
    }
}

} // namespace Test_memory_order_relaxed

```

`memory_order_release`, 适用于store operation，对于采用此内存序的store operation，我们可以称为release operation，设有一个原子变量M上的release operation，对周围内存序的影响是：该release operation前的内存读/写都不能重排到该release operation之后。并且: <br/>
> 截止到该release operation的所有内存写入都对另外线程对M的acquire operation以及之后的内存操作可见，这就是release acquire语义。<br/>
> 截止到该operation的所有M所依赖的内存写入都对另外线程对M的consume operation以及之后的内存操作可见，这就是release consume语义。<br/>

`memory_order_acquire`, 适用于load operation，对于采用此内存序的load operation，我们可以称为acquire operation，设有一个原子变量M上的acquire operation，对周围内存序的影响是：当前线程中该acquire operation后的load 或strore不能被重排到该acquire operation前，其他线程中所有对M的release operation及其之前的写入都对当前线程从该acquire operation开始往后的操作可见.

```cpp
    std::atomic<char*> ptr{nullptr};
    int32_t data = 5;

    void thread1() {
        char *p = new (std::nothrow) char[8];
        memcpy(p, "hello", 6);
        data = 42;
        ptr.store(p, std::memory_order_release);
    }

    void thread2() {
        char *p = nullptr;
        while (!(p = ptr.load(std::memory_order_acquire))) {
            ;
        }

        // ready
        assert(memcmp(p, "hello", 6) == 0); // never fires
        assert(data == 42);  // never fires
    }

    void test() {
        std::thread t1(thread1);
        std::thread t2(thread2);
        t1.join();
        t2.join();
    }
} // Test_memory_order_acquire
```

`memory_order_consume`, 适用于load operation，对于采用此内存序的load operation，我们可以称为consume operation，设有一个原子变量M上的consume operation，对周围内存序的影响是：当前线程中该consume operation后的依赖该consume operation读取的值的load 或strore不能被重排到该consume operation前，其他线程中所有对M的release operation及其之前的对数据依赖变量的写入都对当前线程从该consume operation开始往后的操作可见，相比较于下面讲的memory_order_acquire，memory_order_consume只是阻止了之后有依赖关系的重排。

`memory_order_acq_rel`, 适用于read-modify-write operation，对于采用此内存序的read-modify-write operation，我们可以称为acq_rel operation，既属于acquire operation 也是release operation. 设有一个原子变量M上的acq_rel operation：自然的，该acq_rel operation之前的内存读写都不能重排到该acq_rel operation之后，该acq_rel operation之后的内存读写都不能重排到该acq_rel operation之前. 其他线程中所有对M的release operation及其之前的写入都对当前线程从该acq_rel operation开始的操作可见，并且截止到该acq_rel operation的所有内存写入都对另外线程对M的acquire operation以及之后的内存操作可见。

`memory_order_seq_cst`, 可以用于 load operation，release operation, read-modify-write operation三种操作，用于 load operation的时候有acquire operation的特性，用于 store operation的时候有release operation的特性, 用于 read-modify-write operation的时候有acq_rel operation的特性，除此之外，有个很重要的附加特性，一个单独全序，也就是所有的线程会观察到一致的内存修改，也就是`顺序一致性`的强保证。


### memory barrier / memory fence

`内存栅栏`(Memory Barrier)，由于缓存的出现，会导致一些操作不用到内存，就可以返回继续执行后面的操作。为了保证某些操作必须写入到内存后才执行，就引入了`内存栅栏`。内存栅栏保证了，在栅栏指令之前的所有的内存操作的结果，都在栅栏指令之后的内存操作指令执行之前，被写入到内存中。

```cpp
    inline void lfence() {
        __asm__ __volatile__("lfence" ::: "memory"); // load fence, 读串行
    }
    
    inline void sfence() {
        __asm__ __volatile__("sfence" ::: "memory"); // store fence, 写串行
    }
    
    inline void mfence() {
        __asm__ __volatile__("mfence" ::: "memory"); // load & store memory fence, 读写都串行
    }
```

对于多核心处理器，每个核心都有自己的缓存，而这些缓存并非都实时跟内存进行交互。
这样就会出现一个核心上的缓存数据，跟另外一个核心上的缓存数据不一致的问题。该问题可能会导致程序异常。
为了解决这个问题，操作系统提供内存屏障。内存屏障核心作用: <br/>
> (1) 阻止fence两边的指令重排 <br/>
> (2) 强制把`写缓冲区/高速缓存`中的`脏数据`写回主存，让所有核心缓存中的相应数据失效。即保证所有核心跟内存中最新数据一致，保证数据有效性。<br/>

内存屏障分类 <br/>
> (1) lfence (load fence), 在`读指令前`插入读屏障，让高速缓存中的数据失效，重新从内存中加载数据 <br/>
> (2) sfence (store fence), 在`写指令后`插入写屏障，让写入高速缓存的最新数据都写回到主内存 <br/>
> (3) mfence，同时具有lfence和sfence能力 <br/>

Lock前缀 <br/>
Lock不是内存屏障，但提供了内存屏障类似功能。<br/>
> (1) 先对总线/缓存加锁，再执行后面的指令，再把高速缓存中的脏数据刷回主存，最后释放锁 <br/>
> (2) 通过Lock锁住总线的时候，其余CPU的读写操作都被阻塞，直到锁被释放。Lock锁住总线后的写操作，会使其余cpu的相关cache line失效。<br/>

