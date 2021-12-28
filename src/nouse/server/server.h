/*
 * server.h
 *
 *  Created on: 2015年9月9日
 *      Author: justscu
 */

#ifndef __SHARK_SERVER_H__
#define __SHARK_SERVER_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <functional>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "thread.h"
#include "session.h"

template<class SESSION = Session>
class ServerBase1 {
public:
    ServerBase1(uint16_t port) {
        port_ = port;
        listener_ = nullptr;
        event_base_ = nullptr;
        signal_event_ = nullptr;
        th_pool_ = nullptr;
    }
    ~ServerBase1() {
        if (signal_event_ != nullptr) {
            event_free(signal_event_);
            signal_event_ = nullptr;
        }
        if (listener_ != nullptr) {
            evconnlistener_free(listener_);
            listener_ = nullptr;
        }
        if (event_base_ != nullptr) {
            event_base_free(event_base_);
            event_base_ = nullptr;
        }
        if (th_pool_ != nullptr) {
            delete th_pool_;
            th_pool_ = nullptr;
        }
    }

    ServerBase1(const ServerBase1&) = delete;
    ServerBase1& operator=(const ServerBase1&) = delete;

    bool init() {
        th_pool_ = new ThreadPool(8);
        th_pool_->init();

        event_base_ = event_base_new();
        if (event_base_ == nullptr) {
            return false;
        }

        // listen
        struct sockaddr_in addr;
        memset(&addr, 0x00, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        //addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);
        listener_ = evconnlistener_new_bind(event_base_, cb_listen,
                (void*) this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                (struct sockaddr*) &addr, sizeof(addr));
        if (listener_ == nullptr) {
            printf("evconnlistener_new_bind failed \n");
            return false;
        }

        // 信号处理
        signal_event_ = evsignal_new(event_base_, SIGINT, cb_SIGINT,
                (void* )this);
        if (signal_event_ == nullptr) {
            printf("evsignal_new failed \n");
            return false;
        }
        if (0 != event_add(signal_event_, nullptr)) {
            printf("event_add SIGINT failed \n");
            return false;
        }

        // event_set_fatal_callback(event_fatal_callback);
        return true;
    }

    bool start() {
        th_pool_->start();
        event_base_loop(event_base_, 0);
        return true;
    }

    bool stop() {
        th_pool_->stop();
        while (0 != event_base_loopbreak(event_base_)) {
            sleep(1);
        }
        printf("event_base_loopbreak \n");
        return true;
    }

private:
    // 该函数通过socketpair通知到thread中，实际上会在thread中被调用
    void accept(Thread* t, evutil_socket_t clientfd, struct sockaddr* addr,
            int /*socklen*/) {
        sockaddr_in* peer = (sockaddr_in*) addr;
        char buf[32];
        sprintf(buf, "%s:%d", inet_ntoa(peer->sin_addr), htons(peer->sin_port));
        printf("[%lu] accept: [%s] fd[%d]\n", t->get_thread_id(), buf,
                clientfd);

        struct bufferevent* bev = bufferevent_socket_new(t->get_event_base(),
                clientfd, BEV_OPT_CLOSE_ON_FREE);
        if (bev == nullptr) {
            printf("bufferevent_socket_new failed \n");
            return;
        }
        Session* s = new SESSION(t->get_thread_id(), clientfd, bev,
                std::string(buf));
        bufferevent_setcb(bev, ServerBase1<SESSION>::cb_read, nullptr,
                ServerBase1<SESSION>::cb_event, (void*) s);
        bufferevent_enable(bev, EV_READ);
    }

private:
    static void cb_read(struct bufferevent * /*bev*/, void *session) {
        Session* s = (Session*) session;
        s->OnRead();
    }
    static void cb_event(struct bufferevent * /*bev*/, short what,
            void *session) {
        Session* s = (Session*) session;
        s->OnEvent(what);
    }

private:
    // 监听回调函数，@fd为新客户端的fd.
    static void cb_listen(evconnlistener* /*listener*/, int fd, sockaddr* saddr,
            int socklen, void* arg) {
        ((ServerBase1<SESSION>*) arg)->th_pool_->dispatch(
                std::bind(&ServerBase1<SESSION>::accept,
                        (ServerBase1<SESSION>*) arg, std::placeholders::_1, fd,
                        saddr, socklen));
    }

    static void cb_SIGINT(int sig, short int, void * arg) {
        ServerBase1<SESSION>* t = (ServerBase1<SESSION>*) arg;
        printf("signal[%d] \n", sig);
        t->stop();
    }

private:
    uint16_t port_;
    evconnlistener* listener_;
    event_base* event_base_;
    event* signal_event_;
    ThreadPool* th_pool_;
};

////////////////////////////// ServerBase2 //////////////////////////////
template<class SESSION>
class ServerBase2 {
public:
    ServerBase2(uint16_t port) {
        port_ = port;
        listenfd_ = -1;
        event_base_ = nullptr;
        listen_event_ = nullptr;
        signal_event_ = nullptr;
        th_pool_ = nullptr;
    }

