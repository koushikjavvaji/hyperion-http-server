#include <hyperion/server.h>
#include <thread>
#include <vector>
#include <iostream>

int main()
{
    // int num_cores = std::thread::hardware_concurrency();
    // on your MacBook Air M2 = 8

    int num_cores = 4; // for testing on 4-core machines
    std::cout << "Starting Hyperion on "
              << num_cores
              << " cores"
              << std::endl;

    hyperion::Server server(8080);
    server.start(num_cores);

    return 0;
}