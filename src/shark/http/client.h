#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <inttypes.h>
#include <string>
#include <evhtp.h>
#include <event2/event.h>
#include "../log/log.h"
#include "../utils/queue_notify.h"

namespace HTTP {
//static evhtp_res
//print_data(evhtp_request_t * req, evbuf_t * buf, void * arg) {
//    printf("Got %zu bytes\n", evbuffer_get_length(buf));
//
//    return EVHTP_RES_OK;
//}
//static evhtp_res
//print_new_chunk_len(evhtp_request_t * req, uint64_t len, void * arg) {
//    printf("started new chunk, %" PRIu64 "  bytes\n", len);
//
//    return EVHTP_RES_OK;
//}
//
//static unsigned short int f() {
//	return 0;
//}

static void
request_cb(evhtp_request_t * req, void * /*arg*/) {
    printf("hi %zu\n", evbuffer_get_length(req->buffer_in));
}


static evhtp_res print_data() {
	INFO("print_data");
	return 0;
}
static evhtp_res print_new_chunk_len() {
	INFO("print_new_chunk_len");
	return 0;
}
static evhtp_res print_chunk_complete () {
	INFO("print_chunk_complete");
	return 0;
}
static evhtp_res print_chunks_complete() {
	INFO("print_chunks_complete");
	return 0;
}

class Client {
public:
	Client() {
		ev_base_ = nullptr;
		conn_    = nullptr;
		request_ = nullptr;
		queue_   = nullptr;
	}
	~Client() {
	}
	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;

public:
	bool init(const std::string &ip, uint16_t port) {
		if (ev_base_ == nullptr) {
			ev_base_ = event_base_new();
		}
		ERROR_CMP_RET(ev_base_ == nullptr, false, "event_base_new error");
		if (conn_ == nullptr) {
			conn_ = evhtp_connection_new(ev_base_, ip.c_str(), port);
		}
		ERROR_CMP_RET(conn_ == nullptr, false, "evhtp_connection_new error");
		if (request_ == nullptr) {
			request_ = evhtp_request_new(request_cb, ev_base_);
		}
		ERROR_CMP_RET(request_ == nullptr, false, "evhtp_request_new error");

	    evhtp_set_hook(&request_->hooks, evhtp_hook_on_read, print_data, ev_base_);
	    evhtp_set_hook(&request_->hooks, evhtp_hook_on_new_chunk, print_new_chunk_len, NULL);
	    evhtp_set_hook(&request_->hooks, evhtp_hook_on_chunk_complete, print_chunk_complete, NULL);
	    evhtp_set_hook(&request_->hooks, evhtp_hook_on_chunks_complete, print_chunks_complete, NULL);

	    evhtp_headers_add_header(request_->headers_out, evhtp_header_new("Host",       "ieatfood.net", 0, 0));
	    evhtp_headers_add_header(request_->headers_out, evhtp_header_new("User-Agent", "libevhtp",     0, 0));
	    evhtp_headers_add_header(request_->headers_out, evhtp_header_new("Connection", "close",        0, 0));

	    auto f = [this](const std::string &t) -> void {
	    	int i = evhtp_make_request(this->conn_, this->request_, htp_method_GET, t.c_str());
	    	INFO("evhtp_make_request: [%d] [%s]", i, t.c_str());
	    };

	    queue_ = new (std::nothrow) UTILS::QueueNotify<std::string>(128, ev_base_, f);
	    ERROR_CMP_RET(queue_ == nullptr, false, "new QueueNotify failed");
	    return queue_->init();
	}

	bool unInit() {
		if (queue_ != nullptr) {
			queue_->unInit();
			delete queue_;
			queue_ = nullptr;
		}
		if (ev_base_ != nullptr) {
			event_base_free(ev_base_);
			ev_base_ = nullptr;
		}
		return true;
	}

	static void* loop(void *arg) {
		if (arg != nullptr) {
			INFO("loop ready");
			event_base_loop(((Client*)arg)->ev_base_, 0);
			INFO("event_base_loop");
			return nullptr;
		}
		ERROR("arg is nullptr");
		return nullptr;
	}
	void loop_exit() {
		timeval tv = {0, 100};
		int i = event_base_loopexit(ev_base_, &tv);
		INFO("event_base_loopexit [%d]", i);
	}

	bool request(const std::string &url) {
		if (queue_ != nullptr) {
			return queue_->notify(url);
		}
		ERROR("queue_ is nullptr");
		return false;
	}

private:
	evbase_t            *ev_base_;
	evhtp_connection_t  *conn_;
	evhtp_request_t     *request_;
	UTILS::QueueNotify<std::string>  *queue_;
};

} // namespace HTTP
#endif /*__HTTP_CLIENT_H__*/
