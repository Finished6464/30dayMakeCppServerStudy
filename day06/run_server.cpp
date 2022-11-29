#include <stdio.h>
#include "server.h"

int main()
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    server->Startup("127.0.0.1", 8888);
    loop->Loop();
    printf("server exit.\n");
    return 0;
}