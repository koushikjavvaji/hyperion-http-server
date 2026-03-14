#include <hyperion/server.h>
#include <hyperion/thread_pool.h>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace hyperion
{

    Server::Server(int port) : port_(port) {}

    void Server::start()
    {

        int server_fd = socket(AF_INET, SOCK_STREAM, 0);

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
        {
            std::cerr << "Bind failed\n";
            return;
        }

        listen(server_fd, SOMAXCONN);

        std::cout << "Hyperion server running on port " << port_ << std::endl;

        ThreadPool pool(4); // 4 worker threads

        while (true)
        {

            int client_socket = accept(server_fd, nullptr, nullptr);

            if (client_socket < 0)
                continue;

            pool.enqueue(client_socket);
        }
    }

}
