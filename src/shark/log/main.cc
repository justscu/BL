#include <iostream>

#include "log.h"

extern void log_test();
int main() {
	INFO("fdsfew");
	DEBUG("[%s %d %f]", "fsdfd", 1232, 12.343);
	WARN("here is warning!");
	ERROR("herei s errror ");

	log_test();
}
