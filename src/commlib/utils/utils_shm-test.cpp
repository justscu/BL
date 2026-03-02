#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "utils_shm.h"



struct MyTick {
    uint32_t seq;
    int64_t  timestamp_ns;
    double   price;
};

// 获取当前纳秒时间戳
inline int64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


// 生产者逻辑
void run_producer(const char* name) {
    ShmPrd<MyTick> prd;
    if (!prd.init(name)) {
        std::cerr << "Producer Init Failed: " << prd.err() << std::endl;
        return;
    }
    std::cout << "--- Producer Started ---" << std::endl;

    uint32_t count = 0;
    while (true) {
        MyTick tick;
        tick.seq = ++count;
        tick.price = 3000.50 + (count % 100);
        tick.timestamp_ns = now_ns();

        prd.shm_write(tick);

        if (count % 100000 == 0) {
            std::cout << "[Producer] Sent 100,000 ticks. Last Seq: " << count << std::endl;
        }
        // 控制频率：也可以去掉 usleep 进行压测
        usleep(1);
    }
}


uint32_t cur_idx = 0;
uint64_t total_latency = 0;
uint64_t recv_count = 0;

void process_data(const MyTick &d) {
    fprintf(stdout, "[Consumer] data %u. \n", d.seq);

    if (cur_idx == 0) {
        cur_idx = d.seq;
    }

    if (cur_idx != d.seq) {
        fprintf(stdout, "[Consumer] Gap! Last: %u, Cur: %u. \n", cur_idx, d.seq);
        cur_idx = d.seq;
    }

    total_latency += now_ns() - d.timestamp_ns;
    recv_count++;
    cur_idx++;

    if (recv_count % 100000 == 0) {
        fprintf(stdout, "[Consumer] Recv: %u, Avg Latency: %u ns \n", recv_count, total_latency / recv_count);
    }
}

// 消费者逻辑
void run_consumer(const char* name) {
    ShmCon<MyTick> con;
    if (!con.init(name)) {
        std::cerr << "Consumer Init Failed: " << con.err() << std::endl;
        return;
    }
    std::cout << "--- Consumer Started ---" << std::endl;

    MyTick data;
    // 关键：模仿你 read_data 的逻辑
    // 建议在 ShmCon 里临时把 shm_region_ 设为 public 方便测试获取初始 idx
    // 或者直接从 0 开始触发你的“追尾”逻辑
    uint32_t cur_idx = 0;
    uint32_t last_seq = 0;
    uint64_t total_latency = 0;
    uint64_t recv_count = 0;

    con.shm_read_thread(process_data);
}


void Utils_test_shm(int32_t argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [p|c]" << std::endl;
        return;
    }

    const char* shm_name = "/market_test_shm";
    if (argv[1][0] == 'p') {
        run_producer(shm_name);
    }
    else {
        run_consumer(shm_name);
    }
    return;
}
