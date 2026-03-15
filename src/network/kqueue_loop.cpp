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

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd, (sockaddr *)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    set_non_blocking(server_fd);

    // ── 2. Create kqueue ───────────────────────────────────────────
    int kq = kqueue();

    struct kevent change;
    EV_SET(&change, server_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
    kevent(kq, &change, 1, NULL, 0, NULL);

    // ── 3. Pre-allocate connection pool ───────────────────────────
    // Avoids new/delete on every connection — reduces allocator pressure
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

    std::cout << "Hyperion kqueue server running on port " << port << std::endl;

    // ── 4. Event loop ──────────────────────────────────────────────
    while (true)
    {

        // Block until OS signals something is ready — O(1)
        int n = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);

        for (int i = 0; i < n; i++)
        {
            int fd = (int)events[i].ident;

            // ── New client connecting ──────────────────────────────
            if (fd == server_fd)
            {
                while (true)
                {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd < 0)
                        break;

                    set_non_blocking(client_fd);

                    // Grab from pool instead of heap allocation
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

                    // Register client fd for read events
                    struct kevent ce;
                    EV_SET(&ce, client_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
                    kevent(kq, &ce, 1, NULL, 0, NULL);
                }
                continue;
            }

            // ── Existing client has activity ───────────────────────
            auto it = connections.find(fd);
            if (it == connections.end())
                continue;
            Connection *conn = it->second;

            // Handle EV_EOF — client disconnected
            if (events[i].flags & EV_EOF)
            {
                conn->close_connection();
                connections.erase(it);
                // Return to pool
                if (pool_idx > 0)
                    pool[--pool_idx] = conn;
                else
                    delete conn;
                continue;
            }

            // ── Run state machine ──────────────────────────────────
            // Fall through each state immediately without waiting
            // for next kqueue event — this was the original bug

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

                // Kernel send buffer was full during write
                // Register EVFILT_WRITE so kqueue wakes us when ready
                if (conn->needs_write_watch)
                {
                    conn->needs_write_watch = false;
                    struct kevent we;
                    // EV_ONESHOT — fires once then removes itself
                    EV_SET(&we, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    kevent(kq, &we, 1, NULL, 0, NULL);
                }
            }

            // Clean up closed connections
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