    ~ServerBase2() {
        if (listenfd_ != -1) {
            close(listenfd_);
            listenfd_ = -1;
        }
        if (listen_event_ != nullptr) {
            event_free(listen_event_);
            listen_event_ = nullptr;
        }
        if (signal_event_ != nullptr) {
            event_free(signal_event_);
            signal_event_ = nullptr;
        }
        if (event_base_ != nullptr) {
            event_base_free(event_base_);
            event_base_ = nullptr;
        }
        if (th_pool_ != nullptr) {
            delete th_pool_;
            th_pool_ = nullptr;
        }
    }

    ServerBase2(const ServerBase2&) = delete;
    ServerBase2& operator =(const ServerBase2&) = delete;

public:
    bool init() {
        // thread pool
        th_pool_ = new ThreadPool(8);
        th_pool_->init();

        // listen
        listenfd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (-1 == listenfd_) {
            printf("socket failed \n");
            return false;
        }
        // set NONBLOCK
        int flags = ::fcntl(listenfd_, F_GETFL);
        flags |= O_NONBLOCK;
        ::fcntl(listenfd_, F_SETFL);
        // set reuse
        int reuse = 1;
        if (0
                != setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR,
                        (const void*) &reuse, sizeof(reuse))) {
            printf("setsockopt failed \n");
            return false;
        }
        struct sockaddr_in addr;
        memset(&addr, 0x00, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
//		addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);
        if (0
                != ::bind(listenfd_, (struct sockaddr*) &addr,
                        sizeof(sockaddr))) {
            printf("::bind failed \n");
            return false;
        }
        if (0 != ::listen(listenfd_, 0)) {
            printf("::listen failed \n");
            return false;
        }

        // listen event
        event_base_ = event_base_new();
        if (event_base_ == nullptr) {
            printf("event_base_new failed \n");
            return false;
        }
        listen_event_ = event_new(event_base_, listenfd_, EV_READ | EV_PERSIST,
                cb_listen, this);
        if (listen_event_ == nullptr) {
            printf("event_new failed \n");
            return false;
        }
        if (0 != event_add(listen_event_, nullptr)) {
            printf("event_add failed \n");
            return false;
        }

        // 信号处理
        signal_event_ = evsignal_new(event_base_, SIGINT, cb_SIGINT,
                (void* )this);
        if (signal_event_ == nullptr) {
            printf("evsignal_new failed \n");
            return false;
        }
        if (0 != event_add(signal_event_, nullptr)) {
            printf("event_add SIGINT failed \n");
            return false;
        }

        // event_set_fatal_callback(event_fatal_callback);
        return true;
    }

    bool start() {
        th_pool_->start();
        event_base_loop(event_base_, 0);
        return true;
    }
    bool stop() {
        timeval tv = { 1, 0 };
        while (0 != event_base_loopexit(event_base_, &tv)) {
            sleep(1);
        }

        th_pool_->stop();
        return true;
    }

private:
    void accept(Thread* t, evutil_socket_t fd, short /*event*/) {
        struct sockaddr_in peer;
        memset(&peer, 0x00, sizeof(sockaddr_in));
        socklen_t len = sizeof(struct sockaddr);
        int clientfd = ::accept(fd, (struct sockaddr*) &peer, &len);
        if (clientfd == -1) {
            return;
        }

        char buf[32];
        sprintf(buf, "%s:%d", inet_ntoa(peer.sin_addr), htons(peer.sin_port));
        printf("[%lu] accept: [%s] fd[%d]\n", t->get_thread_id(), buf,
                clientfd);

        struct bufferevent* bev = bufferevent_socket_new(t->get_event_base(),
                clientfd, BEV_OPT_CLOSE_ON_FREE);
        if (bev == nullptr) {
            printf("bufferevent_socket_new err, fd[%d] \n", clientfd);
            return;
        }

        Session* s = new SESSION(t->get_thread_id(), clientfd, bev,
                std::string(buf));

        bufferevent_setcb(bev, cb_read, nullptr, cb_event, (void*) s);
        bufferevent_enable(bev, EV_READ);
    }

private:
    static void cb_read(struct bufferevent * /*bev*/, void *session) {
        Session* s = (Session*) session;
        s->OnRead();
    }
    static void cb_event(struct bufferevent * /*bev*/, short what,
            void *session) {
        Session* s = (Session*) session;
        s->OnEvent(what);
    }

private:
    // fd为监听fd
    static void cb_listen(evutil_socket_t fd, short event, void * arg) {
        // 经过bind后，accept会在Thread中调用
        ((ServerBase2*) arg)->th_pool_->dispatch(
                std::bind(&ServerBase2<SESSION>::accept, (ServerBase2*) arg,
                        std::placeholders::_1, fd, event));
    }
    static void cb_SIGINT(int sig, short int, void * arg) {
        ServerBase2* t = (ServerBase2*) arg;
        printf("signal[%d] \n", sig);
        t->stop();
    }

private:
    uint16_t port_;
    int32_t listenfd_;
    event_base* event_base_;
    event* listen_event_;
    event* signal_event_;
    ThreadPool* th_pool_;
};

#endif /* __SHARK_SERVER_H__ */
