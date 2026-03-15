#include <hyperion/server.h>
#include <thread>
#include <vector>
#include <iostream>

int main()
{
    // int num_cores = std::thread::hardware_concurrency();

    int num_cores = 4;
    std::cout << "Starting Hyperion on "
              << num_cores
              << " cores"
              << std::endl;

    hyperion::Server server(8080);
    server.start(num_cores);

    return 0;
}