#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <functional>
#include "eventloop.h"
#include "../day06/socket_x.h"
#include "../day09/util.h"

class Connection
{
public:
    enum State
    {
        Waiting = 0,
        Reading,
        Writing,
        Closed
    };
    Connection(int fd);
    Connection(EventLoop *loop, int fd, std::function<void(Connection*)> proc_callback, std::function<void(int)> disconn_callback);
    ~Connection();

    int fd() const { return fd_; };
    int GetState() const { return state_; };

    void Close();

    void Read();
    void Write();
    
    const char* GetReadStr() { return read_buff_ != nullptr ? read_buff_->c_str() : nullptr; }
    void SetSendStr(const char* str);
    void SetSendStr(const char* str, int size);

private:
    void ReadNonBlocking();
    void ReadBlocking();
    void WriteNonBlocking();
    void WriteBlocking();

private:
    EventLoop *loop_;
    int fd_;
    Channel *channel_;
    std::function<void(int)> disconn_callback_;
    Buffer* read_buff_;
    Buffer* write_buff_;
    int state_;
};

#endif //_CONNECTION_H_