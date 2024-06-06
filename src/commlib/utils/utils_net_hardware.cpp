#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils_net_hardware.h"


bool Nic::create_socket() {
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        snprintf(err_, sizeof(err_)-1, "create socket err: %s.", strerror(errno));
        return false;
    }
    return true;
}

void Nic::close_socket() {
    if (fd_ > 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool Nic::get_all_usable_net_cards(std::vector<std::string> &eths) {
    eths.clear();

    char buf[2048];
    struct ifconf ifconf;
    ifconf.ifc_len = sizeof(buf);
    ifconf.ifc_ifcu.ifcu_buf = buf;
    if (0 != ioctl(fd(), SIOCGIFCONF, &ifconf)) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFCONF] err: %s.", strerror(errno));
        return false;
    }

    const int32_t cnt = ifconf.ifc_len / sizeof(struct ifreq);
    for (int32_t i = 0; i < cnt; ++i) {
        eths.emplace_back(ifconf.ifc_req[i].ifr_name);
    }

    return true;
}

bool Nic::get_ip(const char *eth, char *ip) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, eth);
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(fd(), SIOCGIFADDR, &ifr) < 0) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFADDR] err: %s.", strerror(errno));
        return false;
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);

    sprintf(ip, "%s", inet_ntoa(sin->sin_addr));
    return true;
}

bool Nic::get_mask(const char *eth, char *mask) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, eth);

    if (ioctl(fd(), SIOCGIFNETMASK, &ifr) < 0) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFNETMASK] err: %s.", strerror(errno));
        return false;
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_netmask);

    sprintf(mask, "%s", inet_ntoa(sin->sin_addr));
    return true;
}

bool Nic::get_mac(const char *eth, uint8_t *mac) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, eth);

    if (ioctl(fd(), SIOCGIFHWADDR, &ifr) < 0) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFHWADDR] err: %s.", strerror(errno));
        return false;
    }

    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    return true;
}

bool Nic::get_mtu(const char *eth, int32_t &mtu) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, eth);

    if (ioctl(fd(), SIOCGIFMTU, &ifr) < 0) {
        snprintf(err_, sizeof(err_)-1, "ioctl[SIOCGIFMTU] err: %s.", strerror(errno));
        return false;
    }

    mtu = ifr.ifr_mtu;
    return true;
}

bool Nic::get_index(const char *eth, int32_t &index) {
    index = if_nametoindex(eth);
    return index != 0;
}
