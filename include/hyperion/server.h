#pragma once

namespace hyperion
{
    class Server
    {
    public:
        explicit Server(int port);
        void start(int num_threads = 1);

    private:
        int port_;
    };
}