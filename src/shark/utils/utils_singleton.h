#pragma once


///////////////////////////////
//
// model 1: 不推荐用
//
// 在真正调用的时候才进行初始化
//
///////////////////////////////
class UtilsSingleton1 {
public:
    UtilsSingleton1* get_instance() {
        if (!instance_) {
            mutex_.lock();
            instance_ = new UtilsSingleton1;
            mutex_.unlock();
        }

        return instance_;
    }

private:
    UtilsSingleton1() {}
    UtilsSingleton1(const UtilsSingleton1&) = delete;
    UtilsSingleton1& operator=(const UtilsSingleton1&) = delete;

private:
    static std::mutex          mutex_;
    static UtilsSingleton1 *instance_;
};


///////////////////////////////
//
// model 2: 推荐使用
//
// 使用static局部变量的特点，在初始化时就会被初始化
//
///////////////////////////////
class UtilsSingleton2 {
public:
    UtilsSingleton2& get_instance() {
        static UtilsSingleton2 ins;
        return ins;
    }

private:
    UtilsSingleton2() {}
    UtilsSingleton2(const UtilsSingleton2&) = delete;
    UtilsSingleton2& operator=(const UtilsSingleton2&) = delete;
};
