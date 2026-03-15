#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "hyperion/connection.h"

#define MAX_EVENTS 1024
#define POOL_SIZE 10000

static void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void start_kqueue_server(int port)
{

    // ── 1. Create server socket ────────────────────────────────────
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // ── THIS IS THE NEW LINE ───────────────────────────────────────
    // Allows multiple threads to bind to same port
    // OS distributes connections across all threads
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd, (sockaddr *)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    set_non_blocking(server_fd);

    // ── 2. Each thread gets its own kqueue ─────────────────────────
    int kq = kqueue();

    struct kevent change;
    EV_SET(&change, server_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
    kevent(kq, &change, 1, NULL, 0, NULL);

    // ── 3. Each thread gets its own connection pool ────────────────
    std::vector<Connection *> pool;
    pool.reserve(POOL_SIZE);
    for (int i = 0; i < POOL_SIZE; i++)
    {
        pool.push_back(new Connection(-1));
    }
    int pool_idx = 0;

    std::unordered_map<int, Connection *> connections;
    connections.reserve(POOL_SIZE);

    struct kevent events[MAX_EVENTS];

    // ── 4. Event loop — same as before ─────────────────────────────
    while (true)
    {

        int n = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);

        for (int i = 0; i < n; i++)
        {
            int fd = (int)events[i].ident;

            if (fd == server_fd)
            {
                while (true)
                {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd < 0)
                        break;

                    set_non_blocking(client_fd);

                    Connection *conn;
                    if (pool_idx < POOL_SIZE)
                    {
                        conn = pool[pool_idx++];
                        conn->reset(client_fd);
                    }
                    else
                    {
                        conn = new Connection(client_fd);
                    }

                    connections[client_fd] = conn;

                    struct kevent ce;
                    EV_SET(&ce, client_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
                    kevent(kq, &ce, 1, NULL, 0, NULL);
                }
                continue;
            }

            auto it = connections.find(fd);
            if (it == connections.end())
                continue;
            Connection *conn = it->second;

            if (events[i].flags & EV_EOF)
            {
                conn->close_connection();
                connections.erase(it);
                if (pool_idx > 0)
                    pool[--pool_idx] = conn;
                else
                    delete conn;
                continue;
            }

            if (conn->state == ConnectionState::READING_REQUEST)
            {
                conn->handle_read();
            }

            if (conn->state == ConnectionState::PROCESSING)
            {
                conn->process_request();
            }

            if (conn->state == ConnectionState::WRITING_RESPONSE)
            {
                conn->handle_write();

                if (conn->needs_write_watch)
                {
                    conn->needs_write_watch = false;
                    struct kevent we;
                    EV_SET(&we, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    kevent(kq, &we, 1, NULL, 0, NULL);
                }
            }

            if (conn->state == ConnectionState::CLOSED)
            {
                conn->close_connection();
                connections.erase(it);
                if (pool_idx > 0)
                    pool[--pool_idx] = conn;
                else
                    delete conn;
            }
        }
    }
}