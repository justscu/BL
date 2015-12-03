#ifndef LOCK_H_
#define LOCK_H_

class RWLock {
public:
    RWLock() {
        pthread_rwlock_init(&rwlock_, nullptr);
    }
    RWLock(const RWLock&) = delete;
    RWLock& operator=(const RWLock&) = delete;

    ~RWLock() {
        pthread_rwlock_destroy(&rwlock_);
    }
public:
    int rdlock() {
        return pthread_rwlock_rdlock(&rwlock_);
    }
    int wrlock() {
        return pthread_rwlock_wrlock(&rwlock_);
    }
    int unlock() {
        return pthread_rwlock_unlock(&rwlock_);
    }
private:
    pthread_rwlock_t rwlock_;
};

#endif /* LOCK_H_ */
