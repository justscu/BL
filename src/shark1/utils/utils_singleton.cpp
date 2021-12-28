#include "utils_singleton.h"

std::mutex       UtilsSingleton1::mutex_;
UtilsSingleton1* UtilsSingleton1::instance_ = nullptr;
