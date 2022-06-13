#include "utils_times.h"
#include <assert.h>

struct Tv {
    const char *t1;
    const char *t2;
};

void TimeCalc_test() {
    Tv v[5];
    {
        v[0].t1 = "20220607123456.123567"; v[0].t2 = "20220607095623.987625";
        v[1].t1 = "20220607012345.987654"; v[1].t2 = "20220607084567.123456";
        v[2].t1 = "20220607090559.234567"; v[2].t2 = "20220607094518.567890";
        v[3].t1 = "20220607112345.345678"; v[3].t2 = "20220607065908.268470";
        v[4].t1 = "20220607131457.456789"; v[4].t2 = "20220607145623.135987";
    }

    for (uint32_t i = 0; i < sizeof(v)/sizeof(v[0]); ++i) {
        int64_t v1 = TimeCalc::diff(v[i].t1, v[i].t2);
        int64_t v2 = TimeCalc::today_us(v[i].t1) - TimeCalc::today_us(v[i].t2);
        assert(v1 == v2);
    }

    assert(TimeCalc::hms(v[0].t1) == 123456);
    assert(TimeCalc::hms(v[1].t1) == 12345);
    assert(TimeCalc::hms(v[2].t1) == 90559);
    assert(TimeCalc::hms(v[3].t1) == 112345);
    assert(TimeCalc::hms(v[4].t1) == 131457);
}
