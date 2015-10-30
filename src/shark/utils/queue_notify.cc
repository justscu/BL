
#include <list>
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "../log/log.h"
#include "lock.h"
#include "queue_notify.h"

namespace UTILS {
#if 0
template<class T>
bool QueueNotify<T>::init() {
	ERROR_CMP_RET(event_base_ == nullptr, false, "event_base_ is null");
	if (0 != evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fd_)) {
		ERROR("evutil_socketpair failed");
		return false;
	}
	evutil_make_socket_nonblocking(fd_[0]);
	evutil_make_socket_nonblocking(fd_[1]);
	// event
	event_ = event_new(event_base_, fd_[0], EV_READ | EV_PERSIST, notify_cb, this);
	ERROR_CMP_RET(nullptr == event_, false, "event_new failed");
	// event add
	if (0 != event_add(event_, nullptr)) {
		ERROR("event_add failed");
		return false;
	}

	return true;
}

template<class T>
bool QueueNotify<T>::unInit() {
	// 关闭read/write
	if (fd_[0] != -1) {
		close(fd_[0]);
		fd_[0] = -1;
	}
	if (fd_[1] != -1) {
		close(fd_[1]);
		fd_[1] = -1;
	}

	if (event_ != nullptr) {
		event_free(event_);
		event_ = nullptr;
	}
	return true;
}
#endif
}
