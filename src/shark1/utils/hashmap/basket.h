#ifndef __UTILS_HASHMAP_BASKET_H__
#define __UTILS_HASHMAP_BASKET_H__

#include <math.h>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "utils.h"

// ********************************************************************
// 这一套接口，是采用 hash table 来存储数据，要求存放在其中的数据结构，都有一个 id_,
// (1) 通过hash(id_)，来找到hash桶的位置
// (2) 在同一个桶里面，根据id_进行排序，查找的时候，采用二分查找法进行查找，若id_相同则数据进行覆盖
// ********************************************************************
namespace HASHMAP {

template<class K>
class Node {
public:
	K        key_;
	uint32_t hash_k_; // key_的hash值
public:
	explicit
	Node(const K & k, const uint32_t hk) : key_(k), hash_k_(hk) {
	}
	Node(const Node& n) {
		key_    = n.key_;
		hash_k_ = n.hash_k_;
	}
	Node& operator= (const Node& n) {
		if (&n != this) {
			key_    = n.key_;
			hash_k_ = n.hash_k_;
		}
		return *this;
	}
	// for sort
	bool operator< (const Node &a) const {
		if      (hash_k_ < a.hash_k_) return true;
		else if (hash_k_ > a.hash_k_) return false;
		return  key_.id_ < a.key_.id_; // caution: V 需要支持 operator <
	}
};

// K为模版类型，里面需要有个 id_ 字段
template<class ID, class K>
class Basket {
public:
	uint32_t           size_;  // hash桶长度
	uint32_t           total_; // 元素个数，= 各桶元素个数之和
	std::vector<K>    *data_;
	std::function<ID (const K&)> hash_func_;
public:
	Basket() {
		size_  = 0;
		total_ = 0;
		data_      = nullptr;
		hash_func_ = nullptr;
	}
	~Basket() {
		unInit();
	}
	Basket(const Basket&)            = delete;
	Basket& operator=(const Basket&) = delete;
public:
	bool init(const uint32_t size, std::function<ID (const K&)> hf) {
		size_      = size;
		hash_func_ = hf;
		total_     = 0;
		if (data_ != nullptr) {
			delete [] data_;
		}
		data_ = new (std::nothrow) std::vector<K> [size_];
		assert(data_ != nullptr);
		return true;
	}
	bool unInit() {
		if (data_ != nullptr) {
			for (uint32_t i = 0; i < size_; ++i) {
				data_[i].clear();
			}
			delete [] data_;
			data_ = nullptr;
		}
		size_  = 0;
		total_ = 0;
		return true;
	}
	// 返回在hash table中的位置
	uint32_t get_basket(const ID &id) const {
		return id % size_;
	}
	uint32_t get_basket(const K &t) const {
		uint32_t d = (uint32_t)hash_func_(t);
		return d % size_;
	}
	// 返回hash桶
	std::vector<K> & operator[] (uint32_t i) {
		assert(i < size_);
		return data_[i];
	}
	const
	std::vector<K> & operator[] (uint32_t i) const {
		assert(i < size_);
		return data_[i];
	}
	// 统计共有多少元素
	Basket& operator++(int) {
		__sync_fetch_and_add(&total_, 1);
		return *this;
	}
private:
	uint32_t get_size(uint32_t /*n*/) {
//		int i = (int)(floor(log2(abs(n))) + 1);
//		int min = i < 30 ? i :  30;
//		return 1 << min;
		return 10;
	}
public:
	bool load(const std::vector<K> & vec) {
		unInit(); // 释放之前分配的内存
		total_ = vec.size();
		size_  = get_size(total_);
		data_  = new (std::nothrow) std::vector<K> [size_];
		assert(data_ != nullptr);
		//
		std::vector<uint32_t> vec_count(size_, 0); // 统计每个vector的大小
		std::vector<Node<K>> vec_node;
		vec_node.reserve(total_); // 预留空间
		for (size_t i = 0; i < vec.size(); ++i) {
			uint32_t hv = (uint32_t)hash_func_(vec[i]) % size_;
			vec_count[hv]++;
			vec_node.push_back(Node<K>(vec[i], hv));
		}
		// sort: 先按桶的顺序进行排序，同一桶中的元素，按照id_排序
		std::sort(vec_node.begin(), vec_node.end());
		// save
		uint32_t c = 0;
		for (uint32_t i = 0; i < size_; ++i) {
			data_[i].reserve(vec_count[i]);
			for (uint32_t j = 0; j < vec_count[i]; ++j) {
				data_[i].push_back(vec_node[c++].key_);
			}
		}
		return true;
	}
public:
	// 数据存放顺序：size_, 各桶中元素个数(uint32_t)， total_, 各元素
	bool load(const std::string &sha) {
		unInit(); // 释放之前分配的内存
		FP fp;
		if (false == fp.open(sha, "rb")) {
			return false;
		}
		// size_
		if (false == fp.read(size_)) {
			return false;
		}
		data_  = new (std::nothrow) std::vector<K>[size_];
		if (data_ == nullptr) {
			ERROR("new std::vector<TYPE>[size_] failed.");
			return false;
		}
		// 各桶中元素个数
		std::vector<uint32_t> idx;
		if (false == fp.read(idx, size_)) {
			return false;
		}
		// total_
		if (false == fp.read(total_)) {
			return false;
		}
		// 各桶中元素
		for (uint32_t i = 0; i < size_; ++i) {
			if (false == fp.read(data_[i], idx[i])) {
				return false;
			}
		}
		// close
		fp.close();
		return true;
	}
	bool dump(const std::string &sha) {
		FP fp;
		if (false == fp.open(sha, "wb")) {
			INFO("dump [%s] failed", sha.c_str());
			return false;
		}
		// size_
		if (false == fp.write(size_)) {
			INFO("dump [%s] failed", sha.c_str());
			return false;
		}
		// 各桶中元素个数
		std::vector<uint32_t> idx;
		idx.resize(size_);
		for (uint32_t i = 0; i < size_; ++i) {
			idx[i] = data_[i].size();
		}
		if (false == fp.write(idx)) {
			INFO("dump [%s] failed", sha.c_str());
			return false;
		}
		// 总元素个数
		fp.write(total_);
		// 各桶中元素
		for (uint32_t i = 0; i < size_; ++i) {
			if(false == fp.write(data_[i])) {
				INFO("dump [%s] failed", sha.c_str());
				return false;
			}
		}
		// close
		fp.close();
		INFO("dump [%s] success", sha.c_str());
		return true;
	}

public:
	void print() const {
		printf("bucket size[%d] total[%d] \n", size_, total_);
		for (uint32_t i = 0; i < size_; ++i) {
			printf("== vector[%d] size[%lu]: \n", i, data_[i].size());
			const std::vector<K> &vec = data_[i];
			for (auto & v : vec) {
				std::cout << v << std::endl;
			}
			printf("\n");
		}
	}
public:
	// 返回桶大小
	inline uint32_t get_size(uint32_t s) const {
		int i = (floor(log2(s)) + 1);
		i = (i < 30) ? i : 30;
		return 1 << i;
	}
};

/////////////////////////////////////////////// test
#include <ostream>
struct TestNode {
	int id_;
	int a2;
	int a3;
public:
	TestNode(int _1 = 0, int _2 = 0, int _3 = 0) {
		id_ = _1, a2 = _2, a3 = _3;
	}
	TestNode(const TestNode& t) {
		id_ = t.id_, a2 = t.a2, a3 = t.a3;
	}
	TestNode& operator=(const TestNode& t) {
		if (&t != this) {
			id_ = t.id_, a2 = t.a2, a3 = t.a3;
		}
		return *this;
	}
	friend std::ostream & operator << (std::ostream& o, const TestNode & t) {
		o << t.id_ << ":" << t.a2 << ":" << t.a3;
		return o;
	}
};

void baset_test() {
	std::vector<TestNode> v;
	v.reserve(1000);
	for (uint32_t i = 0; i < 100; ++i) {
		TestNode t(i, i+10, i-10);
		v.push_back(t);
	}

	std::function<uint32_t(const TestNode&)> f = [](const TestNode &i) -> uint32_t {
		return i.id_ * 3;
	};
	Basket<int, TestNode> b;
	b.init(10, f);
	b.load(v);
	b.print();
	b.unInit();
}

}

#endif /* __UTILS_HASHMAP_BASKET_H__ */
