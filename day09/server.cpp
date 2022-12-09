#include "server.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>



EventLoop::EventLoop()
: require_quit_(false)
{
    ep_ = new EpollX;
}

EventLoop::~EventLoop()
{
    delete ep_;
}

void EventLoop::Loop()
{
    while (!require_quit_) {
        EPOLLEVENTS chs = ep_->Poll();
        for (auto it = chs.begin(); it != chs.end(); ++it) {
            (*it)->HandleEvent();
        }
    }
}

Acceptor::Acceptor(EventLoop *loop, std::function<void(int)> callback)
: loop_(loop)
, sock_(nullptr)
// , addr_(nullptr)
, channel_(nullptr)
, callback_(callback)
{
}

bool Acceptor::Prepare(const char* addr, int port)
{
    InetAddress serv_addr(addr, port);
    sock_ = new SocketX;
    if (sock_->Bind(&serv_addr) != -1 && sock_->Listen() != -1) {
        channel_ = new Channel(loop_->ep(), sock_->fd(), std::bind(&Acceptor::Connect, this));
        channel_->EnableReading(EPOLLIN);
        return true;
    }

    return false;
}

Acceptor::~Acceptor()
{
    if (channel_) {
        delete channel_;
    }

    if (sock_) {
        sock_->Close();
        delete sock_;
    }
}

void Acceptor::Connect()
{
    if (sock_)  {
        InetAddress clnt_addr;
        clnt_addr.Reset();
        SocketX clnt_sock(sock_->Accept(&clnt_addr));
        if (clnt_sock.fd() != -1) {
            printf("new client fd %d! IP: %s Port: %d\n", clnt_sock.fd(), inet_ntoa(clnt_addr.addr()->sin_addr), ntohs(clnt_addr.addr()->sin_port));
            clnt_sock.SetBlockMode(false);
            callback_(clnt_sock.fd());
        }    
    }
}

Connection::Connection(EventLoop *loop, int fd, std::function<void(int)> callback)
: loop_(loop)
, fd_(fd)
, callback_(callback)
{
    channel_ = new Channel(loop_->ep(), fd_, std::bind(&Connection::Echo, this)); //该连接的Channel
    channel_->EnableReading(EPOLLIN | EPOLLET); //打开读事件监听

    buff_ = new Buffer;
}

Connection::~Connection()
{
    if (channel_) delete channel_;
    if (buff_) delete buff_;
}

void Connection::Echo()
{
    // 回显sockfd发来的数据
    if (fd_) {
        char buf[1024] = {0};     //定义缓冲区
        // memset(&buf, 0, sizeof(buf));       //清空缓冲区
        while (true) {    //由于使用非阻塞IO，需要不断读取，直到全部读取完毕
            ssize_t bytes_read = read(fd_, buf, sizeof(buf));
            if (bytes_read > 0) { //保存读取到的bytes_read大小的数据            
                buff_->Append(buf, bytes_read);
            } else if (bytes_read == -1 && errno == EINTR) {  //客户端正常中断、继续读取
                continue;
            } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
                //该fd上数据读取完毕
                printf("message from client fd %d: %s\n", fd_, buf);  
                // write(fd_, buf, bytes_read);           //将相同的数据写回到客户端
                errif(write(fd_, buff_->c_str(), buff_->Size()) == -1, "socket write error");
                buff_->Clear();
                break;
            } else if (bytes_read == 0) {  //EOF事件，一般表示客户端断开连接
                printf("client fd %d disconnected\n", fd_);
                //close(fd_);   //关闭socket会自动将文件描述符从epoll树上移除
                callback_(fd_);
                break;
            } //剩下的bytes_read == -1的情况表示其他错误
            else {
                // close(fd_);
                callback_(fd_);
                errif(true, "socket read error");
            }
        }
    }
}

Server::Server(EventLoop *loop)
: loop_(loop)
, acceptor_(nullptr)
{    
}

Server::~Server()
{        
    // for (std::size_t i = 0; i < channels_.size(); i++)
    //     delete channels_[i];
    for (const auto& n : connections_) {
        close(n.first);
        delete n.second;
    }
}

bool Server::Startup(const char* addr, int port)
{
    // SocketX serv_sock;
    // InetAddress serv_addr(addr, port);
    // if (serv_sock.Bind(&serv_addr) != -1 && serv_sock.Listen() != -1) {
    //     Channel *serv_channel = new Channel(loop_->ep(), serv_sock.fd(), std::bind(&Server::NewConnection, this, serv_sock.fd()));
    //     serv_channel->EnableReading(EPOLLIN);
    //     channels_.push_back(serv_channel);
    //     return true;
    // }
    
    // return false;
    acceptor_ = new Acceptor(loop_, std::bind(&Server::NewConnection, this, std::placeholders::_1));
    return acceptor_->Prepare(addr, port);
}

// void Server::HandleReadEvent(int clnt_fd)
// {
//     ssize_t offset = 0; 
//     char buf[1024] = {0};     //定义缓冲区
//     // memset(&buf, 0, sizeof(buf));       //清空缓冲区
//     while (true) {    //由于使用非阻塞IO，需要不断读取，直到全部读取完毕
//         ssize_t bytes_read = read(clnt_fd, buf + offset, sizeof(buf));
//         if (bytes_read > 0) { //保存读取到的bytes_read大小的数据            
//             offset += bytes_read;
//         } else if (bytes_read == -1 && errno == EINTR) {  //客户端正常中断、继续读取
//             continue;
//         } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
//             //该fd上数据读取完毕
//             printf("message from client fd %d: %s\n", clnt_fd, buf);  
//             write(clnt_fd, buf, sizeof(buf));           //将相同的数据写回到客户端
//             break;
//         } else if (bytes_read == 0) {  //EOF事件，一般表示客户端断开连接
//             printf("client fd %d disconnected\n", clnt_fd);
//             close(clnt_fd);   //关闭socket会自动将文件描述符从epoll树上移除
//             break;
//         } //剩下的bytes_read == -1的情况表示其他错误
//         else {
//             close(clnt_fd);
//             errif(true, "socket read error");
//         }
//     }
// }

void Server::NewConnection(int fd)
{
    // SocketX serv_sock(serv_fd);
    // InetAddress clnt_addr;
    // clnt_addr.Reset();
    // SocketX clnt_sock(serv_sock.Accept(&clnt_addr));
    // if (clnt_sock.fd() != -1) {
    //     printf("new client fd %d! IP: %s Port: %d\n", clnt_sock.fd(), inet_ntoa(clnt_addr.addr()->sin_addr), ntohs(clnt_addr.addr()->sin_port));
    //     clnt_sock.SetBlockMode(false);
    //     Channel *clnt_channel = new Channel(loop_->ep(), clnt_sock.fd(), std::bind(&Server::HandleReadEvent, this, clnt_sock.fd()));        
    //     clnt_channel->EnableReading(EPOLLIN | EPOLLET);
    //     channels_.push_back(clnt_channel);
    // }
    Connection *conn = new Connection(loop_, fd, std::bind(&Server::DeleteConnection, this, std::placeholders::_1));
    connections_[fd] = conn;
}

void Server::DeleteConnection(int fd)
{
    // printf("DeleteConnection(%d)\n", fd);
    Connection *conn = connections_[fd];
    connections_.erase(fd);
    close(fd);
    delete conn;
    // printf("DeleteConnection(%d) ok.\n", fd);
}