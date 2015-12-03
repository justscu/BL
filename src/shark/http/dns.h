#ifndef __HTTP_DNS_H__
#define __HTTP_DNS_H__

#include <string>
#include "log/log.h"
#include "event2/util.h"

class DNS {
public:
    // 同步解析DNS
    static bool resolve(const std::string &host, std::string &ip);
};

#endif /*__HTTP_DNS_H__*/
