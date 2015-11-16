#include <event2/buffer.h>
#include "client.h"


Client::Client(const std::string &ip, uint16_t port, event_base *ev, Parser *parser) {
	ip_         = ip;
	port_       = port;
	event_base_ = ev;
	event_      = nullptr;
	connection_ = nullptr;
	parser_     = parser;
}

Client::~Client() {
	unInit();
}

bool Client::init() {
	if (event_ != nullptr) {
		event_free(event_);
	}
	event_ = event_new(event_base_, -1, EV_TIMEOUT | EV_PERSIST, evt_cb, this);
	if (event_ == nullptr) {
		ERROR("event_new err[%s]", strerror(errno));
		return false;
	}
	if (connection_ != nullptr) {
		connection_->unInit();
		delete connection_;
	}
	connection_ = new Connection(this);
	if (connection_ == nullptr) {
		ERROR("new Connection failed");
		return false;
	}
	connection_->init();
	if (parser_ == nullptr) {
		ERROR("parser_ = nullptr");
		return false;
	}
	return true;
}

bool Client::unInit() {
	if (connection_ != nullptr) {
		connection_->unInit();
		delete connection_;
		connection_ = nullptr;
	}
	if (event_ != nullptr) {
		event_free(event_);
		event_ = nullptr;
	}
	return true;
}

bool Client::start(long int s, long int us) {
	timeval tv = {s, us};
	return 0 == event_add(event_, &tv);
}

void Client::push(const CURI & uri) {
	vec_uri_.push_back(uri);
}

void Client::push(const std::string &u, const evhttp_cmd_type m) {
	push(CURI(m, u));
}

void Client::evt_cb(evutil_socket_t, short, void *ptr) {
	if (ptr == nullptr) {
		ERROR("evt_cb. ptr = nullptr");
		return;
	}
	Client *cli = (Client*)ptr;
	for (auto &it : cli->vec_uri_) {
		Request *req = new Request(cli->parser_); // 在此处申请内存
		if (req == nullptr) {
			ERROR("new Request failed.");
			continue;
		}
		if (req->init()) {
			req->request(cli->connection_->conn_, it);
		}
	}
}

// Connection
bool Connection::init() {
	if (conn_ != nullptr) {
		evhttp_connection_free(conn_);
	}
	conn_ = evhttp_connection_base_new(client_->event_base_, nullptr, client_->ip_.c_str(), client_->port_);
	if (conn_ == nullptr) {
		ERROR("evhttp_connection_base_new[%s:%d] err[%s]", client_->ip_.c_str(), client_->port_, strerror(errno));
		return false;
	}
	evhttp_connection_set_closecb(conn_, close_cb, this);
	return true;
}

bool Connection::unInit() {
	evhttp_connection_free(conn_);
	conn_ = nullptr;
	return true;
}

void Connection::close_cb(struct evhttp_connection *, void *) {
	INFO("Connection::close_cb");
}

// Request
bool Request::init() {
	if (ev_req_ != nullptr) {
		evhttp_request_free(ev_req_);
	}
	ev_req_ = evhttp_request_new(req_cb, this);
	if (ev_req_ == nullptr) {
		ERROR("evhttp_request_new err[%s]", strerror(errno));
		return false;
	}
	// http header
	evhttp_add_header(ev_req_->output_headers, "Host", "shark");
	evhttp_add_header(ev_req_->output_headers, "User-Agent", "libEvent");
	evhttp_add_header(ev_req_->output_headers, "Connection", "keepalive");

	return true;
}

bool Request::unInit() {
	if (ev_req_ != nullptr) {
		evhttp_request_free(ev_req_);
	}
	return true;
}

bool Request::request(evhttp_connection *conn, const CURI &uri) {
	if (ev_req_ == nullptr) {
		ERROR("ev_req_ = nullptr");
		return false;
	}
	if (conn == nullptr) {
		ERROR("conn = nullptr");
		return false;
	}
	if(0 != evhttp_make_request(conn, ev_req_, uri.m, uri.url.c_str())) {
		ERROR("evhttp_make_request[%d:%s] err[%s]", uri.m, uri.url.c_str(), strerror(errno));
		return false;
	}
	return true;
}

void Request::req_cb(evhttp_request *req, void *ptr) {
	if (ptr == nullptr) {
		ERROR("ptr=nullptr");
		return;
	}
	Request *r = (Request*)ptr;
	r->parser_->handle(req);
	delete r; // 在该处释放内存
}

///////////////////////////////////////////// for test
class MyParser : public Parser {
public:
	virtual void handle(evhttp_request *req) {
		INFO("MyParser: [%d][%s]", req->response_code, req->response_code_line);
		// 解析从 http server 过来的数据
		evbuffer *buf = req->input_buffer;
		size_t len = evbuffer_get_length(buf);
		char *p = new char[len+1];
		evbuffer_copyout(buf, p, len);
		p[len] = 0;
		std::cout << p << std::endl;
		delete []p;
	}
};

int t_client() {
	event_base *e = event_base_new();
	MyParser parser;
	Client c("10.198.194.26", 8080, e, &parser);
	c.init();
	// push
	c.push("/_status");
	c.push("/_health_check");
	c.push("/brand/?bids=53930,14569907,14570687,319188&fields=to_warehouse,warehouse");
	c.push("/brand/?bids=53930,14569907,14570687,319188&fields=pids");
	c.push("/product/?pids=6051582,6051583,6051584&fields=icons,pv,uv,sizes");

	c.start(3, 0);
	event_base_loop(e, 0);
	return 0;
}
