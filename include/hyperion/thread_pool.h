#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

class ThreadPool
{
public:
    ThreadPool(size_t threads);
    void enqueue(int client_socket);

private:
    std::vector<std::thread> workers;
    std::queue<int> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;

    void worker_loop();
};
