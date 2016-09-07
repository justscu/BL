#include <stdio.h>
#include "staticx.h"

namespace DYTEST {

__attribute__((visibility("hidden"))) Data  data;

Data::Data() {
	data_ = 0;
	fprintf(stdout, "ctor() this[%p] data_[%d] &data[%p]\n", this, data_, &data);
}
Data::~Data() {
	fprintf(stdout, "dtor() this[%p] &data[%p]\n", this, &data);
}

void Data::print() {
	fprintf(stdout, "print1:: this[%p] data_[%d] &data[%p]\n", this, data_, &data);
	data_++;
	fprintf(stdout, "print2:: this[%p] data_[%d] &data[%p]\n", this, data_, &data);
}


__attribute__((visibility("hidden"))) Data* get_data() {
	return &data;
}

Data* get_data2() {
	return & data;
}

}
