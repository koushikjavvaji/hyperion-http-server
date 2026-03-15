#include <hyperion/server.h>
#include <iostream>

extern void start_kqueue_server(int port);

namespace hyperion
{

    Server::Server(int port) : port_(port) {}

    void Server::start()
    {
        std::cout << "Hyperion server starting on port " << port_ << std::endl;
        start_kqueue_server(port_);
    }

}