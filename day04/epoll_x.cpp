#include "epoll_x.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "util.h"


EpollX::EpollX()
{
    ep_fd_ = epoll_create1(0);
    // memset(&ev_, 0, sizeof(ev_));
}

EpollX::~EpollX()
{
    close(ep_fd_);
}

int EpollX::AddFd(int socket_fd, uint32_t ev)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = ev;
    event.data.fd = socket_fd;
    int ret = epoll_ctl(ep_fd_, EPOLL_CTL_ADD, socket_fd, &event);
    errif(ret == -1, "epoll_ctl error.");
    return ret;
}

EPOLLEVENTS EpollX::Poll()
{
#define MAX_EVENTS 10    
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(ep_fd_, events, MAX_EVENTS, -1);
    errif(nfds == -1, "epoll_wait error.");
    EPOLLEVENTS ret;
    for (int i = 0; i < nfds; i++)
        ret.push_back(events[i]);
    return ret;
}