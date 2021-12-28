#include <iostream>

extern "C" {
	void dym2_f1();
	void dym2_f2() { std::cout << "dym2_f2()" << std::endl; } 
}
