#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include "parse_libpcap_file.h"
#include "tinyini.h"

std::mutex mutex;

struct Cfg {
    char   filename[256];
    char      srcip[32];
    char    srcport[8];
    char      dstip[32];
    char    dstport[8];
    char     protol[8];
    char    logfile[256];
    char    loglvl[32];
};

void usage() {
    fprintf(stdout, "./pkg_reassemble cfg.ini \n");
    exit(-1);
}

void tcp_data_ready_cbfunc(const char *src, const int32_t len) {
    UtilsFileOpe ope;
    ope.open("/tmp/tcp_rb-file.txt", "ab+");
    ope.write(src, len);
}

int32_t main(int32_t argc, char **argv) {
    if (argc < 2) { usage(); }

    IniReader ini;
    if (!ini.load_ini(argv[1])) {
        fprintf(stdout, "read ini file failed. \n");
        return 0;
    }

    Cfg cfg;
    memset(&cfg, 0x00, sizeof(cfg));
    strncpy(cfg.filename, ini["capfile.filename"], sizeof(cfg.filename)-1);
    strncpy(cfg.srcip,    ini["capfile.srcip"],    sizeof(cfg.srcip)-1);
    strncpy(cfg.srcport,  ini["capfile.srcport"],  sizeof(cfg.srcport)-1);
    strncpy(cfg.dstip,    ini["capfile.dstip"],    sizeof(cfg.dstip)-1);
    strncpy(cfg.dstport,  ini["capfile.dstport"],  sizeof(cfg.dstport)-1);
    strncpy(cfg.protol,   ini["capfile.protocol"], sizeof(cfg.protol)-1);
    strncpy(cfg.logfile,  ini["capfile.logfile"],  sizeof(cfg.logfile)-1);
    strncpy(cfg.loglvl,   ini["capfile.loglevel"], sizeof(cfg.loglvl)-1);

    log_init(LOG::Format::to_lvl(cfg.loglvl), cfg.logfile);
    ini.print();

    SrSwBuffer buf;
    if (!buf.init()) {
        return 0;
    }

    std::thread *rth = new std::thread(std::bind(parse_pcap_data,
                                                 cfg.srcip, cfg.dstip, cfg.srcport, cfg.dstport,
                                                 std::ref(buf), tcp_data_ready_cbfunc));
    read_libpcap_file(cfg.filename, std::ref(buf));

    rth->join();
    delete rth;

    return 0;
}
