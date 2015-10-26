// 用libevhtp，实现http server
// 
#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "evhtp.h"
#include "../log/log.h"

namespace HTTP {


// http server
class Server {
public:
	Server(const std::string &host, const uint16_t port, const int threads_num, const std::stirng &name, const std::string &ver)
			:host_(host)
			,port_(port)
			,threads_num_(threads_num)
			,name_(name+ver) {

	}
	~Server() {
	}

	bool init() {
		if (nullptr == (ev_base_ = event_base_new()) {
			ERROR("event_base_new err");
			return false;
		}
		if (nullptr == (ev_htp_ = evhtp_new(ev_base_, nullptr_))) {
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

	bool start() {
		if (ev_base_ != nullptr) {
			event_base_loop(ev_base_, 0);
		}
	}

	bool stop() {
		if (ev_base_ != nullptr) {
			event_base_loopbreak(ev_base_);
		}
	}

	bool unInit() {
		if (ev_htp_ != nullptr) {
			ev_htp_free(ev_htp_);
			ev_htp_ = nullptr;
		}
		if (ev_base_ != nullptr) {
			event_base_free(ev_base_);
			ev_base_ = nullptr;
		}
		return false;
	}

	virtual void set_cb() {
		evhtp_set_pre_accept_cb(evhtp_, http_pre_accept_cb, this);
		evhpt_set_gencb        (evhtp_, http_gen_cb,        this);
		// status
		evhtp_set_cb(evhtp_, "/status",       http_status_cb,       this);
		evhtp_set_cb(evhtp_, "/health_check", http_health_check_cb, this); 
	}
	// status
	virtual void status(evbuffer *buf) {
		evad(buf, "status ok!");
		evpf(buf, "pv : %d \n", pv_);
		return;
	}
	// health_check
	virtual void health_check(evbuffer *buf) {
		evad(buf, "health check ok!");
		return;
	}

private:
	static evhtp_res http_pre_accept_cb() {
		__sync_fetch_and_add(&pv_, 1);
		return EVHTP_RES_OK;
	}
	// http 头部
	void http_header(evhttp_request_t *req, const string &mime, const std::strin &charset="utf-8") {
		std::string type(mime);
		mime += "; charset=" + charset;
		evhtp_header_key_add(req->headers_out, "Content-Type",   0);//0, assign
		evhtp_header_val_add(req->headers_out, type.c_str(),     1);//1, copy
		evhtp_header_key_add(req->headers_out, "Server",         0);
		evhtp_header_val_add(req->headers_out, name_.c_str()     1);
		evhtp_header_key_add(req->headers_out, "Connection",     0);
		evhtp_header_val_add(req->headers_out, "close",          0);
	}
private:
	static void http_gen_cb(evhtp_request_t *req, void *ptr) {
		if (ptr != nullptr) {
			((Server*)ptr)->http_header(req, "text/plain");
		}
		evpf(req->buffer_out, "%s", name.c_str());
		return evhtp_send_reply(req, EVHTP_RES_OK);
	}
    static http_status_cb(evhtp_request_t *req, void *ptr) {
		if (ptr != nullptr) {
	    	((Server*)ptr)->http_header(req, "text/plain");
			((Server*)ptr)->status(req->buffer_out);
		}
		return evhtp_send_reply(req, EVHTP_RES_OK);
	}
	static http_health_check_cb(evhtp_request_t *req, void*) {
		if (ptr != nullptr) {
	    	((Server*)ptr)->http_header(req, "text/plain");
			((Server*)ptr)->health_check(req->buffer_out);
		}
		return evhtp_send_reply(req, EVHTP_RES_OK);
	}
private:
	std::string  name_;        // server name & ver.
	 int32_t     threads_num_; // 开启多少个http线程
	uint32_t     pv_; 
	evhtp_base  *ev_base_;
	evhtp_htp   *ev_htp_;
};

}

#endif /*__HTTP_SERVER_H__*/
