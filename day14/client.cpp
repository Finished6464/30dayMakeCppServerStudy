#include <cstring>
#include <iostream>
#include "connection.h"

int main()
{
    SocketX *s = new SocketX();
    if (s->Connect("127.0.0.1", 8888) != -1) {
        Connection *conn = new Connection(s->fd());
        char buf[1024];     //定义缓冲区
        while (true) {
            memset(&buf, 0, sizeof(buf));       //清空缓冲区
            scanf("%s", buf);             //从键盘输入要传到服务器的数据
            conn->SetSendStr(buf);
            conn->Write();
            if (conn->GetState() != Connection::State::Closed) {
                conn->Read();
                std::cout << "Message from server: " << conn->GetReadStr() << std::endl;
            }
            else {
                break;
            }
        }
        delete conn;
    }

    s->Close();
    delete s;
    printf("client exit.\n");
    return 0;
}