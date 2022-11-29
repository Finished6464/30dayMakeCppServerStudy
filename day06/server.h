#ifndef SERVER_H_
#define SERVER_H_

#include "channel.h"


class EventLoop 
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void UpdateChannel(Channel*);

    void RequireQuit() { require_quit_ = true; }

    EpollX * ep() const { return ep_; }

private:
    EpollX *ep_;
    bool require_quit_;
};


class Server
{
public:
    Server(EventLoop *loop = nullptr);
    ~Server();

    bool Startup(const char* addr, int port);
    void HandleReadEvent(int clnt_fd);
    void NewConnection(int serv_fd);

private:
    EventLoop *loop_;
    EPOLLEVENTS channels_;
};

#endif //SERVER_H_