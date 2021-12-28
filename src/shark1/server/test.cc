#include <iostream>

#include "thread.h"
#include "server.h"
#include "t2.h"

using namespace std;

void FUNC(Thread*t) {
    printf("-----id[%lu] \n", t->get_thread_id());
}

void f_thread_pool() {
    ThreadPool pool(4);
    pool.init();
    pool.start();

    for (int i = 0; i < 50; ++i) {
        pool.dispatch(std::bind(FUNC, std::placeholders::_1));
    }

    pool.stop();

    return;
}

int main() {
    ServerBase1<t2> b(12345);

    bool iB = b.init();
    if (iB == false) {
        printf("b.init false \n");
        return 0;
    }
    b.start();
}
