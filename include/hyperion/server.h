#pragma once

namespace hyperion
{

    class Server
    {
    public:
        explicit Server(int port);
        void start();

    private:
        int port_;
    };
}