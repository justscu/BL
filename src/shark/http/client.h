#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <inttypes.h>
#include <string>
#include <event2/http.h>
#include <event2/event.h>
#include <event2/http_struct.h>
#include "../log/log.h"


class Connection;
class Request;
class Parser;

struct CURI {
public:
	evhttp_cmd_type   m;
	std::string       url;
public:
	CURI(evhttp_cmd_type md, const std::string &u) { m = md; url = u;}
   ~CURI() {}
	CURI(const CURI& c) {
		m   = c.m;
		url = c.url;
	}
	CURI& operator= (const CURI&) = delete;
};

class Client {
public:
	std::string  ip_;
	uint16_t     port_;
	event_base  *event_base_;
	event       *event_;
	Connection  *connection_;
	Parser      *parser_;
	std::vector<CURI> vec_uri_;
public:
	Client(const std::string &ip, uint16_t port, event_base *ev, Parser *parser);
   ~Client();
	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;

public:
	bool   init();
	bool unInit();
	bool start(long int second = 1, long int usecond = 0);
	void push(const CURI & uri);
	void push(const std::string &url, const evhttp_cmd_type m = EVHTTP_REQ_GET);

private:
	static void evt_cb(evutil_socket_t, short, void *ptr);
};

class Connection {
public:
	Client              *client_;
	evhttp_connection   *conn_;

public:
	Connection(Client *c): client_(c), conn_(nullptr) {}
   ~Connection() {}
	Connection(const Connection&) = delete;
	Connection& operator= (const Connection&) = delete;
public:
	bool   init();
	bool unInit();
	bool bClosed() { return conn_ == nullptr;}
	bool reConn()  { return init();}
public:
	static void close_cb(struct evhttp_connection *, void *);
};

class Request {
public:
	evhttp_request *ev_req_;
	Parser         *parser_;
public:
	Request(Parser *parser): ev_req_(nullptr), parser_(parser) {}
	virtual ~Request() {}
    Request(const Request&) = delete;
    Request& operator= (const Request &) = delete;

public:
	bool   init();
	bool unInit();
	bool request(evhttp_connection *conn, const CURI &uri);
private:
	static void req_cb(evhttp_request *req, void *);
};

class Parser {
public:
	virtual ~Parser() {}
public:
	virtual void handle(evhttp_request *req) = 0;
};

#endif /*__HTTP_CLIENT_H__*/
