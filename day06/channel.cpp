#include "channel.h"
#include <cstring>
#include <stdio.h>
#include "util.h"

Channel::Channel()
: ep_(nullptr)
, fd_(-1)
, events_(0)
, revents_(0)
, in_epoll_(false)
, callback_(nullptr)
{
}

Channel::Channel(EpollX *ep, int fd)
: ep_(ep)
, fd_(fd)
, events_(0)
, revents_(0)
, in_epoll_(false)
, callback_(nullptr)
{
}

Channel::Channel(EpollX *ep, int fd, std::function<void()> callback)
: ep_(ep)
, fd_(fd)
, events_(0)
, revents_(0)
, in_epoll_(false)
, callback_ (callback)
{    
}

void Channel::HandleEvent()
{
    if (fd_ && callback_)
        callback_();
}

int Channel::EnableReading(uint32_t ev/* = EPOLLIN | EPOLLET*/)
{
    if (ep_) {
        SetEvents(ev);
        return ep_->UpdateChannel(this);
    }
    else {
        fprintf(stderr, "ep_ is invalid.\n");
        return -1;
    }    
}
