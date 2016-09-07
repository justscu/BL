#include "machine_info.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

void machine_info_test() {
    char buf[256] = {0};
    int32_t buf_size = sizeof(buf);
    if (get_mac(buf, buf_size)) {
        fprintf(stdout, "MAC: [%s]\n", buf);
    } else {
        fprintf(stderr, "ERROR: get_mac failed. \n");
    }

    memset(buf, 0x00, sizeof(buf));
    const char* sip = "192.168.1.128";
    if (get_mac_by_ip(sip, buf, buf_size)) {
        fprintf(stdout, "MAC: [%s]\n", buf);
    } else {
        fprintf(stderr, "get_mac_by_ip failed. sip[%s]\n", sip);
    }

    memset(buf, 0x00, sizeof(buf));
    if (get_hostname(buf, buf_size)) {
        fprintf(stdout, "hostName: [%s]\n", buf);
    } else {
        fprintf(stderr, "get_hostname failed.\n");
    }

    memset(buf, 0x00, sizeof(buf));
    if (get_disk_sn(buf, buf_size)) {
        fprintf(stdout, "disk sn: [%s]\n", buf);
    } else {
        fprintf(stderr, "get_disk_sn failed.\n");
    }


#ifdef __linux__
    // 当取不到硬盘序列号时，使用uuid
    char uuid[256];
    if (get_sda_uuid(uuid, sizeof(uuid))) {
        fprintf(stdout, "uuid: [%s]\n", uuid);
    } else {
        fprintf(stderr, "get_sda_uuid failed.\n");
    }
#endif
}

