#include "server.h"

namespace HTTP {

bool Server::init(const std::string &host, uint16_t port) {
    if (nullptr == (ev_base_ = event_base_new())) {
        ERROR("event_base_new err");
        return false;
    }
    if (nullptr == (ev_htp_ = evhtp_new(ev_base_, nullptr))) {
        ERROR("evhtp_ew err");
        return false;
    }
    if (0 != evhtp_use_threads(ev_htp_, nullptr, threads_num_, nullptr)) {
        ERROR("evhtp_use_threads err");
        return false;
    }
    if (0 != evhtp_bind_socket(ev_htp_, host.c_str(), port, 1024)) {
        ERROR("evhtp_bind_socket err");
        return false;
    }
    return true;
}

bool Server::start() {
    if (ev_base_ != nullptr) {
        set_cb();
        event_base_loop(ev_base_, 0);
    }
    return true;
}

bool Server::stop() {
    if (ev_base_ != nullptr) {
        event_base_loopbreak(ev_base_);
    }
    return true;
}

bool Server::unInit() {
    if (ev_htp_ != nullptr) {
        evhtp_free(ev_htp_);
        ev_htp_ = nullptr;
    }
    if (ev_base_ != nullptr) {
        event_base_free(ev_base_);
        ev_base_ = nullptr;
    }
    return false;
}

void Server::set_cb() {
    evhtp_set_pre_accept_cb(ev_htp_, http_pre_accept_cb, this);
    evhtp_set_gencb(ev_htp_, http_gen_cb, this);
    // status
    evhtp_set_cb(ev_htp_, "/status", http_status_cb, this);
    evhtp_set_cb(ev_htp_, "/health_check", http_health_check_cb, this);
}
// status
void Server::status(evbuffer *buf) {
    std::string s("status ok! \n");
    evbuffer_add(buf, s.c_str(), s.length());
    evbuffer_add_printf(buf, "pv : %d (%ld)\n", pv_, pthread_self());
    return;
}
// health_check
void Server::health_check(evbuffer *buf) {
    std::string s("health check ok!");
    evbuffer_add(buf, s.c_str(), s.length());
    return;
}

evhtp_res Server::http_pre_accept_cb(evhtp_connection_t *, void *ptr) {
    if (ptr != nullptr) {
        __sync_fetch_and_add(&((Server*) ptr)->pv_, 1);
    }
    return EVHTP_RES_OK;
}
// http 头部
void Server::http_header(evhtp_request_t *req, const std::string &mime,
        const std::string &charset) {
    std::string type(mime);
    type += "; charset=" + charset;
    evhtp_header_key_add(req->headers_out, "Content-Type", 0); //0, assign
    evhtp_header_val_add(req->headers_out, type.c_str(), 1); //1, copy
    evhtp_header_key_add(req->headers_out, "Server", 0);
    evhtp_header_val_add(req->headers_out, name_.c_str(), 1);
    evhtp_header_key_add(req->headers_out, "Connection", 0);
    evhtp_header_val_add(req->headers_out, "close", 0);
}

void Server::http_gen_cb(evhtp_request_t *req, void *ptr) {
    if (ptr != nullptr) {
        ((Server*) ptr)->http_header(req, "text/plain");
        evbuffer_add_printf(req->buffer_out, "%s",
                ((Server*) ptr)->name_.c_str());
    }
    return evhtp_send_reply(req, EVHTP_RES_OK);
}
void Server::http_status_cb(evhtp_request_t *req, void *ptr) {
    if (ptr != nullptr) {
        ((Server*) ptr)->http_header(req, "text/plain");
        ((Server*) ptr)->status(req->buffer_out);
    }
    return evhtp_send_reply(req, EVHTP_RES_OK);
}
void Server::http_health_check_cb(evhtp_request_t *req, void *ptr) {
    if (ptr != nullptr) {
        ((Server*) ptr)->http_header(req, "text/plain");
        ((Server*) ptr)->health_check(req->buffer_out);
    }
    return evhtp_send_reply(req, EVHTP_RES_OK);
}

}
