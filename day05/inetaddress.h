#ifndef _INETADDRESS_H_
#define _INETADDRESS_H_

#include <arpa/inet.h>

class InetAddress
{
public:
    InetAddress();
    InetAddress(const char* addr, int port);

    struct sockaddr_in* addr() { return &addr_; }
    socklen_t addr_size() { return sizeof(addr_); }

    void Reset();

private:
    struct sockaddr_in addr_;
};


#endif //_INETADDRESS_H_