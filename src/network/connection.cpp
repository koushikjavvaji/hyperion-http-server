#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

void handle_connection(int client_socket)
{
    char buffer[4096] = {0};

    read(client_socket, buffer, sizeof(buffer));

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 19\r\n"
        "\r\n"
        "Hello from Hyperion";

    write(client_socket, response, strlen(response));

    close(client_socket);
}
