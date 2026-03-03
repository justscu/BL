#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <thread>
#include <functional>
#include "utils_shm.h"

// 测试逻辑：5个生产者线程，生成数据，会写入 MyTick::seq
//  消费者进程，会读取该数据，并对MyTick::seq进行类加.
//  最终要求消费者的类加结果: (1+5M) * 5M * 5 / 2 = 62,500,012,500,000.

struct MyTick {
    volatile uint32_t seq;
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

    std::atomic<uint64_t> all_cnt{0};

    auto test_func = [&]() {
        uint32_t m = 0;
        while (true) {
            all_cnt += 1;

            MyTick tick;
            tick.seq = ++m;
            tick.price = 3000.50 + (m % 100);
            tick.timestamp_ns = now_ns();

            prd.shm_write(tick);

            if (m % 100000 == 0) {
                std::cout << "[Producer] Sent 100,000 ticks. Last Seq: " << m << " ," << all_cnt << std::endl;
            }
            // 控制频率：也可以去掉 usleep 进行压测
            usleep(1);

            if (m >= 5000000) {
                std::cout << "exit, " << all_cnt << std::endl;
                break;
            }
        }
    };

    // TODO 写线程个数: 5
    for (uint32_t i = 0; i < 5; ++i) {
        std::thread th(std::bind(test_func));
        th.detach();
    }

    while(1) { sleep(1); }
}


uint32_t cur_idx = 0;
uint64_t total_latency = 0;
uint64_t recv_count = 0;
uint64_t sum_all = 0;

void process_data(const MyTick &d) {
    // fprintf(stdout, "[Consumer] data %u. \n", d.seq);

    sum_all += d.seq;

    if (cur_idx == 0) {
        cur_idx = d.seq;
    }

    if (cur_idx != d.seq) {
        // TODO 在单线程写时，可以把这个地方打开，看时不时顺序写入的
        // fprintf(stdout, "[Consumer] Gap! Last: %u, Cur: %u. \n", cur_idx, d.seq);
        cur_idx = d.seq;
    }

    total_latency += now_ns() - d.timestamp_ns;
    recv_count++;
    cur_idx++;

    if (recv_count % 100000 == 0) {
        fprintf(stdout, "[Consumer] Recv: %u, Avg Latency: %u ns, %ld. \n", recv_count, total_latency / 100000, sum_all);
        total_latency = 0;
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
