#ifndef __PARSE_MACHINE_INFO_H__
#define __PARSE_MACHINE_INFO_H__

#include <stdint.h>

// 获取主机名
// len : 调用者分配的hn的大小
bool get_hostname(char* hn,  int32_t len);

// 获取第一块读到的网卡的mac地址
// len : 调用者分配的mac的内存大小
bool get_mac     (char* mac, int32_t len);

// 获取ip地址对应的mac地址
// 当ip=NULL时，mac为第一块获取到的网卡的mac地址
// len : 调用者分配的mac的内存大小
bool get_mac_by_ip(const char* ip, char* mac, int32_t len);

// 获取硬盘序列号（可能会需要admin权限）
// sn ： 返回第一块读到的硬盘的序列号
// len： 调用者分配的sn的内存大小
bool get_disk_sn (char* sn,  int32_t len);

#ifdef __linux__
// 因权限问题或在虚拟机中，可能获取不到disk sn，可以使用该函数来获取一个uuid，充当sn使用
bool get_sda_uuid(char* uuid, int32_t uuid_len);
#endif

// 对提供的函数进行测试
void machine_info_test();



#endif /* __PARSE_MACHINE_INFO_H__ */
