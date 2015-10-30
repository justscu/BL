#include "log/log.h"
#include "http/server.h"

int main() {
	HTTP::Server s(8, "test_http_server", "1.2.0");
	s.init("127.0.0.1", 12345);
	s.start();
	return 0;
}
