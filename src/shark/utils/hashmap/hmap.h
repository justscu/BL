#ifndef __UTILS_HASHMAP_HMAP_H__
#define __UTILS_HASHMAP_HMAP_H__

#include "basket.h"
#include "lock.h"

namespace HASHMAP {

template <class ID, class V>
class Ivt {
public:
	ID         id_;
	uint32_t   size_;
	V         *offset_;
public:
	Ivt(ID id = 0, uint32_t size = 0, V *v = nullptr) {
		id_ = id; size_ = size; offset_ = v;
	}
	Ivt(const Ivt& ivt) {
		id_ = ivt.id_; size_ = ivt.size_; offset_ = ivt.offset_;
	}
	Ivt& operator= (const Ivt& ivt) {
		if (&ivt != this) {
			id_ = ivt.id_; size_ = ivt.size_; offset_ = ivt.offset_;
		}
		return *this;
	}
	void free() {
		if (offset_ != nullptr) {
			delete [] offset_;
			offset_ = nullptr;
		}
	}
	bool operator < (const Ivt<ID, V>& ivt) {
		return id_ < ivt.id_;
	}

	friend std::ostream& operator << (std::ostream & o, const Ivt & ivt) {
		o << ivt.id_ << ":" << ivt.size_ << ":" ; //  << std::string(ivt.offset_, ivt.size_);
		for (size_t i = 0; i < ivt.size_; ++i) {
			o << ivt.offset_[i] << " ";
		}
		return o;
	}
};

template <class V>
class Invert {
public:
	uint32_t   size_;
	V         *offset_; // 数组
public:
	Invert() {size_ = 0; offset_ = 0;}
	~Invert() {
		if (offset_ != nullptr) {
			delete [] offset_;
			offset_ = nullptr;
		}
	}
	bool load(const std::string &ivt) {
		if (offset_ != nullptr) {
			delete [] offset_;
			offset_ = nullptr;
			size_ = 0;
		}
		FP fp;
		do {
			if (false == fp.open(ivt, "rb")) break;
			size_ = fp.size<V>();
			if (size_ == 0) return true;
			offset_ = new (std::nothrow) V[size_];
			if (offset_ == nullptr) break;
			if (false == fp.read(offset_, size_)) break;
			return true;
		} while(0);
		return false;
	}
};


// HMap
template <class ID, class V>
class HMap {
private:
	Basket<ID, Ivt<ID, V>> b_;
	Invert<V>              v_;
	RWLock                *lock_;
public:
	HMap():lock_(nullptr) {}
	~HMap() {
		unInit();
	}
public:
	bool init(uint32_t size, std::function<ID (const Ivt<ID, V>&)> func) {
		return b_.init(size, func);
	}
	bool unInit() {
		for (uint32_t i = 0; i < b_.size_; ++i) {
			for (size_t j = 0; j < b_[i].size(); ++j) {
				Ivt<ID, V> &ivt = b_[i][j];
				if (ivt.offset_ < v_.offset_ || ivt.offset_ > v_.offset_+v_.size_) {
					ivt.free();
				}
			}
		}
		return b_.unInit();
	}
public:
	// 若set成功，ivt.offset_所指向的内存，由hash_table接管
	bool set(const Ivt<ID, V> &ivt) {
		uint32_t i = b_.get_basket(ivt.id_);
		if (lock_ != nullptr) { lock_->rdlock(i); }
		Ivt<ID, V> *ptr = (Ivt<ID, V>*)biFind<ID, Ivt<ID, V>>(b_[i], ivt.id_);
		// 新加入的
		if (ptr == nullptr) {
			b_[i].push_back(ivt);
			std::sort(b_[i].begin(), b_[i].end(),
				[](const Ivt<ID, V>& a, const Ivt<ID, V> &b) -> bool {return a.id_ < b.id_;});
			b_++;
		// 替换旧的
		} else {
			// 不在Invert中，需要先把之前的内存释放掉
			if (ptr->offset_ < v_.offset_ || ptr->offset_ > v_.offset_+v_.size_) {
				ptr->free();
			}
			ptr->size_   = ivt.size_;
			ptr->offset_ = ivt.offset_;
		}
		if (lock_ != nullptr) { lock_->unlock(i); }
		return true;
	}
	// 若get成功，会返回一块指向Ivt.offset_的内存，需要调用者自己释放
	bool get(const ID &id, Ivt<ID, V> &ivt) {
		bool ret = false;
		if (b_.data_ == nullptr) return false;
		uint32_t i = b_.get_basket(id);
		if (lock_ != nullptr) { lock_->rdlock(i); }
		Ivt<ID, V> *ptr = (Ivt<ID, V>*)biFind<ID, Ivt<ID, V>>(b_[i], id);
		if (ptr != nullptr) {
			ret = true;
			ivt = *ptr; // 需要Ivt重载 operator =
		}
		if (lock_ != nullptr) { lock_->unlock(i); }
		return ret;
	}

