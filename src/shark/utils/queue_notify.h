#ifndef __UTILS_QUEUE_NOTIFY_H__
#define __UTILS_QUEUE_NOTIFY_H__

#include <list>
#include <functional>
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "../log/log.h"
#include "lock.h"

namespace UTILS {

// 带通知功能的队列
template<class T>
class QueueNotify {
public:
	QueueNotify(const typename std::list<T>::size_type size, event_base *eb, std::function<void(T&)> cb)
		: list_size_(size)
		, event_base_(eb)
		, event_(nullptr)
		, dispute_cb_(cb) {
	}
	~QueueNotify() {
	}
	QueueNotify(const QueueNotify&) = delete;
	QueueNotify& operator=(const QueueNotify&) = delete;

public:
	bool init() {
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

	bool unInit() {
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
	bool notify(const T & t) {
		mutex_.Acquire();
		bool del = (list_size_ == list_.size());
		if (del) {
			list_.pop_front(); //　丢弃最前面的一条
		}
		list_.push_back(t);
		mutex_.Release();
		if (del) {
			INFO("list is full, drop the front");
		}
		::write(fd_[1], "A", 1);
		return true;
	}

private:
	static void notify_cb(evutil_socket_t /*fd*/, short int /*which*/, void *arg) {
		if (arg == nullptr) {
			ERROR("arg is null");
			return;
		}
		QueueNotify *ptr = (QueueNotify *)arg;
		char buf;
		::read(ptr->fd_[0], &buf, 1);

		std::list<T> l;
		ptr->mutex_.Acquire();
		l.assign(ptr->list_.begin(), ptr->list_.end());
		ptr->mutex_.Release();

		for (auto &it : l) {
			ptr->dispute_cb_(it);
		}
	}

private:
	Mutex                                    mutex_;
	std::list<T>                             list_;
	const typename std::list<T>::size_type   list_size_;

	evutil_socket_t   fd_[2]; // 0读1写
	event_base       *event_base_;
	event            *event_;
	std::function<void(T&)> dispute_cb_;
};

}


#endif /* __UTILS_QUEUE_NOTIFY_H__ */
