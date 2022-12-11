#include <iostream>
#include "server.h"

int main()
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop, [](Connection *conn) {  // 业务逻辑
        conn->Read();
        if (conn->GetState() != Connection::State::Closed) {
            std::cout << "Message from client " << conn->fd() << ": " << conn->GetReadStr() << std::endl;
            conn->SetSendStr(conn->GetReadStr());
            conn->Write();
        }
        else {
            return;
        }
  });
    server->Startup("127.0.0.1", 8888);
    loop->Loop();
    std::cout << "server exit." << std::endl;
    // printf("server exit.\n");
    return 0;
}