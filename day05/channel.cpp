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
{
}

Channel::Channel(EpollX *ep, int fd)
: events_(0)
, revents_(0)
, in_epoll_(false)
{
    ep_ = ep;
    fd_ = fd;
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
