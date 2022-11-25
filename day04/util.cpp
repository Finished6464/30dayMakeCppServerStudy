#include "util.h"
#include <stdio.h>
// #include <fcntl.h>

void errif(bool condition, const char *errmsg)
{
    if (condition) {
        perror(errmsg);
    }
}

// void setnonblocking(int fd)
// {
//     int ret = fcntl(fd, F_GETFL, 0);
//     errif(ret == -1, "fcntl F_GETFL error");
//     errif(fcntl(fd, F_SETFL, ret | O_NONBLOCK) == -1, "fcntl F_SETFL error");
// }