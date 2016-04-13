#include "machine_info.h"
#include <iostream>
#include <string.h>

void machine_info_test() {
    char buf[256] = {0};
    int32_t buf_size = sizeof(buf);
    get_mac(buf, buf_size);
    std::cout << "get_mac: MAC:" << buf << std::endl;

    memset(buf, 0x00, sizeof(buf));

    get_mac_by_ip("192.168.1.128", buf, buf_size); // 10.25.26.219
    std::cout << "get_mac_by_ip: MAC:" << buf << std::endl;

    memset(buf, 0x00, sizeof(buf));
    get_hostname(buf, buf_size);
    std::cout << "get_hostname: " << buf << std::endl;

    memset(buf, 0x00, sizeof(buf));
    get_disk_sn(buf, buf_size);
    std::cout << "get_disk_sn: " << buf << std::endl;


#ifdef __linux__
    char uuid[256];
    get_sda_uuid(uuid, sizeof(uuid));
    std::cout << "get_sda_uuid:" << uuid << std::endl;
#endif
}

