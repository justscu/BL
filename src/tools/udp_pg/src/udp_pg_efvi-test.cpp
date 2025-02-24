#include <arpa/inet.h>
#include <functional>
#include "fmt/format.h"
#include "fmt/color.h"
#include "udp_pg_efvi.h"
#include "commx/utils.h"


// 组播组
const char *g_dst_ip[16] = {
        "230.1.2.128", "230.1.2.129", "230.1.2.130", "230.1.2.131",
        "230.1.2.132", "230.1.2.133", "230.1.2.134", "230.1.2.135",
        "230.1.2.136", "230.1.2.137", "230.1.2.138", "230.1.2.139",
        "230.1.2.140", "230.1.2.141", "230.1.2.142", "230.1.2.143",
};

const uint16_t g_dst_port[16] = {
        12128, 12129, 12130, 12131,
        12132, 12133, 12134, 12135,
        12136, 12137, 12138, 12139,
        12140, 12141, 12142, 12143
};

// 发送多少个包，sleep 1次
#define SEND_MAX_CNT 1000

enum kSendMode {
    kSendMode_dma   = 1, // dma
    kSendMode_ctpio = 2  // ctpio
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 组播发送线程
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void efvi_multicast_send_thread(uint8_t cpu_id,
                                const char *mode, // DMA or CTPIO
                                const char *nic, // 发送组播的网卡
                                const uint32_t udp_payload_len // udp载荷的长度
                               ) {
    bind_thread_to_cpu(cpu_id);
    fmt::print("bind_thread_to_cpu({}). \n", cpu_id);

    fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic,
            "efvi_multicast_send_thread: {} {} {} \n", mode, nic, udp_payload_len);

    const kSendMode send_mode = (0 == strncasecmp(mode, "dma", 3)) ? kSendMode_dma : kSendMode_ctpio;

    uint8_t src_mac[8];
    char    src_ip[16];
    {
        Nic nic_funcs;
        if (!nic_funcs.get_mac(nic, src_mac) || !nic_funcs.get_ip(nic, src_ip)) {
            fmt::print("{}. \n", nic_funcs.err());
            return;
        }
    }

    EfviUdpSend tx;
    if (!tx.init(nic)) {
        fmt::print("{} \n", tx.err());
        return;
    }

    fmt::print("send_mode    : {} \n", send_mode == kSendMode_dma ? "DMA" : "CTPIO");
    fmt::print("efvi_version : {} \n", tx.efvi_version());
    fmt::print("efvi_driver  : {} \n", tx.efvi_driver_interface());
    fmt::print("efvi_nic_arch: {} \n", tx.efvi_nic_arch());
    fmt::print("support_ctpio: {} \n", tx.is_efvi_support_ctpio(nic));
    fmt::print("X3 card      : {} \n\n", tx.is_x3());

    sleep(5);

    MakeMCastPkt mcast;
    PKT_BUF *cell = nullptr;
    for (uint64_t i = 1; true;) {
        cell = tx.get_usable_dma_addr();
        if (!cell) {
            tx.poll_tx_completions();
            fmt::print(fg(fmt::rgb(250, 200, 0)) | fmt::emphasis::bold, "[{}] get_usable_dma_addr() failed. \n", cpu_id);

            sleep(1);
            continue;
        }

        //
        char *mac_addr = cell->user_buf;
        mcast.init_hdr_partial(mac_addr, (const char*)src_mac, src_ip, g_dst_ip[i % 16]);
        mcast.set_udp_hdr(mac_addr, g_dst_port[i % 16], g_dst_port[i % 16]);

        // udp payload
        char *d = mac_addr + 14 + 20 + 8;
        *((uint64_t*)(d)) = i;
        struct timespec *ts = (timespec*)(d + 8);
        clock_gettime(CLOCK_REALTIME, ts);

        memcpy(d + 64, g_dst_ip + 1, 800); // 拷贝随机数据

        const int32_t vlen = mcast.finish_hdr(mac_addr, i, udp_payload_len);

        if (send_mode == kSendMode_dma) {
            if (!tx.dma_send(cell, vlen)) {
                fmt::print(fg(fmt::rgb(250, 200, 0)) | fmt::emphasis::bold, "[{}] {} \n", cpu_id, tx.err());
            }
            tx.poll_tx_completions();
        }
        else if (send_mode == kSendMode_ctpio) {
            if (!tx.ctpio_send(cell, vlen)) {
                fmt::print(fg(fmt::rgb(250, 200, 0)) | fmt::emphasis::bold, "[{}] {} \n", cpu_id, tx.err());
            }
            tx.poll_tx_completions();
        }

        if (i % SEND_MAX_CNT == 0) {
            char tm[32] = {0};
            UtilsTimefmt::get_now2(tm);
            fmt::print("{}: {} ef_vi {}: [{}] send len[{}] \n", tm, i, mode, cpu_id, vlen);
            UtilsCycles::sleep(1000*500); // 500ms
        }
        ++i;
    } // for
}


// str: 包含mac头的数据
// len: 数据长度
static void recv_cb_func(const char *str, int32_t len) {
//    const udp_hdr *udp_hd = (udp_hdr*)(str + sizeof(mac_hdr) + sizeof(ip_hdr));
//    if (udp_hd->dst_port != htons(dst_port)) {
//        fmt::print(fg(fmt::rgb(250, 0, 136)) | fmt::emphasis::italic, "port {}, unwant data.. \n", ntohs(udp_hd->dst_port));
//        return;
//    }

    static Sta sta;
    static std::vector<int64_t> delay_vec;

    static int32_t recv_cnt = 0;
    ++recv_cnt;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    MakeUdpPkt udp;
    const uint64_t  idx = *(uint64_t*)(str+udp.udp_payload_offset());
    const timespec *pre =  (timespec*)(str+udp.udp_payload_offset()+8);

    int64_t cost = (now.tv_sec - pre->tv_sec) * 1000000000 + (now.tv_nsec - pre->tv_nsec);

    delay_vec.push_back(cost);

    if (idx % SEND_MAX_CNT == 0) {
        const Sta::Rst &rst = sta(delay_vec);
        fmt::print(fg(fmt::rgb(10, 255, 10)) | fmt::emphasis::italic,
                "                 idx[{}] recv_cnt[{}], 1/2RTT time(ns): {} {} {} {}, mid[{}]. \n",
                idx, recv_cnt,
                rst.min, rst.max, rst.avg, rst.stddev, rst.m50);

        delay_vec.clear();
        delay_vec.reserve(128);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 组播接收线程
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void efvi_multicast_recv_thread(uint8_t cpu_id,
                                const char *nic,  // 接收组播的网卡
                                const char *dst_ip,     // 组播目的ip
                                const uint16_t dst_port // 组播目的port
                               ) {
    bind_thread_to_cpu(cpu_id);
    fmt::print("bind_thread_to_cpu({}). \n", cpu_id);

    EfviUdpRecv rx;
    if (!rx.init(nic) || !rx.add_filter(dst_ip, dst_port)) {
        fmt::print("{} \n", rx.err());
        return;
    }

    rx.recv(recv_cb_func);
}
