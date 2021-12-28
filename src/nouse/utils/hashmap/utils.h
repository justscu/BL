#ifndef __UTILS_HASHMAP_UTILS_H__
#define __UTILS_HASHMAP_UTILS_H__

#include <vector>
#include <string>
#include "log/log.h"

namespace HASHMAP {

// 文件操作类，从文件中读取反串行化数据
class FP {
private:
	FILE        *fp_;
	std::string  name_;
public:
	FP() {
		fp_ = nullptr;
	}
	~FP() {
		close();
	}
public:
	bool open(const std::string & file, const char *flag) {
		fp_ = fopen(file.c_str(), flag);
		if (fp_ == nullptr) {
			ERROR("fopen[%s] err[%s]", file.c_str(), strerror(errno));
			return false;
		}
		name_ = file;
		return true;
	}
	bool close() {
		if (fp_ != nullptr) {
			fclose(fp_);
			fp_ = nullptr;
		}
		return true;
	}
public:
	// 计算文件包含多少个TYPE数据
	template <class TYPE>
	uint32_t size() const {
		long int off = ftell(fp_); // 当前位置
		fseek(fp_, 0, SEEK_END);
		long int end = ftell(fp_);
		fseek(fp_, off, SEEK_SET); // 恢复当前位置
		long int ret = (end-off)/sizeof(TYPE);
		return (uint32_t)ret;
	}
	// 返回文件长度
	uint64_t length() const {
		long int off = ftell(fp_); // 当前位置
		fseek(fp_, 0, SEEK_END);
		long int end = ftell(fp_);
		fseek(fp_, off, SEEK_SET); // 恢复当前位置
		return (uint64_t)end;
	}
	// 从文件中读取数据到vector
	template <class TYPE>
	bool read(std::vector<TYPE> & vec, const size_t n) const {
		if (fp_ == nullptr) return false;
		if (n == 0)         return true;
		vec.resize(n);
		if (n == fread(&vec[0], sizeof(TYPE), n, fp_)) return true; // 读成功
		ERROR("fread [%s] err[%s]", name_.c_str(), strerror(errno));
		return false;
	}
	template <class TYPE>
	bool read(TYPE & d) const {
		if (fp_ == nullptr)  return false;
		if (1 == fread(&d, sizeof(TYPE), 1, fp_)) return true;
		ERROR("fread [%s] err[%s]", name_.c_str(), strerror(errno));
		return false;
	}
	template <class TYPE>
	bool read(TYPE *d, const size_t size) const {
		if (fp_ == nullptr) return false;
		if (size == 0)      return true;
		if (size == fread(d, sizeof(TYPE), size, fp_)) return true;
		ERROR("fread [%s] err[%s]", name_.c_str(), strerror(errno));
		return false;
	}
	// 写文件
	template <class TYPE>
	bool write(const std::vector<TYPE> &vec) const {
		if (fp_ == nullptr) {
			return false;
		}
		if (vec.size() == 0) {
			return true;
		}
		size_t n = fwrite(&vec[0], sizeof(TYPE), vec.size(), fp_);
		if (n != vec.size()) {
			ERROR("fwrite[%s] n[%d] vec.size[%d] err[%s]", name_.c_str(), n, vec.size(), strerror(errno));
			return false;
		}
		return true;
	}
	template <class TYPE>
	bool write(const TYPE& d) const {
		if (fp_ == nullptr) {
			return false;
		}
		size_t n = fwrite(&d, sizeof(TYPE), 1, fp_);
		if (n != 1) {
			ERROR("fwrite[%s] n[%d] err[%s]", name_.c_str(), n, strerror(errno));
			return false;
		}
		return true;
	}
	template <class TYPE>
	bool write(const TYPE *d, const size_t size) {
		if (fp_ == nullptr) return false;
		if (size == 0)      return true;
		size_t n = fwrite(d, sizeof(TYPE), size, fp_);
		if (n != size) {
			ERROR("fwrite[%s] n[%d] size[%d] err[%s]", name_.c_str(), n, size, strerror(errno));
			return false;
		}
		return true;
	}
};

// 二分查找， 要求数据在vector中有序
template<class ID, class TYPE>
const TYPE * biFind(const std::vector<TYPE> &vec, const ID &id) {
	size_t low = 0;
	size_t hig = vec.size()-1;
	if (vec.size() == 0) {
		return nullptr;
	}
	if (vec[low].id_ == id) {
		return &vec[low];
	}
	if (vec[hig].id_ == id) {
		return &vec[hig];
	}
	while (low <= hig) {
		size_t mid = (low + hig)/2;
		if (vec[mid].id_ == id) {
			return &vec[mid];
		} else if (vec[mid].id_ > id) {
			hig = mid - 1;
		} else {
			low = mid + 1;
		}
	}
	return nullptr;
}

}

//////////////////////// test
namespace HASHMAP {
namespace TEST {

#include <string>
#include <ostream>
#include <stdio.h>

struct T {
	uint32_t id_;
	std::string str;
public:
	friend std::ostream& operator << (std::ostream & o, const T & t) {
		o << t.id_ << " : " << t.str;
		return o;
	}
};

void test_biFind() {
	std::vector<T> vec;
	for (uint32_t i = 0; i < 20; ++i) {
		T t;
		t.id_ = i;
		char buf[16] = {0};
		sprintf(buf, "%d for test", i);
		t.str = buf;
		vec.push_back(t);
	}

	const T* t = biFind<uint32_t, T>(vec, 20);
	std::cout << t << std::endl;
	t = biFind<uint32_t, T>(vec, 1);
	std::cout << *t << std::endl;
	t = biFind<uint32_t, T>(vec, 10);
	std::cout << *t << std::endl;
}

}
}

#endif /* __UTILS_HASHMAP_UTILS_H__ */
