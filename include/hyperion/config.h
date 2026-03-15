#pragma once
#include <string>

namespace hyperion
{
    struct Config
    {
        int port = 8080;
        int num_threads = 1;
        std::string host = "0.0.0.0";
    };
}