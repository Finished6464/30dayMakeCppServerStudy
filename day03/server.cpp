#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>

#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

void setnonblocking(int fd)
{
    int ret = fcntl(fd, F_GETFL, 0);
    errif(ret == -1, "fcntl F_GETFL error");
    errif(fcntl(fd, F_SETFL, ret | O_NONBLOCK) == -1, "fcntl F_SETFL error");
}

void handleEvent(int fd)
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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8887);
    errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0);
#define MAX_EVENTS 10    
    struct epoll_event events[MAX_EVENTS], ev;
    ev.events = EPOLLIN;    //在代码中使用了ET模式，且未处理错误，在day12进行了修复，实际上接受连接最好不要用ET模式
    ev.data.fd = sockfd;    //该IO口为服务器socket fd
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);    //将服务器socket fd添加到epoll
    struct sockaddr_in clnt_addr;
    while (true) {    // 不断监听epoll上的事件并处理
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);   //有nfds个fd发生事件
        for (int i = 0; i < nfds; ++i) {  //处理这nfds个事件
            if (events[i].data.fd == sockfd) {    //发生事件的fd是服务器socket fd，表示有新客户端连接                
                socklen_t clnt_addr_len = sizeof(clnt_addr);
                memset(&clnt_addr, 0, sizeof(clnt_addr));
                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
                ev.data.fd = clnt_sockfd;   
                ev.events = EPOLLIN | EPOLLET;  //对于客户端连接，使用ET模式，可以让epoll更加高效，支持更多并发
                setnonblocking(clnt_sockfd);    //ET需要搭配非阻塞式socket使用
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);   //将该客户端的socket fd添加到epoll
            } else if (events[i].events & EPOLLIN) {      //发生事件的是客户端，并且是可读事件（EPOLLIN）
                handleEvent(events[i].data.fd);         //处理该fd上发生的事件
            }
        }
    }

    printf("server exit 0.\n");
    return 0;
}
