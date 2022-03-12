#include <atomic>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 自旋锁
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilSpinLock {
public:
    explicit UtilSpinLock(std::atomic_flag &flag) : flag_(flag) {
        while (flag_.test_and_set(std::memory_order_acquire)) {}
    }

    ~UtilSpinLock() {
        flag_.clear(std::memory_order_release);
    }

    UtilSpinLock(const UtilSpinLock&) = delete;
    UtilSpinLock& operator=(const UtilSpinLock&) = delete;

private:
    std::atomic_flag &flag_;
};
