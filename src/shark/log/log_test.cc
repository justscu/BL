#include <thread>
#include "log.h"

static
void thread_cb(int id) {
	for (int i = 0; i < 100; ++i) {
		DEBUG("identify[%d] [%d]  000--- OK, debug, info", id, i);
	}
	sleep(10);
}

int log_test() {
	LOG::FileLogger *pDebug = new LOG::FileLogger("log_test.debug");
	//LOG::Logger *pDebug = new LOG::OStreamLogger(std::cout);
	SET_LOGGER(LOG::LOGLEVEL::kDebug, pDebug);

	std::thread* th[200];
	for (size_t i = 0; i < sizeof(th)/sizeof(th[0]); ++i) {
		th[i] = new std::thread(std::bind(thread_cb, i));
	}

	for (size_t i = 0; i < sizeof(th)/sizeof(th[0]); ++i) {
		th[i]->join();
	}

	delete pDebug;
	return 0;	
}
