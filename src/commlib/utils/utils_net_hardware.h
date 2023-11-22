#pragma once

#include <stdint.h>
#include <vector>
#include <string>

// Net interface card.
class Nic {
public:
    Nic() { create_socket(); }
    ~Nic() { close_socket(); }

    Nic(const Nic&) = delete;
    Nic& operator=(const Nic&) = delete;



    bool get_all_usable_net_cards(std::vector<std::string> &eths);
    bool get_ip(const char *eth, char *ip);
    bool get_mask(const char *eth, char *mask);
    bool get_mac(const char *eth, uint8_t *mac);
    bool get_mtu(const char *eth, int32_t &mtu);
    bool get_index(const char *eth, int32_t &index);

    const char* err() { return err_; }

private:
    bool create_socket();
    void close_socket();
    int32_t fd() const { return fd_; }

private:
    char err_[256];
    int32_t fd_ = -1;
};



