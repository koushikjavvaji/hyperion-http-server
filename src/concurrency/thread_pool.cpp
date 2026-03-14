#include "hyperion/thread_pool.h"
#include <unistd.h>

extern void handle_connection(int);

ThreadPool::ThreadPool(size_t threads)
{
    for (size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back(&ThreadPool::worker_loop, this);
    }
}

void ThreadPool::enqueue(int client_socket)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(client_socket);
    }

    condition.notify_one();
}

void ThreadPool::worker_loop()
{
    while (true)
    {

        int client_socket;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            condition.wait(lock, [this]
                           { return !tasks.empty(); });

            client_socket = tasks.front();
            tasks.pop();
        }

        handle_connection(client_socket);
    }
}