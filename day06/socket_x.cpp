#include "socket_x.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "util.h"

SocketX::SocketX()
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd_ == -1, "init socket error.");
}

SocketX::SocketX(int fd)
:fd_(fd)
{
}

int SocketX::Bind(InetAddress* addr)
{
    int ret = bind(fd(), (sockaddr*)addr->addr(), addr->addr_size());
    errif(ret == -1, "bind error.");
    return ret;
}

int SocketX::Listen()
{ 
    int ret = listen(fd(), SOMAXCONN); 
    errif(ret == -1, "listen error.");
    return ret;
}

int SocketX::Accept(InetAddress* addr)
{
    socklen_t len = addr->addr_size();
    int ret = accept(fd(), (sockaddr*)addr->addr(), &len);
    errif(ret == -1, "accept error.");
    return ret;
}

int SocketX::Connect(const char* addr, int port)
{
    InetAddress serv_addr(addr, port);
    int ret = connect(fd(), (sockaddr*)(serv_addr.addr()), serv_addr.addr_size()); 
    errif(ret == -1, "connect error.");
    return ret;
}

void SocketX::Close()
{
    if (fd() >= 0) close(fd());
}

void SocketX::SetBlockMode(bool enable)
{
    int flags = fcntl(fd(), F_GETFL, 0);
    errif(flags == -1, "fcntl F_GETFL error");
    if (enable) flags &= ~O_NONBLOCK;
    else flags |= O_NONBLOCK;
    errif(fcntl(fd(), F_SETFL, flags) == -1, "fcntl F_SETFL error");
}