#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/hdreg.h>
#include "machine_info.h"

// returns the host name of the system.
bool get_hostname(char* hostname, int32_t len) {
    return 0 == gethostname(hostname, len);
}

// 返回ip地址
static bool get_ip(char* ip, int32_t ip_len) {
    struct ifaddrs* ifAddr = NULL;
    if (0 != getifaddrs(&ifAddr)) {
        fprintf(stderr, "getifaddrs error[%s] \n", strerror(errno));
        return false;
    }

    while (ifAddr != NULL) {
        const void* p = &((struct sockaddr_in*) ifAddr->ifa_addr)->sin_addr;
        if (AF_INET == ifAddr->ifa_addr->sa_family) {
            inet_ntop(AF_INET, p, ip, ip_len);
            // fprintf(stdout, "IP: %s \n", ip);
            if (0 != strncmp(ip, "127.0.0.1", strlen("127.0.0.1"))) {
                return true;
            }
        } else if (AF_INET6 == ifAddr->ifa_addr->sa_family) {
            //inet_ntop(AF_INET6, p, ip, ip_len);
            //fprintf(stdout, "IP: %s \n", ip);
        }
        ifAddr = ifAddr->ifa_next;
    }

    return false;
}

// 通过ip地址，获取对应的mac地址
// @sip, 本网卡对应的IP地址，若sip为NULL，则返回第一块获取到的网卡的mac
bool get_mac_by_ip(const char* sip, char* mac, int32_t mac_len) {
    char ip[32];
    if (sip == NULL) {
        if(!get_ip(ip, sizeof(ip))) {
            fprintf(stderr, "get_ip failed. \n");
            return false;
        }
    } else {
        strcpy(ip, sip);
    }
    // 先将ip地址进行翻译
    struct sockaddr_in addr_raw;
    memset(&addr_raw, 0, sizeof(addr_raw));
    addr_raw.sin_family = AF_INET;
    if (0 == inet_aton(ip, (struct in_addr *) &addr_raw.sin_addr)) {
        fprintf(stderr, "inet_aton ip[%s] error[%s] \n", ip, strerror(errno));
        return false;
    }

    char buf[1024];
    struct ifconf ifconf;
    ifconf.ifc_len = sizeof(buf);
    ifconf.ifc_buf = buf;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket failed [%s] \n", strerror(errno));
        return false;
     }
     //获取所有接口信息
   if (-1 == ioctl(sockfd, SIOCGIFCONF, &ifconf)) {
        fprintf(stderr, "ioctl failed [%s] \n", strerror(errno));
        close(sockfd);
        return false;
    }

    int ifcount = ifconf.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < ifcount; ++i) {
        // 需要的ip
        if (((struct sockaddr_in *) &(ifconf.ifc_req[i].ifr_addr))->sin_addr.s_addr
                == addr_raw.sin_addr.s_addr) {
            struct ifreq ifqmac;
            strncpy(ifqmac.ifr_name, ifconf.ifc_req[i].ifr_name,
                    sizeof(ifconf.ifc_req[i].ifr_name));
            ioctl(sockfd, SIOCGIFHWADDR, &ifqmac);
            close(sockfd);
            const char* add = ifqmac.ifr_hwaddr.sa_data;
            snprintf(mac, mac_len, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                    add[0] & 0xFF, add[1] & 0xFF, add[2] & 0xFF, add[3] & 0xFF,
                    add[4] & 0xFF, add[5] & 0xFF);
            return true;
        }
    }

    close(sockfd);
    return false;
}

// 返回第一块获取到的网卡的mac
bool get_mac(char* mac, int32_t mac_len) {
    return get_mac_by_ip(NULL, mac, mac_len);
}


//////////////  disk sn
static bool get_disk_sn_by_name(const char*disk_name, char* sn, int32_t sn_len) {
    int fd = open(disk_name, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "open [%s] error[%s] \n", disk_name, strerror(errno));
        return false;
    }
    struct hd_driveid drive;
    int iRet = ioctl(fd, HDIO_GET_IDENTITY, &drive);
    close(fd);
    if (0 == iRet) {
        // 去空格
        int ix = 0;
        for (; ix < 20; ++ix) {
            if (drive.serial_no[ix] != ' ') {
                break;
            }
        }
        if (ix == 20) {
            return false;
        }
        memcpy(sn, drive.serial_no+ix, 20-ix);
    } else {
        fprintf(stderr, "ioctl [%s] error[%s] \n", disk_name, strerror(errno));
    }

    return true;
}

bool get_disk_sn(char* sn , int32_t sn_len) {
    char path[256] = "/dev/";
    DIR* dp = opendir(path);
    if (NULL == dp) {
        fprintf(stderr, "opendir[%s] error[%s] \n", path, strerror(errno));
        return false;
    }

    struct dirent* entry = NULL;
    while ((entry = readdir(dp)) != NULL) {
        int len = strlen(entry->d_name);
        if (len < 3) { // must /dev/hdx or /dev/sdx
            continue;
        }
        // @http://blog.csdn.net/zollty/article/details/7001950
        if (strncmp(entry->d_name, "sd", 2) == 0 || strncmp(entry->d_name, "hd", 2) == 0) {
            path[5] = 0;
            strncat(path+4, entry->d_name, len);
            if (get_disk_sn_by_name(path, sn, sn_len)) {
                closedir(dp);
                return true;
            }
        }
    }

    closedir(dp);
    return false;
}

// vm虚拟机，需要修改vmx文件，设置disk.EnableUUID = "TRUE"，启动虚拟机时，才会显示SCSI ID
bool get_sda_uuid(char* uuid, int32_t uuid_len) {
    const int32_t max_len = 128;
    char dirname[max_len] = "/dev/disk/by-id/";
    const int32_t dirlen = strlen(dirname);
    DIR* dp = opendir(dirname);
    if (NULL == dp) {
        fprintf(stderr, "opendir[%s] error[%s] \n", dirname, strerror(errno));
        return false;
    }
    if (0 != chdir(dirname)) {
        fprintf(stderr, "chdir[%s] error[%s] \n", dirname, strerror(errno));
        return false;
    }

    struct stat statbuf;
    struct dirent* entry = NULL;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
            continue;
        }
        if (strncmp("ata", entry->d_name, 3) != 0            // ata硬盘
                && strncmp("scsi", entry->d_name, 4) != 0    // scsi硬盘
                && strncmp("lvm", entry->d_name, 3) != 0) {  // lvm 虚拟硬盘
            continue;
        }
        dirname[dirlen] = 0;
        strncat(dirname + dirlen, entry->d_name, max_len - dirlen - 1);
        if (0 != lstat(dirname, &statbuf)) {
            fprintf(stderr, "lstat[%s] error[%s] \n", dirname, strerror(errno));
            continue;
        }
        // link 文件
        if (S_ISLNK(statbuf.st_mode)) {
            char linkbuf[max_len] = {0};
            if (-1 == readlink(dirname, linkbuf, max_len)) {
                fprintf(stderr, "readlink[%s] error[%s]\n", dirname, strerror(errno));
                continue;
            }
            int32_t tlen = strlen(linkbuf) - 1;
            int32_t pos = 0;
            while (tlen >= 2) {
                if (linkbuf[tlen] == '/') {
                    pos = tlen + 1;
                    break;
                }
                tlen--;
            }
            // 取sda
            if (0 == strncmp(linkbuf + pos, "sda", 3)) {
                closedir(dp);
                strncpy(uuid, entry->d_name, uuid_len);
                return true;
            }
        }
    }

    return false;
}
