#include "utils_pseudo_time.h"
#include "utils_strings.h"
#include "utils_times.h"

class Utils0Test {
public:
    void UtilsPseudoTime_example() {
        UtilsPseudoTime *p = UtilsPseudoTime::get_instance(); //后台另启动一个独立线程更新时间

        while (true) {
            sleep(1);
            fprintf(stdout, "%ld \n", p->get_msec());
        }
    }

    void UtilsString_example() {

    }

    void UtilsCycles_example() {
        UtilsCycles::init();
        uint64_t c1 = UtilsCycles::current_cpu_cycles();
        //
        // ... test code
        //
        uint64_t c2 = UtilsCycles::current_cpu_cycles();

        fprintf(stdout, "%f \n", UtilsCycles::cycles_to_second(c2-c1));
    }


    void UtilsTimeFormat_example() {
        char buf[32] = {0};
        UtilsTimeFormat ut;
        ut.get_now1(buf);
        std::cout << buf << std::endl;
        ut.get_now2(buf);
        std::cout << buf << std::endl;
    }
};
