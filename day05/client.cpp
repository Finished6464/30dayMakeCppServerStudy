#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "util.h"
#include "socket_x.h"

int main()
{
    SocketX *s = new SocketX();
    if (s->Connect("127.0.0.1", 8888) != -1) {
        char buf[1024];     //定义缓冲区
        while (true) {
            memset(&buf, 0, sizeof(buf));       //清空缓冲区
            scanf("%s", buf);             //从键盘输入要传到服务器的数据
            ssize_t write_bytes = write(s->fd(), buf, sizeof(buf));      //发送缓冲区中的数据到服务器socket，返回已发送数据大小
            if (write_bytes == -1) {          //write返回-1，表示发生错误
                printf("socket already disconnected, can't write any more!\n");
                break;
            }
            memset(&buf, 0, sizeof(buf));       //清空缓冲区 
            ssize_t read_bytes = read(s->fd(), buf, sizeof(buf));    //从服务器socket读到缓冲区，返回已读数据大小
            if (read_bytes > 0) {
                printf("message from server: %s\n", buf);
            }
            else if (read_bytes == 0) {      //read返回0，表示EOF，通常是服务器断开链接，等会儿进行测试
                printf("server socket disconnected!\n");
                break;
            }
            else if (read_bytes == -1) {     //read返回-1，表示发生错误，按照上文方法进行错误处理
                errif(true, "socket read error");
                break;
            }
        }
    }

    s->Close();
    delete s;
    printf("client exit.\n");
    return 0;
}