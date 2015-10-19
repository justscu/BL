#ifndef __SHARK_t2_H__
#define __SHARK_t2_H__

#include "session.h"

class t2: public Session {
public:
	t2(pthread_t tid, evutil_socket_t fd, struct bufferevent* bev, const std::string& peer):Session(tid, fd, bev, peer) {

	}

	virtual void OnRead () {
		char buf[256] = {0};
		bufferevent_read(bev_, buf, sizeof(buf));
		printf("[%lu] t2 recv:%s", thread_id_, buf);

		std::string b("t2 echo: ");
		b += buf;
		bufferevent_write(bev_, b.c_str(), b.length());
		if (0 == strncmp(buf, "bye", 3)) {
			bufferevent_free(bev_); // 在调用bufferevent_free时，会自动close掉该fd
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

		printf("[%lu] t2 what[%s] \n", thread_id_, wh.c_str());

		bufferevent_free(bev_);
	}
};

#endif /*__SHARK_t2_H__*/
