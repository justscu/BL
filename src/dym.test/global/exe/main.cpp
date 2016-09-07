#include "../dym1/dym1.h"
#include "../dym2/dym2.h"

#include <dlfcn.h>
#include <stdio.h>

int test1() {
	dym1_f1();
	dym2_f1();
	return 0;
}

typedef void (*FUNC)();

int test2() {
//	void* dym1_h = dlopen("../dym1/debug/libdym1.dylib", RTLD_LAZY|RTLD_GLOBAL);
//	void* dym2_h = dlopen("../dym2/debug/libdym2.dylib", RTLD_LAZY|RTLD_GLOBAL);
	
	void* dym1_h = dlopen("../dym1/debug/libdym1.dylib", RTLD_LAZY|RTLD_LOCAL);
	void* dym2_h = dlopen("../dym2/debug/libdym2.dylib", RTLD_LAZY|RTLD_LOCAL);

	FUNC f1 = (FUNC)dlsym(dym1_h, "dym1_f1");
	FUNC f2 = (FUNC)dlsym(dym2_h, "dym2_f1");

	f1();
	f2();

	dlclose(dym1_h);
	dlclose(dym2_h);
	return 0;
}

int test3() {
	dym1_f2();
	dym2_f2();

	return 0;
}

int main() {
	fprintf(stdout, "------main---\n");
	test1();
	test3();

	return 0;
}
