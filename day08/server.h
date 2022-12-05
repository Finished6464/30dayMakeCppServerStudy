#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include "../day06/channel.h"
#include "../day06/socket_x.h"


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


class Acceptor
{
public:
    Acceptor(EventLoop *loop, std::function<void(int)> callback);
    ~Acceptor();

    bool Prepare(const char* addr, int port);
    void Connect();

private:
    EventLoop *loop_;
    SocketX *sock_;
    // InetAddress *addr_;
    Channel *channel_;
    std::function<void(int)> callback_;
};


class Connection
{
public:
    Connection(EventLoop *loop, int fd, std::function<void()> callback);
    ~Connection();

    void Echo();

private:
    EventLoop *loop_;
    int fd_;
    Channel *channel_;
    std::function<void()> callback_;
};

class Server
{
public:
    Server(EventLoop *loop = nullptr);
    ~Server();

    bool Startup(const char* addr, int port);
    // void HandleReadEvent(int clnt_fd);
    void NewConnection(int fd);
    void DeleteConnection(int fd);

private:
    EventLoop *loop_;
    // EPOLLEVENTS channels_;
    Acceptor* acceptor_;
    std::map<int, Connection*> connections_; //所有TCP连接
};

#endif //SERVER_H_