#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "channel.h"
#include "util.h"
#include "socket_x.h"

static void handleEvent(int fd)
{
    ssize_t offset = 0; 
    char buf[1024] = {0};     //定义缓冲区
    // memset(&buf, 0, sizeof(buf));       //清空缓冲区
    while (true) {    //由于使用非阻塞IO，需要不断读取，直到全部读取完毕
        ssize_t bytes_read = read(fd, buf + offset, sizeof(buf));
        if (bytes_read > 0) { //保存读取到的bytes_read大小的数据            
            offset += bytes_read;
        } else if (bytes_read == -1 && errno == EINTR) {  //客户端正常中断、继续读取
            continue;
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
            //该fd上数据读取完毕
            printf("message from client fd %d: %s\n", fd, buf);  
            write(fd, buf, sizeof(buf));           //将相同的数据写回到客户端
            break;
        } else if (bytes_read == 0) {  //EOF事件，一般表示客户端断开连接
            printf("client fd %d disconnected\n", fd);
            close(fd);   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        } //剩下的bytes_read == -1的情况表示其他错误
        else {
            close(fd);
            errif(true, "socket read error");
        }
    }
}

int main()
{
    EPOLLEVENTS channels;
    SocketX serv_sock;
    InetAddress serv_addr("127.0.0.1", 8888);
    if (serv_sock.Bind(&serv_addr) != -1 && serv_sock.Listen() != -1) {
        EpollX ep;
        // ep.AddFd(serv_sock.fd(), EPOLLIN);
        Channel *serv_channel = new Channel(&ep, serv_sock.fd());
        channels.push_back(serv_channel);
        if (serv_channel->EnableReading(EPOLLIN) != -1) {
            InetAddress clnt_addr;
            while (true) {
                EPOLLEVENTS events = ep.Poll();
                for (std::size_t i = 0; i < events.size(); i++) {
                    if (events[i]->fd() == serv_channel->fd()) {    //发生事件的fd是服务器socket fd，表示有新客户端连接     
                        clnt_addr.Reset();
                        SocketX clnt_sock(serv_sock.Accept(&clnt_addr));
                        if (clnt_sock.fd() != -1) {
                            printf("new client fd %d! IP: %s Port: %d\n", clnt_sock.fd(), inet_ntoa(clnt_addr.addr()->sin_addr), ntohs(clnt_addr.addr()->sin_port));
                            clnt_sock.SetBlockMode(false);
                            Channel *clnt_channel = new Channel(&ep, clnt_sock.fd());
                            channels.push_back(clnt_channel);
                            clnt_channel->EnableReading(EPOLLIN | EPOLLET);
                        }
                    } else if ((events[i])->revents() & EPOLLIN) {      //发生事件的是客户端，并且是可读事件（EPOLLIN）
                        // printf("begin handle client event.\n");
                        handleEvent(events[i]->fd());         //处理该fd上发生的事件
                    }
                }
            }
        }
    }

    for (std::size_t i = 0; i < channels.size(); i++)
        delete channels[i];
    serv_sock.Close();
    printf("server exit.\n");
    return 0;
}