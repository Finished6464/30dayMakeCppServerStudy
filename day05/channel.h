#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "epoll_x.h"

class Channel 
{
public:
    Channel();
    Channel(EpollX *ep, int fd);
    
    //EnableReading(uint32_t ev = EPOLLIN | EPOLLET);
    int EnableReading(uint32_t ev = EPOLLIN | EPOLLET);
    int fd() const { return fd_; }
    uint32_t events() const { return events_; }
    void SetEvents(uint32_t ev) { events_ = ev; }
    uint32_t revents() const { return revents_; }
    void SetRevents(uint32_t ev) { revents_ = ev; }
    bool IsInEpoll() const { return in_epoll_; }
    void SetInEpolll(bool enable) { in_epoll_ = enable; }

private:
    EpollX *ep_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool in_epoll_;
};

#endif //CHANNEL_H_