#ifndef __UTILS_HASHMAP_HSET_H__
#define __UTILS_HASHMAP_HSET_H__

#include "basket.h"
#include "lock.h"

namespace HASHMAP {

template <class ID, class V>
class HSet {
private:
	Basket<ID, V>  b_;
	RWLock        *lock_;
public:
	HSet() {lock_ = nullptr;}
   ~HSet() {unInit();}
	bool init(uint32_t size, std::function<ID (const V&)> func) {
		return b_.init(size, func);
	}
	bool unInit() {
		return b_.unInit();
	}
public:
	// 单线程，不需要设置lock，多线程中才需要
	void set_lock(const RWLock *lock) {
		lock_ = lock;
	}
	bool set(const V & k) {
		uint32_t i = b_.get_basket(k);
		if(lock_ != nullptr) { lock_->wrlock(i); }
		V *ptr = (V*)biFind<ID, V>(b_[i], k.id_);
		if (ptr != nullptr) {
			*ptr = k; // 找到该值，直接替换
		} else {
			std::vector<V>& vec = b_[i]; // 返回桶
			vec.push_back(k);
			b_++; // 统计＋1
			if (vec.size() > 1) { // size > 1, 排序
				std::sort(vec.begin(), vec.end(),
						[](const V& a, const V &b) -> bool {return a.id_ < b.id_;}
				);
			}
		}
		if(lock_ != nullptr) { lock_->unlock(i); }
		return true;
	}
	bool get(const ID &id, V &k) const {
		bool ret = false;
		if (b_.data_ == nullptr) return false;
		uint32_t i = b_.get_basket(id);
		if(lock_ != nullptr) { lock_->rdlock(i); }
		V *ptr = (V*)biFind<ID, V>(b_[i], id);
		if (ptr != nullptr) {
			k = *ptr;
			ret = true;
		}
		if(lock_ != nullptr) { lock_->unlock(i); }
		return ret;
	}

	// dump
	bool dump(const std::string &sha) {
		return b_.dump(sha);
	}
	// load from file
	bool load(const std::string &sha) {
		return b_.load(sha);
	}
	// load from vector
	bool load(const std::vector<V> &vec) {
		return b_.load(vec);
	}
	void print() {
		b_.print();
	}
};
//////////////////////////////////// test
struct TS {
	uint32_t id_;
	int a;
	float b;
public:
	TS() {id_ = 0; a = 0; b = 0;}
	TS(uint32_t _1, int _2, float _3) {id_ = _1, a = _2, b = _3;}
	TS(const TS & t) {
		id_ = t.id_, a = t.a, b = t.b;
	}
	TS& operator=(const TS & t) {
		if (this != &t) {
			id_ = t.id_, a = t.a, b = t.b;
		}
		return *this;
	}
public:
	friend std::ostream& operator << (std::ostream & o, const TS & t) {
		o << t.id_ << ":" << t.a << ":" << t.b;
		return o;
	}
};
void test_HSet() {
	std::function<uint32_t(const TS&)> f = [](const TS & s) {
		return s.id_;
	};
	HSet<uint32_t, TS> s;
	s.init(4, f);

	std::vector<TS> vec;
	for (uint32_t i = 0; i < 100; ++i) {
		TS t(i, i-10, i*0.3);
		vec.push_back(t);
	}

	s.load(vec);
	TS ts2;
	if (s.get(1, ts2)) {
		std::cout << ts2 << std::endl;
	}

	if (s.get(12, ts2)) {
		std::cout << ts2 << std::endl;
	}

	TS ts3(12, 100, 0.98567);
	s.set(ts3);

	if (s.get(12, ts2)) {
		std::cout << ts2 << std::endl;
	}

	s.print();
	s.dump("./tSSet.sha");
	s.unInit();

	printf("--------------after-------------------\n");
	HSet<uint32_t, TS> sa;
	sa.init(4, f);
	sa.load("./tSSet.sha");
	sa.print();
	sa.unInit();
}

}
#endif /*__UTILS_HASHMAP_HSET_H__*/
