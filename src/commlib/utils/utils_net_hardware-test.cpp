#include "utils_net_hardware.h"

void nic_hardware_test() {
    std::vector<std::string> eths;

    Nic nic;
    if (!nic.get_all_usable_net_cards(eths)) {
        fprintf(stderr, "%s. \n", nic.err());
        return;
    }

    for (auto &it : eths) {
        char ip[32];
        char mask[32];
        uint8_t mac[32];
        int32_t mtu;
        int32_t idx;

        nic.get_ip(it.c_str(), ip);
        nic.get_mask(it.c_str(), mask);
        nic.get_mac(it.c_str(), mac);
        nic.get_mtu(it.c_str(), mtu);
        nic.get_index(it.c_str(), idx);

        char add[32] = {0};
        sprintf(add, "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0] & 0xFF, mac[1] & 0xFF,
                mac[2] & 0xFF, mac[3] & 0xFF,
                mac[4] & 0xFF, mac[5] & 0xFF);

        fprintf(stdout, "%s, %s, %s, %s, %d, idx[%d] \n", it.c_str(), ip, mask, add, mtu, idx);
    }
}
