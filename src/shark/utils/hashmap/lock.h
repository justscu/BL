#ifndef __UTILS_HASHMAP_LOCK_H__
#define __UTILS_HASHMAP_LOCK_H__

#include <pthread.h>
#include <assert.h>
#include "../../log/log.h"

namespace HASHMAP {

class RWLock {
public:
	RWLock() {
		lock_ = nullptr;
		size_ = 0;
	}
	~RWLock() {
		unInit();
	}
	RWLock(const RWLock&)            = delete;
	RWLock& operator=(const RWLock&) = delete;

public:
	bool init(uint32_t size) {
		size_ = size;
		lock_ = new pthread_rwlock_t[size_];
		assert(lock_ != nullptr);
		for (uint32_t i = 0; i < size_; ++i) {
			if ( 0 != pthread_rwlock_init(lock_+i, nullptr)) {
				ERROR("pthread_rwlock_init err[%s]", strerror(errno));
				return false;
			}
		}
		return true;
	}
	bool unInit() {
		for (uint32_t i = 0; i < size_; ++i) {
			if ( 0 != pthread_rwlock_destroy(lock_+i)) {
				ERROR("pthread_rwlock_destroy err[%s]", strerror(errno));
				return false;
			}
		}
		if (lock_ != nullptr) {
			delete [] lock_;
			lock_ = nullptr;
		}
		size_ = 0;
		return true;
	}

	bool rdlock(uint32_t i) {
		return 0 == pthread_rwlock_rdlock(lock_+(i%size_));
	}
	bool wrlock(uint32_t i) {
		return 0 == pthread_rwlock_wrlock(lock_+(i%size_));
	}
	bool unlock(uint32_t i) {
		return 0 == pthread_rwlock_unlock(lock_+(i%size_));
	}
	uint32_t size() const { return size_; }

private:
	pthread_rwlock_t *lock_;
	uint32_t          size_; // 锁的个数

};

}
#endif /*__UTILS_HASHMAP_LOCK_H__*/
