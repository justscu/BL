#include "dns.h"


bool DNS::resolve(const std::string &host, std::string &ip) {
	struct evutil_addrinfo *res = nullptr;
	struct evutil_addrinfo  hints_in;
	memset(&hints_in, 0, sizeof(evutil_addrinfo));
	hints_in.ai_family   = AF_UNSPEC; // AF_INET, v4; AF_INT6, v6; AF_UNSPEC, both v4 & v6
	hints_in.ai_socktype = SOCK_STREAM;
	hints_in.ai_protocol = IPPROTO_TCP;
	hints_in.ai_flags    = EVUTIL_AI_ADDRCONFIG;

	int err = evutil_getaddrinfo(host.c_str(), nullptr, &hints_in, &res);
	if (err != 0) {
		ERROR("evutil_getaddrinfo err[%d][%s] \n", err, evutil_gai_strerror(err));
		return false;
	}
	if (res == nullptr) {
		ERROR("evutil_getaddrinfo res is null");
		return false;
	}
	// 解析
	char buf[128] = {0};
	for (evutil_addrinfo *ai = res; ai != nullptr; ai = ai->ai_next) {
		if (ai->ai_family == AF_INET) {
			struct sockaddr_in *sin  = (struct sockaddr_in*)ai->ai_addr;
			evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
		} else if (ai->ai_family == AF_INET6) {
			struct sockaddr_in6 *sin = (struct sockaddr_in6*)ai->ai_addr;
			evutil_inet_ntop(AF_INET6, &sin->sin6_addr, buf, sizeof(buf));
		}
		if (ip.length() > 0) ip += ";";
		ip += buf;
	}
	evutil_freeaddrinfo(res);
	return true;
}

/////////////////////////// test DNS
void test_DNS() {
    std::string host("www.google.com");
    std::string ip;
    bool err = DNS::resolve(host, ip);
    INFO("err[%s] ip[%s]", err?"true":"false", ip.c_str());
}
