#include <memory>
#include <thread>

#include "server.h"

int main()
{
    auto server_listen_thread = std::make_unique<std::thread>(setup_tcp_server);
    server_listen_thread->join();
    return 0;
}