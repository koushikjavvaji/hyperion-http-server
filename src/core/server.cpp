#include <hyperion/server.h>
#include <iostream>
#include <thread>
#include <vector>

extern void start_kqueue_server(int port);

namespace hyperion
{

    Server::Server(int port) : port_(port) {}

    void Server::start(int num_threads)
    {
        std::cout << "Hyperion starting on port "
                  << port_
                  << " with "
                  << num_threads
                  << " threads"
                  << std::endl;

        // spawn num_threads - 1 background threads
        // main thread will run the last one
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads - 1; i++)
        {
            threads.emplace_back([this]()
                                 { start_kqueue_server(port_); });
        }

        // main thread runs its own event loop
        // blocks here forever
        start_kqueue_server(port_);

        // if somehow we get here, wait for all threads
        for (auto &t : threads)
        {
            t.join();
        }
    }
}