#include "utils_statistic_stddev.h"

void Stddev::reset(stddev *d) {
    d->count  = 0;
    d->min = LONG_MAX;
    d->max = LONG_MIN;
    d->sum = 0;
    d->sum_sq = 0;
}

void Stddev::add(stddev *d, int64_t value) {
    d->count += 1;
    d->sum   += value;
    d->sum_sq+= value*value;
    if (value < d->min) { d->min = value; }
    if (value > d->max) { d->max = value; }
}

void Stddev::remove(stddev *d, int64_t old) {
    d->count -= 1;
    d->sum   -= old;
    d->sum_sq-= old*old;
}

void Stddev::modify(stddev *d, int64_t oldv, int64_t newv) {
    remove(d, oldv);
    add(d, newv);
}

//  avg: 均值
// mdev: 平均偏差（偏离平均值的程度），偏差越大说明波动越大
void Stddev::get(const stddev *d, double &avg, double &mdev) {
    if (d->count > 0) {
        avg = (double(d->sum)) / (double)(d->count);
    }
    else {
        avg = 0;
    }

    if (d->count > 2) {
        double v1 = (double(d->sum)) * avg;
        double v2 = (d->sum_sq - v1) / d->count;
        mdev = sqrt(fabs(v2));
    }
    else {
        avg = 0;
    }
}
