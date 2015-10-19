/*
 * session.h
 *
 *  Created on: 2015年9月9日
 *      Author: justscu
 */

#ifndef __SHARK_SESSION_H__
#define __SHARK_SESSION_H__

#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include <string>

// 每个socket，对应一个session
class Session {
public:
	Session(pthread_t tid, evutil_socket_t fd, struct bufferevent* bev, const std::string& peer):
		thread_id_(tid), fd_(fd), bev_(bev), peer_addr_(peer) {

	}
	virtual ~Session() {
		if (bev_ != nullptr) {
			bufferevent_free(bev_);
			bev_ = nullptr;
		}
	}

public:
	virtual void OnRead () {
		char buf[256] = {0};
		bufferevent_read(bev_, buf, sizeof(buf));
		printf("[%lu] Session recv:%s", thread_id_, buf);

		std::string b("Session echo: ");
		b += buf;
		bufferevent_write(bev_, b.c_str(), b.length());
		if (0 == strncmp(buf, "bye", 3)) {
			bufferevent_free(bev_); // 在调用bufferevent_free时，会自动close掉该fd
			bev_ = nullptr;
		}
	}

	virtual void OnEvent(short what) {
		std::string wh;
		if (what & BEV_EVENT_READING) {
			wh.append("BEV_EVENT_READING ");
		}
		if (what & BEV_EVENT_WRITING) {
			wh.append("BEV_EVENT_WRITING ");
		}
		if (what & BEV_EVENT_EOF) {
			wh.append("BEV_EVENT_EOF ");
		}
		if (what & BEV_EVENT_ERROR) {
			wh.append("BEV_EVENT_ERROR ");
		}
		if (what & BEV_EVENT_TIMEOUT) {
			wh.append("BEV_EVENT_TIMEOUT ");
		}
		if (what & BEV_EVENT_CONNECTED) {
			wh.append("BEV_EVENT_CONNECTED ");
		}

		printf("[%lu] Session what[%s] \n", thread_id_, wh.c_str());

		bufferevent_free(bev_);
		bev_ = nullptr;
	}

public:
	evutil_socket_t     get_fd()  const { return fd_;}
	struct bufferevent* get_bev() const {return bev_;}
	const std::string&  get_peer_addr() const {return peer_addr_;}

public:
	const pthread_t           thread_id_;
	const evutil_socket_t     fd_; //客户端连接上来端fd
	const std::string         peer_addr_;

protected:
	struct bufferevent*       bev_;
};



#endif /* __SHARK_SESSION_H__ */
