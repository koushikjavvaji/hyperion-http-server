#include <hyperion/server.h>
#include <hyperion/file_cache.h>
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

        // load all files into memory ONCE at startup
        FileCache::instance().load_all();
        std::cout << "All files cached in memory" << std::endl;

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads - 1; i++)
        {
            threads.emplace_back([this]()
                                 { start_kqueue_server(port_); });
        }

        start_kqueue_server(port_);

        for (auto &t : threads)
        {
            t.join();
        }
    }
}