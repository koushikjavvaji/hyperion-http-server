#include <hyperion/server.h>

int main()
{
    hyperion::Server server(8080);
    server.start();
    return 0;
}