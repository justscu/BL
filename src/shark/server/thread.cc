
#include <iostream>
#include <string.h>
#include "thread.h"

Thread::~Thread() {
	// 关闭read/write
	if (fd_[0] != -1) {
		close(fd_[0]);
		fd_[0] = -1;
	}
	if (fd_[1] != -1) {
		close(fd_[1]);
		fd_[1] = -1;
	}
	if (thread_ != nullptr) {
		delete thread_;
		thread_ = nullptr;
	}
	if (event_ != nullptr) {
		event_free(event_);
		event_ = nullptr;
	}
	if (event_base_ != nullptr) {
		event_base_free(event_base_);
		event_base_ = nullptr;
	}
}

bool Thread::start() {
	if (thread_ != nullptr) {
		std::cout << "thread_ not nullptr" << std::endl;
		return false;
	}
	thread_ = new std::thread(std::bind(&Thread::cb_thread, this));
	if (thread_ == nullptr) {
		return false;
	}

	thread_id_ = thread_->native_handle();
	return true;

}

bool Thread::stop() {
	if (thread_ != nullptr) {
		return 1 == write(fd_[1], "E", 1);
	}
	return true;
}

bool Thread::join() {
	if (thread_ != nullptr) {
		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}
	return true;
}

void Thread::OnRead(struct bufferevent *bev) {
	char buf[256] = {0};
	bufferevent_read(bev, buf, sizeof(buf));
	printf("[%lu] Thread recv:%s", get_thread_id(), buf);

	std::string b("Thread echo: ");
	b += buf;
	bufferevent_write(bev, b.c_str(), b.length());
	if (0 == strncmp(buf, "bye", 3)) {
		bufferevent_free(bev); // 在调用bufferevent_free时，会自动close掉该fd
	}
}

void Thread::OnEvent(struct bufferevent *bev, short what) {
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

	printf("[%lu] Thread what[%s] \n", get_thread_id(), wh.c_str());

	bufferevent_free(bev);
}

bool Thread::dispatch(std::function<void(Thread*)> func) {
	// lock
	lock_.wrlock();
	funcs_.push_back(func);
	lock_.unlock();

	return 1 == write(fd_[1], "A", 1);
}

std::thread::id Thread::get_id() const {
	return thread_->get_id();
}

// 返回线程id
pthread_t Thread::get_thread_id() const {
	// return thread_->native_handle();
	// return pthread_self();
	return thread_id_;
}


// 线程回调函数
void Thread::cb_thread() {
	if (0 != evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fd_)) {
		std::cout << "evutil_socketpair failed" << std::endl;
		return;
	}
	printf("socketpair, thread[%lu]: fd_[0]=%d, fd_[1]=%d\n", get_thread_id(), fd_[0], fd_[1]);
	evutil_make_socket_nonblocking(fd_[0]);
	evutil_make_socket_nonblocking(fd_[1]);

	// event base
	event_base_ = event_base_new();
	if (nullptr == event_base_) {
		std::cout << "event_base_new failed" << std::endl;
		return;
	}
	// event
	event_ = event_new(event_base_, fd_[0], EV_READ | EV_PERSIST, cb_read, this);
	if (nullptr == event_) {
		std::cout << "event_new failed" << std::endl;
		return;
	}
	// event add
	if ( 0 != event_add(event_, nullptr)) {
		std::cout << "event_add failed" << std::endl;
		return;
	}
	// event loop
	event_base_loop(event_base_, 0);
}

// 读回调
void Thread::cb_read(evutil_socket_t fd, short int /*which*/, void *arg) {
	Thread* t = (Thread*)arg;
	if (nullptr == arg) {
		return;
	}

	char buf;
	int n = read(fd, &buf, 1);
	if (n != 1) {
		return;
	}

	if (buf == 'A') {
		do {
			t->lock_.wrlock();
			if (t->funcs_.empty()) {
				t->lock_.unlock();
				break;
			}
			std::function<void (Thread*)> func = t->funcs_.front();
			t->funcs_.pop_front();
			t->lock_.unlock();

			if(func != nullptr) {
				func(t);
			}
		} while(1);

	} else if (buf == 'E') { // exist
		event_base_loopbreak(t->event_base_);
		printf("thread[%lu] event_base_loopbreak \n", t->get_thread_id());

	} else {
		std::cout << "unknown " << buf << std::endl;
	}

	return;
}
