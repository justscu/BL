#include "log/log.h"
#include "http/server.h"
#include "http/client.h"
#include <thread>

int main() {
//	HTTP::Server s(8, "test_http_server", "1.2.0");
//	s.init("127.0.0.1", 12345);
//	s.start();

	HTTP::Client client;
	client.init("127.0.0.1", 8080);
	client.request("/_status");

	std::thread *t = new std::thread(std::bind(HTTP::Client::loop, &client));
	sleep(500);
	client.loop_exit();
	client.unInit();
	t->detach();
	delete t;
	return 0;
}
