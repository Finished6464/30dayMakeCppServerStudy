#ifndef EPOLL_X_H_
#define EPOLL_X_H_

#include <sys/epoll.h>
#include <vector>

typedef std::vector<epoll_event> EPOLLEVENTS;

class EpollX
{
public:
    EpollX();
    ~EpollX();

    int AddFd(int socket_fd, uint32_t ev);
    EPOLLEVENTS Poll();

private:
    int ep_fd_;     
};

#endif //EPOLL_X_H_