#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include "../day06/channel.h"
#include "../day06/socket_x.h"
#include "../day09/util.h"


class ThreadPool
{
public:
    ThreadPool(int size = 10);  // 默认size最好设置为std::thread::hardware_concurrency()
    ~ThreadPool();
    // bool Add(std::function<void()> func);
    // template<class F, class... Args>
    // auto ThreadPool::Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
template<class F, class... Args>
auto Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex tasks_mtx_;
    std::condition_variable cv_;
    bool stoped_;    
};

template<class F, class... Args>
auto ThreadPool::Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> 
{
    using return_type = typename std::result_of<F(Args...)>::type;  //返回值类型

    auto task = std::make_shared< std::packaged_task<return_type()> > (  //使用智能指针
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)  //完美转发参数
        );  

    std::future<return_type> res = task->get_future();  // 使用期约
    {   //队列锁作用域
        std::unique_lock<std::mutex> lock(tasks_mtx_);   //加锁

        if(stoped_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks_.emplace([task](){ (*task)(); });  //将任务添加到任务队列
    }
    cv_.notify_one();    //通知一次条件变量
    return res;     //返回一个期约
}


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
    // ThreadPool *thread_poll_;
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
};
#endif //SERVER_H_