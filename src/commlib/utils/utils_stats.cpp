#include <stdio.h>
#include "utils_stats.h"

const Sta::Rst& Sta::operator()(int64_t *arr, int64_t cnt) {
    if (!arr || cnt == 0) { return rst_; }

    std::stable_sort(arr, arr+cnt);
    calc1(arr, cnt); // min, max, avg
    calc_variance_stddev(arr, cnt);

    rst_.m25 = get_percentile(arr, cnt, 25);
    rst_.m50 = get_percentile(arr, cnt, 50);
    rst_.m75 = get_percentile(arr, cnt, 75);
    rst_.m90 = get_percentile(arr, cnt, 90);
    rst_.m95 = get_percentile(arr, cnt, 95);
    rst_.m99 = get_percentile(arr, cnt, 99);

    return rst_;
}

const Sta::Rst& Sta::operator()(std::vector<int64_t> &vec) {
    return operator()(&(vec[0]), vec.size());
}

void Sta::calc1(const int64_t *arr, int64_t cnt) {
    rst_.cnt = cnt;
    rst_.min = rst_.max = arr[0];

    int64_t sum0 = arr[0];
    int64_t sum1 = sum0;

    // min, max
    for (int64_t i = 1; i < cnt; ++i) {
        if (arr[i] < rst_.min) {
            rst_.min = arr[i];
        }
        else if (arr[i] > rst_.max) {
            rst_.max = arr[i];
        }

        sum0 += arr[i];
        if (sum0 >= sum1) {
            sum1 = sum0;
        }
        else {
            snprintf(err_, sizeof(err_)-1, "sum overflow.");
            return;
        }
    }

    rst_.avg = sum1 / cnt;
}

// 方差, 标准差
void Sta::calc_variance_stddev(const int64_t *arr, int64_t cnt) {
    if (cnt == 1) { return; }

    if (rst_.avg == 0) { calc1(arr, cnt); }

    double sumsq1 = 0, sumsq2 = 0;
    for (int32_t i = 0; i < cnt; ++i) {
        sumsq1 += ((arr[i] - rst_.avg) * (arr[i] - rst_.avg));
        if (sumsq1 >= sumsq2) {
            sumsq2 = sumsq1;
        }
        else {
            snprintf(err_, sizeof(err_)-1, "variance overflow.");
            return;
        }
    }

    rst_.variance = uint64_t(sumsq2 / cnt);
    rst_.stddev   = (uint64_t)sqrt(rst_.variance);
}

// percentile, 分位数 [0, 100]
int64_t Sta::get_percentile(const int64_t *arr, int64_t cnt, int64_t percentile) const {
    if (percentile < 0 || percentile > 100) {
        assert(0);
        return 0;
    }

    int64_t pos = ((cnt * percentile) / 100 + 0.5) -1;
    if (pos <= 0) { pos = 0; }
    return arr[pos];
}
