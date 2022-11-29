#include "inetaddress.h"
#include <cstring>

InetAddress::InetAddress()
{
    Reset();
}

InetAddress::InetAddress(const char* addr, int port)
{
    Reset();
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(addr);
    addr_.sin_port = htons(port);
}

void InetAddress::Reset()
{ 
    memset(&addr_, 0, sizeof(addr_)); 
}

