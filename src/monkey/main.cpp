#include <iostream>
#include <stdio.h>
#include "src/timer/timer.h"
// #include "src/monitor_file_change/monitor_file.h"
// #include "src/monitor_file_change/parse_mktdt90.h"
// #include "src/hardware_info/machine_info.h"


extern void multicast_usage();
extern void multicast_server(int argc, char** argv);
extern void multicast_client(int argc, char** argv);
void multicast(int argc, char** argv) {
    if (argc < 6) {
        return multicast_usage();
    }
    if (argv[1][0] == 's' || argv[1][0] == 'S') {
        return multicast_server(argc, argv);
    }

    if (argv[1][0] == 'c' || argv[1][0] == 'C') {
        return multicast_client(argc, argv);
    }
}


extern void test_channel_data();
extern void tst_sync_fetch_and_add();


int main(int argc, char**argv) {
	//test_monitor_file("F:/mkdt90/mktdt90-1.txt");
	//test_parse_mktdt90();
	// machine_info_test();
    // multicast(argc, argv);

    // test_channel_data();
    // tst_sync_fetch_and_add();
    Timer timer;
    if(!timer.init()) return 0;
    std::cout << timer.cycles_per_second() << std::endl;

    uint64_t a1 = timer.rdtsc();
    timer.sleep_microseconds(100);
    uint64_t a2 = timer.rdtsc();

    std::cout << timer.cycles_to_seconds(a2-a1) << " s" << std::endl;

	getchar();
	return 0;
}