	// dump
	bool dump(const std::string &sha, const std::string &ivt) {
		std::vector<Ivt<ID, V>> vec;
		if (false == dump(ivt, vec)) return false;
		Basket<ID, Ivt<ID, V>> b;
		b.init(10, b_.hash_func_);
		b.load(vec);
		bool ret = b.dump(sha);
		b.unInit();
		return ret;
	}
	// load from file
	bool load(const std::string &sha, const std::string &ivt) {
		do {
			if (false == v_.load(ivt)) break;
			if (false == b_.load(sha)) break;
			// 修正偏移
			for (uint32_t i = 0; i < b_.size_; ++i) {
				for (size_t j = 0; j < b_[i].size(); ++j) {
					Ivt<ID, V> & ivt = b_[i][j];
					ivt.offset_ += ((uint64_t)v_.offset_)/(uint64_t)sizeof(V);
				}
			}
			return true;
		} while(0);
		return false;
	}
	void print() {
		b_.print();
	}

private:
	// dump ivt
	bool dump(const std::string &ivt, std::vector<Ivt<ID, V>> &vec) {
		FP fp;
		fp.open(ivt, "wb");
		V *off = nullptr;
		for (uint32_t i = 0; i < b_.size_; ++i) {
			for (size_t j = 0; j < b_[i].size(); ++j) {
				Ivt<ID, V> &d = b_[i][j];
				if (d.offset_ != nullptr) {
					if (false == fp.write(d.offset_, d.size_)) {
						INFO("dump [%s] failed", ivt.c_str());
						return false;
					} else {
						// 修正在Basket中的偏移
						Ivt<ID, V> tmp(d.id_, d.size_, off);
						vec.push_back(tmp);
						off += d.size_;
					}
				}
			}
		}
		INFO("dump [%s] success", ivt.c_str());
		return true;
	}
};

//////////////////////////////////////// for test
void test_HMap1() {
	std::function<uint32_t(const Ivt<int32_t, char>&)> f = [](const Ivt<int32_t, char> &ivt) {
		return ivt.id_;
	};

	std::string sha("./hmap.sha");
	std::string ivt("./hmap.ivt");

	HMap<int32_t, char> h1;
	h1.init(10, f);
	for (int32_t i = 0; i < 100; ++i) {
		char *buf = new char[32];
		sprintf(buf, "[%d-data]", i);
		Ivt<int32_t, char> ivt(i, (uint32_t)strlen(buf), buf);
		h1.set(ivt);
	}
	h1.print();
	h1.dump(sha, ivt);
	h1.unInit();
	///
	HMap<int32_t, char> h2;
	h2.init(10, f);
	h2.load(sha, ivt);
	h2.print();
	///

	char *buf = new char[32];
	memcpy(buf, "sdfwefewf", strlen("sdfwefewf"));
	Ivt<int32_t, char> b(12, 10, buf);
	h2.set(b);
	h2.print();

	//
	char *buf2 = new char[32];
	memcpy(buf2, "11111111111111111111111", strlen("11111111111111111111111"));
	Ivt<int32_t, char> c(12, sizeof("11111111111111111111111"), buf2);
	h2.set(c);
	h2.print();
	h2.unInit();
}

void test_HMap2() {
	std::function<uint32_t(const Ivt<int32_t, float>&)> f = [](const Ivt<int32_t, float> &ivt) {
		return ivt.id_;
	};

	std::string sha("./hmap.sha");
	std::string ivt("./hmap.ivt");

	HMap<int32_t, float> h1;
	h1.init(10, f);
	for (int32_t i = 0; i < 100; ++i) {
		float *buf = new float[32];
		for(size_t j = 0; j < 32; ++j) {
			buf[j] = j*0.11;
		}
		Ivt<int32_t, float> ivt(i, 32, buf);
		h1.set(ivt);
	}
	h1.print();
	h1.dump(sha, ivt);
	h1.unInit();
	//
	printf("------------after-----------\n");
	HMap<int32_t, float> h2;
	h2.init(10, f);
	h2.load(sha, ivt);
	h2.print();
	h2.unInit();
}

}

#endif /*__UTILS_HASHMAP_HMAP_H__*/
