#include "log/log.h"
#include <thread>

//#include "utils/hashmap/basket.h"
//#include "utils/hashmap/hset.h"
//#include "utils/hashmap/hmap.h"
//#include "utils/hashmap/utils.h"

#include "utils/utils_times.h"
#include "utils/utils_pseudo_time.h"
#include "udp_multicast/udp_multicast.h"

void mulicast_test(int32_t argc, char **argv);

int main(int32_t argc, char **argv) {
//	HASHMAP::baset_test();
//	HASHMAP::test_HSet();
//	HASHMAP::test_HMap1();
//	HASHMAP::test_HMap2();
//	HASHMAP::test_HMpa3();

//    log_test();

    mulicast_test(argc, argv);

	return 0;
}
