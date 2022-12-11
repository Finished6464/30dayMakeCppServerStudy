#include "connection.h"
#include <unistd.h>
#include <cstring>

Connection::Connection(int fd)
: loop_(nullptr)
, fd_(fd)
, channel_(nullptr)
, disconn_callback_(nullptr)
, read_buff_(new Buffer)
, write_buff_(new Buffer)
, state_(State::Waiting)
{
}

Connection::Connection(EventLoop *loop, int fd, std::function<void(Connection*)> proc_callback, std::function<void(int)> disconn_callback)
: loop_(loop)
, fd_(fd)
, disconn_callback_(disconn_callback)
, read_buff_(new Buffer)
, write_buff_(new Buffer)
, state_(State::Waiting)
{
    // channel_ = new Channel(loop_->ep(), fd_, std::bind(&Connection::Echo, this)); //该连接的Channel
    channel_ = new Channel(loop_->ep(), fd_, std::bind(proc_callback, this));
    channel_->EnableReading(EPOLLIN | EPOLLET); //打开读事件监听
}

Connection::~Connection()
{
    Close();
    if (channel_) delete channel_;
    if (read_buff_) delete read_buff_;
    if (write_buff_) delete write_buff_;
}

void Connection::Close()
{
    if (fd_ >= 0) {
        if (disconn_callback_)  disconn_callback_(fd_);
        close(fd_);
        fd_ = -1;
    }

    read_buff_->Clear();
    write_buff_->Clear();
    state_ = State::Closed;
}

void Connection::Read() 
{
    if (fd_ < 0) {
        printf("fd is invalid.\n");
        return;
    }

    if (state_ != State::Waiting) {
        printf("state is not waiting.\n");
        return;
    }

    read_buff_->Clear();
    state_ = State::Reading;
    if (channel_) 
        ReadNonBlocking();
    else 
        ReadBlocking();
    if (state_ == State::Reading)
        state_ = State::Waiting;
}

void Connection::Write() 
{
    if (fd_ < 0) {
        printf("fd is invalid.\n");
        return;
    }

    if (state_ != State::Waiting) {
        printf("state is not waiting.\n");
        return;
    }

    state_ = State::Writing;
    if (channel_) 
        WriteNonBlocking();
    else
        WriteBlocking();

    if (state_ == State::Writing)
        state_ = State::Waiting;
}

void Connection::ReadNonBlocking()
{
    // 回显sockfd发来的数据
    char buf[1024] = {0};     //定义缓冲区
    // memset(&buf, 0, sizeof(buf));       //清空缓冲区    
    while (true) {    //由于使用非阻塞IO，需要不断读取，直到全部读取完毕
        ssize_t bytes_read = read(fd_, buf, sizeof(buf));
        if (bytes_read > 0) { //保存读取到的bytes_read大小的数据            
            read_buff_->Append(buf, bytes_read);
        } 
        else if (bytes_read == -1 && errno == EINTR) {  //客户端正常中断、继续读取
            continue;
        } 
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
            // //该fd上数据读取完毕
            // printf("message from client fd %d: %s\n", fd_, buf);  
            // // write(fd_, buf, bytes_read);           //将相同的数据写回到客户端
            // errif(write(fd_, buff_->c_str(), buff_->Size()) == -1, "socket write error");
            // buff_->Clear();
            break;
        } 
        else if (bytes_read == 0) {  //EOF事件，一般表示客户端断开连接
            printf("read EOF, client fd (%d) disconnected\n", fd_);
            Close();
            break;
        } //剩下的bytes_read == -1的情况表示其他错误
        else {
            printf("Other error on client fd (%d)\n", fd_);
            Close();
            break;
        }
    }
}

void Connection::ReadBlocking() 
{ 
    char buf[1024] = {0};
    ssize_t bytes_read = read(fd_, buf, sizeof(buf));
    if (bytes_read > 0) {
        read_buff_->Append(buf, bytes_read);
    } 
    else if (bytes_read == 0) {
        printf("read EOF, blocking client fd (%d) disconnected\n", fd_);
        Close();
    } 
    else if (bytes_read == -1) {
        printf("Other error on blocking client fd (%d)\n", fd_);
        Close();
    }
}

void Connection::WriteNonBlocking() 
{
  const char *data = write_buff_->c_str();
  int data_size = write_buff_->Size();
  int offset = 0;
  while (offset < data_size) {
    ssize_t bytes_write = write(fd_, data + offset, data_size - offset);
    if (bytes_write == -1 && errno == EINTR) {
        printf("continue writing\n");
        continue;
    }
    if (bytes_write == -1 && errno == EAGAIN) {
      break;
    }
    if (bytes_write == -1) {
      printf("Other error on client fd (%d)\n", fd_);
      Close();
      break;
    }
    offset += bytes_write;
  }
}

void Connection::WriteBlocking()
{
    ssize_t bytes_write = write(fd_, write_buff_->c_str(), write_buff_->Size());
    if (bytes_write == -1) {
        printf("Other error on blocking client fd (%d)\n", fd_);
        Close();
    }        
}

void Connection::SetSendStr(const char* str) 
{ 
    SetSendStr(str, strlen(str) + 1);
}

void Connection::SetSendStr(const char* str, int size)
{
    if (!str ) {
        fprintf(stderr, "set send buff failed!\n");
        return;
    }

    write_buff_->Clear();
    write_buff_->Append(str, size); 
}

// void Connection::Echo()
// {
//     // 回显sockfd发来的数据
//     if (fd_) {
//         char buf[1024] = {0};     //定义缓冲区
//         // memset(&buf, 0, sizeof(buf));       //清空缓冲区
//         while (true) {    //由于使用非阻塞IO，需要不断读取，直到全部读取完毕
//             ssize_t bytes_read = read(fd_, buf, sizeof(buf));
//             if (bytes_read > 0) { //保存读取到的bytes_read大小的数据            
//                 buff_->Append(buf, bytes_read);
//             } else if (bytes_read == -1 && errno == EINTR) {  //客户端正常中断、继续读取
//                 continue;
//             } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//非阻塞IO，这个条件表示数据全部读取完毕
//                 //该fd上数据读取完毕
//                 printf("message from client fd %d: %s\n", fd_, buf);  
//                 // write(fd_, buf, bytes_read);           //将相同的数据写回到客户端
//                 errif(write(fd_, buff_->c_str(), buff_->Size()) == -1, "socket write error");
//                 buff_->Clear();
//                 break;
//             } else if (bytes_read == 0) {  //EOF事件，一般表示客户端断开连接
//                 printf("client fd %d disconnected\n", fd_);
//                 //close(fd_);   //关闭socket会自动将文件描述符从epoll树上移除
//                 disconn_callback_(fd_);
//                 break;
//             } //剩下的bytes_read == -1的情况表示其他错误
//             else {
//                 // close(fd_);
//                 disconn_callback_(fd_);
//                 errif(true, "socket read error");
//             }
//         }
//     }
// }