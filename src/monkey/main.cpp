#include <iostream>
#include "src/monitor_file_change/monitor_file.h"
#include "src/monitor_file_change/parse_mktdt90.h"

int main(int argc, char**argv) {
	test_monitor_file("F:/mkdt90/mktdt90-1.txt");
	//test_parse_mktdt90();
	return 0;
}