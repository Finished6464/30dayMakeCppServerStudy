#ifndef SOCKET_X_H_
#define SOCKET_x_H_

#include "inetaddress.h"

class SocketX 
{
public:
    SocketX();
    SocketX(int fd);
    
    int fd() const { return fd_; }
    int Bind(InetAddress* addr);
    int Listen();
    int Accept(InetAddress* addr);
    int Connect(const char* addr, int port);
    void SetBlockMode(bool enable);
    void Close();

private:
    int fd_;
};

#endif //SOCKET_X_H_