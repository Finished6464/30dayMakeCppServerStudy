#ifndef EPOLL_X_H_
#define EPOLL_X_H_

#include <sys/epoll.h>
#include <vector>

class Channel;
typedef std::vector<Channel*> EPOLLEVENTS;

class EpollX
{
public:
    EpollX();
    ~EpollX();

    int UpdateChannel(Channel *channel);
    // int AddFd(int socket_fd, uint32_t ev);
    EPOLLEVENTS Poll();

private:
    int ep_fd_;
    EPOLLEVENTS channel_ptrs_;
};

#endif //EPOLL_X_H_