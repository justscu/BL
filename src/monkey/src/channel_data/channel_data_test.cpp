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
void th_write(uint16_t chid, ChnnDataMgr<DATA>* mgr) {
    fprintf(stdout, "thread: channel %u begin. \n", chid);
    DATA d;
    for (int64_t i = 0; i < W100*2-chid; ++i) {
        d.channel = chid;
        d.trade.seq = i;
        snprintf(d.str, 32, "%u.%ld", chid, i);

        if (!mgr->add(chid, i, &d)) {
            fprintf(stdout, "add %u.%ld failed. \n", chid, i);
        }
    }

    fprintf(stdout, "thread: channel %u exit. \n", chid);
}

void th_read(ChnnDataMgr<DATA>* mgr) {
    for (int32_t i = 1; i < 5; ++i) {
        usleep(30);
        mgr->dump();
    }
}


void test_channel_data() {
    ChnnDataMgr<DATA> mgr;
    mgr.init(W100*30);


    std::thread* th1 = new std::thread(std::bind(th_write, 2011, &mgr));
    std::thread* th2 = new std::thread(std::bind(th_write, 2012, &mgr));
    std::thread* th3 = new std::thread(std::bind(th_write, 2013, &mgr));
    std::thread* th4 = new std::thread(std::bind(th_write, 2014, &mgr));

    std::thread* th5 = new std::thread(std::bind(th_read, &mgr));

    th1->join();
    th2->join();
    th3->join();
    th4->join();

    getchar();

    mgr.dump();
}
