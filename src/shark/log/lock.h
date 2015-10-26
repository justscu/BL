#ifndef __LOG_LOCK_H__
#define __LOG_LOCK_H__

#include <pthread.h>

namespace LOG {
class Mutex {
public:
	Mutex() {
		pthread_mutex_init(&mutex_, nullptr);
	}
	~Mutex() {
		pthread_mutex_destroy(&mutex_);
	}
	Mutex(const Mutex &) = delete;
	Mutex& operator= (const Mutex &) = delete;

	// return: 0, success; else failed.
	int Acquire(bool block = true) {
		if (block) {
			return pthread_mutex_lock(&mutex_);
		} else {
	       return pthread_mutex_trylock(&mutex_);
		}
	}
	// return: 0, success; else failed.
	int Release() {
		return pthread_mutex_unlock(&mutex_);
	}

private:
	pthread_mutex_t  mutex_;
};

}

#endif /* __LOG_LOCK_H__ */
