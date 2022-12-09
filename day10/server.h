#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "../day06/channel.h"
#include "../day06/socket_x.h"
#include "../day09/util.h"


class ThreadPoll 
{
public:
    ThreadPoll(int size = 10);  // 默认size最好设置为std::thread::hardware_concurrency()
    ~ThreadPoll();
    bool Add(std::function<void()> func);

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex tasks_mtx_;
    std::condition_variable cv_;
    bool stoped_;    
};


class EventLoop 
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void RequireQuit() { require_quit_ = true; }

    EpollX * ep() const { return ep_; }

private:
    EpollX *ep_;
    bool require_quit_;
    ThreadPoll *thread_poll_;
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
    Connection(EventLoop *loop, int fd, std::function<void(int)> callback);
    ~Connection();

    void Echo();

private:
    EventLoop *loop_;
    int fd_;
    Channel *channel_;
    std::function<void(int)> callback_;
    Buffer* buff_;
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