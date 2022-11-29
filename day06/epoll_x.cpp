#include "epoll_x.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "util.h"
#include "channel.h"


EpollX::EpollX()
{
    ep_fd_ = epoll_create1(0);
    // memset(&ev_, 0, sizeof(ev_));
}

EpollX::~EpollX()
{
    close(ep_fd_);
    // for (std::size_t i = 0; i < channel_ptrs_.size(); i++)
    //     delete channel_ptrs_[i];
}

// int EpollX::AddFd(int socket_fd, uint32_t ev)
// {
//     struct epoll_event event;
//     memset(&event, 0, sizeof(event));
//     event.events = ev;
//     event.data.fd = socket_fd;
//     int ret = epoll_ctl(ep_fd_, EPOLL_CTL_ADD, socket_fd, &event);
//     errif(ret == -1, "epoll_ctl error.");
//     return ret;
// }

EPOLLEVENTS EpollX::Poll()
{
#define MAX_EVENTS 10    
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(ep_fd_, events, MAX_EVENTS, -1);
    errif(nfds == -1, "epoll_wait error.");
    EPOLLEVENTS ret;
    for (int i = 0; i < nfds; i++) {
        Channel *channel = (Channel *)events[i].data.ptr;
        channel->SetRevents(channel->events());
        ret.push_back(channel);
    }
        
    return ret;
}

int EpollX::UpdateChannel(Channel *channel)
{
    int fd = channel->fd();  //拿到Channel的文件描述符
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = channel->events();   //拿到Channel希望监听的事件
    ev.data.ptr = channel;
    channel->SetEvents(ev.events);
    int ret;
    if (!channel->IsInEpoll()) {
        // Channel *ptr = new Channel(this, fd);
        // ptr->SetEvents(ev.events);
        // ptr->SetRevents(channel->revents());
        // ev.data.ptr = ptr;
        ret = epoll_ctl(ep_fd_, EPOLL_CTL_ADD, fd, &ev);//添加Channel中的fd到epoll  
        if (ret != -1)  {
            channel->SetInEpolll(true);
            // channel_ptrs_.push_back(ptr);
        }
        else {
            errif(true, "epoll add error");
        }
    } 
    else {
        // ev.data.ptr = channel;        
        ret = epoll_ctl(ep_fd_, EPOLL_CTL_MOD, fd, &ev);
        errif(ret == -1, "epoll modify error");//已存在，则修改
    }

    return ret;
}