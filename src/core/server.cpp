#include <hyperion/server.h>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace hyperion {

Server::Server(int port) : port_(port) {}

void Server::start() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return;
    }

    listen(server_fd, 10);

    std::cout << "Hyperion server running on port " << port_ << std::endl;

    while (true) {

        int client_socket = accept(server_fd, nullptr, nullptr);

        if (client_socket < 0)
            continue;

        char buffer[4096] = {0};

        read(client_socket, buffer, sizeof(buffer));

        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 19\r\n"
            "\r\n"
            "Hello from Hyperion";

        write(client_socket, response, strlen(response));

        close(client_socket);
    }
}

}
