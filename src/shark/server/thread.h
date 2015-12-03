#ifndef __SHARK_THREAD_H__
#define __SHARK_THREAD_H__

#include <vector>
#include <list>
#include <thread> // c++11 thread需要
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include <unistd.h>
#include "lock.h"

class Thread {
public:
    Thread() :
            thread_(nullptr), event_(nullptr), event_base_(nullptr) {
        thread_id_ = 0;
    }
    virtual ~Thread();
    Thread(const Thread&) = delete;
    Thread operator=(const Thread&) = delete;

public:
    virtual bool start();
    virtual bool stop();
    virtual bool join();
    virtual void OnRead(struct bufferevent *bev);
    virtual void OnEvent(struct bufferevent *bev, short what);

public:
    //  分派任务
    bool dispatch(std::function<void(Thread*)> func);
    std::thread::id get_id() const;
    // 返回线程id
    pthread_t get_thread_id() const;
public:
    event_base* get_event_base() {
        return event_base_;
    }
private:
    // 线程回调函数
    void cb_thread();
    // 有新任务时，读回调(socketpair)
    static void cb_read(evutil_socket_t fd, short int /*which*/, void *arg);

private:
    evutil_socket_t fd_[2]; // 0读1写
    pthread_t thread_id_; // thread id
    std::thread* thread_;
    event* event_;
    event_base* event_base_;

    RWLock lock_;
    std::list<std::function<void(Thread*)> > funcs_;
};

// 用来管理threads
class ThreadPool {
public:
    ThreadPool(uint32_t numbers) :
            numbers_(numbers), cur_(0) {
        threads_.reserve(numbers_);
    }
    ~ThreadPool() {
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 初始化
    bool init() {
        for (uint32_t i = 0; i < numbers_; ++i) {
            Thread* t = new Thread;
            threads_.push_back(t);
        }
        return true;
    }
    // 启动线程
    bool start() {
        for (auto i : threads_) {
            i->start();
        }
        /*
         * 在 Thread::start()后，便启动了线程，但何时调用线程函数Thread::cb_thread，是随机的
         * 所以在这个地方sleep(1)，等待Thread::cb_thread执行成功
         * */
        sleep(1);
        return true;
    }
    bool stop() {
        for (auto i : threads_) {
            i->stop();
        }

        for (auto i : threads_) {
            i->join();
        }

        for (auto i : threads_) {
            delete i;
        }
        return true;
    }

    bool dispatch(std::function<void(Thread*)> func) {
        uint32_t cur = cur_++ % numbers_;
        return threads_[cur]->dispatch(func);
    }

private:
    uint32_t numbers_; // 线程数
    uint32_t cur_;     // 当前使用的线程
    std::vector<Thread*> threads_;
};

#endif /*  __SHARK_THREAD_H__ */
