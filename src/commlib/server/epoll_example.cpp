#include <sys/epoll.h>
#include <thread>
#include <set>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log.h"
#include "socket.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 使用epoll监听多个端口
// 当有新的连接上来后，开启一个新的线程来处理
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class EpollExam1 {
public:
    bool start(std::set<uint16_t> &listen_ports) {
        std::set<int32_t>  listen_fds;

        if (!create_listen_fds(listen_ports, listen_fds)) { return false; }

        int32_t ep_fd = epoll_create(256);
        if (-1 == ep_fd) {
            log_err("epoll_create(256) failed. %s.", strerror(errno));
            close_fds(listen_fds);
            return false;
        }

        // add.
        for (auto it : listen_fds) {
            struct epoll_event event;
            event.data.fd = it; // listen fd.
            event.events  = EPOLLIN; // read
            if (-1 == epoll_ctl(ep_fd, EPOLL_CTL_ADD, it, &event)) {
                log_err("epoll_ctl [EPOLL_CTL_ADD] failed. %s.", strerror(errno));
                close_fds(listen_fds);
                return false;
            }
        }

        // monitor
        epoll_event   es[64];
        const int32_t es_max = sizeof(es) / sizeof(es[0]);
        while (true) {
            const int32_t cnt = epoll_wait(ep_fd, es, es_max, -1); // block
            if (cnt == -1) {
                if (errno == EINTR) { continue; }
                log_err("epoll_wait return -1. %s.", strerror(errno));
                return false;
            }

            for (int32_t i = 0; i < cnt; ++i) {
                // listen fd called.
                if (listen_fds.end() != listen_fds.find(es[i].data.fd)) {
                    const int32_t fd = SocketOps::accept_new_client(es[i].data.fd);
                    std::thread th(std::bind(&EpollExam1::new_client_thread, this, fd));
                    th.detach();
                }
                else if (es[i].events & EPOLLIN) {
                    char buf[128];
                    ::recv(es[i].data.fd, buf, sizeof(buf), MSG_DONTWAIT);
                    log_dbg("EPOLLIN");
                }
                else if (es[i].events & EPOLLERR) {
                    log_dbg("EPOLLERR");
                }
            }
        } // while

        close_fds(listen_fds);
        return true;
    }

private:
    bool create_listen_fds(const std::set<uint16_t> &listen_ports,
                           std::set<int32_t> &listen_fds) {
        for (auto it : listen_ports) {
            int32_t fd = SocketOps::create_tcp_listen_socket(it);
            if (fd > 0) {
                listen_fds.insert(fd);
            }
        }

        if (listen_fds.empty()) {
            log_info("have no listen fd.");
            return false;
        }
        return true;
    }

    void close_fds(std::set<int32_t> &fds) {
        for (auto it : fds) {
            ::close(it);
        }
    }

    void new_client_thread(int32_t client_fd) {
        //
        char buf[256];
        SocketOps::readn(client_fd, buf, sizeof(buf));
        sleep(5);
        close(client_fd);
    }
};


void test() {
    std::set<uint16_t> listen_ports;
    listen_ports.insert(7001);
    listen_ports.insert(7002);
    listen_ports.insert(7003);
    listen_ports.insert(7004);

    EpollExam1 e;
    e.start(listen_ports);
}
