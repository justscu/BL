// 用libevhtp，实现http server
// 
#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <pthread.h>
#include <string>
#include <evhtp.h>
#include <event2/event.h>
#include "../log/log.h"

namespace HTTP {

// http server
class Server {
public:
    Server(const int threads_num, const std::string &name,
            const std::string &ver) :
            threads_num_(threads_num), name_(name + ver) {
        pv_ = 0;
        ev_base_ = nullptr;
        ev_htp_ = nullptr;
    }
    virtual ~Server() {
    }
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool init(const std::string &host, uint16_t port);
    bool start();
    bool stop();
    bool unInit();

    virtual void set_cb();
    // status
    virtual void status(evbuffer *buf);
    // health_check
    virtual void health_check(evbuffer *buf);

private:
    // http 头部
    void http_header(evhtp_request_t *req, const std::string &mime,
            const std::string &charset = "utf-8");
    static evhtp_res http_pre_accept_cb(evhtp_connection_t *, void *ptr);
    static void http_gen_cb(evhtp_request_t *req, void *ptr);
    static void http_status_cb(evhtp_request_t *req, void *ptr);
    static void http_health_check_cb(evhtp_request_t *req, void *ptr);

private:
    int32_t threads_num_; // 开启多少个http线程
    std::string name_;        // server name & ver.
    uint32_t pv_;
    event_base *ev_base_;
    evhtp_t *ev_htp_;
};

}

#endif /*__HTTP_SERVER_H__*/
