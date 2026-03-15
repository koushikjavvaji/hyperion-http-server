#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096

static void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];

    int bytes = read(client_fd, buffer, sizeof(buffer));

    if (bytes <= 0)
    {
        close(client_fd);
        return;
    }

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 19\r\n"
        "\r\n"
        "Hello from Hyperion";

    write(client_fd, response, strlen(response));

    close(client_fd);
}

void start_kqueue_server(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (sockaddr *)&address, sizeof(address));
    listen(server_fd, SOMAXCONN);

    set_non_blocking(server_fd);

    int kq = kqueue();

    struct kevent change;
    EV_SET(&change, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &change, 1, NULL, 0, NULL);

    struct kevent events[MAX_EVENTS];

    std::cout << "Hyperion kqueue server running on port " << port << std::endl;

    while (true)
    {
        int n = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].ident;

            if (fd == server_fd)
            {
                while (true)
                {
                    int client_fd = accept(server_fd, NULL, NULL);

                    if (client_fd < 0)
                        break;

                    set_non_blocking(client_fd);

                    struct kevent client_event;
                    EV_SET(&client_event, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

                    kevent(kq, &client_event, 1, NULL, 0, NULL);
                }
            }
            else
            {
                handle_client(fd);
            }
        }
    }
}
