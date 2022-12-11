#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include "connection.h"
#include "eventloop.h"


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




class Server
{
public:
    Server(EventLoop *loop, std::function<void(Connection*)> conn_callback);
    ~Server();

    bool Startup(const char* addr, int port);
    void Stop();
    
    // void HandleReadEvent(int clnt_fd);
    void NewConnection(int fd);
    void DeleteConnection(int fd);

private:
    // EventLoop *loop_;
    // Acceptor* acceptor_;
    // std::map<int, Connection*> connections_; //所有TCP连接
    EventLoop *main_reactor_;     //只负责接受连接，然后分发给一个subReactor
    Acceptor *acceptor_;                     //连接接受器
    std::map<int, Connection*> connections_; //TCP连接
    std::vector<EventLoop*> sub_reactors_;    //负责处理事件循环
    ThreadPool *thpool_;     //线程池

    std::function<void(Connection*)> conn_callback_;
};
#endif //SERVER_H_