#include <thread>
#include <unistd.h>
#include "channel_data.h"

// 5 threads write.

#define W100 1000000 // W100
struct DATA {
    uint16_t channel;
    struct {
    int64_t seq;
    } trade;
    char    str[32]; // itoa(seq)
};


// write thread
void th_write(uint16_t chid, DataArrMgr<DATA>* dt_mgr) {
    fprintf(stdout, "thread: channel %u begin. \n", chid);

    ChnnDataMgr<DATA> ch_mgr;
    ch_mgr.init(dt_mgr);

    DATA d;
    for (int64_t i = 0; i < W100*2; ++i) {
        d.channel = chid;
        d.trade.seq = i;
        snprintf(d.str, 32, "%u.%ld", chid, i);

        if (!ch_mgr.add(chid, i, &d)) {
            fprintf(stdout, "add %u.%ld failed. \n", chid, i);
        }

        snprintf(d.str, 32, "%u.%ld", chid+1, i);
        if (!ch_mgr.add(chid+1, i, &d)) {
            fprintf(stdout, "add %u.%ld failed. \n", chid+1, i);
        }

        snprintf(d.str, 32, "%u.%ld", chid+2, i);
        if (!ch_mgr.add(chid+2, i, &d)) {
            fprintf(stdout, "add %u.%ld failed. \n", chid+2, i);
        }
    }

    fprintf(stdout, "thread: channel %u exit. \n", chid);
}

void th_read(DataArrMgr<DATA>* mgr) {
    for (int32_t i = 0; i < 3; ++i) {
        usleep(120);
        mgr->dump();
    }
}


void test_channel_data() {
    DataArrMgr<DATA> data_mgr;
    if (!data_mgr.init(W100*30)) { return ; }

    std::thread* th1 = new std::thread(std::bind(th_write, 2010, &data_mgr));
    std::thread* th2 = new std::thread(std::bind(th_write, 2020, &data_mgr));
    std::thread* th3 = new std::thread(std::bind(th_write, 2030, &data_mgr));
    std::thread* th4 = new std::thread(std::bind(th_write, 2040, &data_mgr));

    std::thread* th5 = new std::thread(std::bind(th_read, &data_mgr));

    th1->join();
    th2->join();
    th3->join();
    th4->join();

    getchar();

    data_mgr.dump();
}
