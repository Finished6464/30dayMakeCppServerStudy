#include "eventloop.h"
#include "stdio.h"

ThreadPool::ThreadPool(int size)
: stoped_(false)
{
    for (int i = 0; i < size; ++i) {  //  启动size个线程
        threads_.emplace_back(std::thread([this]() {  //定义每个线程的工作函数
            while (true) {    
                std::function<void()> task;
                {   //在这个{}作用域内对std::mutex加锁，出了作用域会自动解锁，不需要调用unlock()
                    std::unique_lock<std::mutex> lock(tasks_mtx_);
                    cv_.wait(lock, [this]() {     //等待条件变量，条件为任务队列不为空或线程池停止
                        return stoped_ || !tasks_.empty();
                    });
                    if (stoped_ && tasks_.empty()) return;   //任务队列为空并且线程池停止，退出线程
                    task = tasks_.front();   //从任务队列头取出一个任务
                    tasks_.pop();
                }

                task();     //执行任务
            }
        }));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(tasks_mtx_);
        stoped_ = true;
    }

    cv_.notify_all();
    printf("wait for thread poll finish.\n");
    for (auto& t : threads_)
        t.join();
    printf("all of threads finish.\n");
}


EventLoop::EventLoop()
: require_quit_(false)
{
    ep_ = new EpollX;
    // thread_poll_ = new ThreadPool;
}

EventLoop::~EventLoop()
{    
    // delete thread_poll_;
    delete ep_;
}

void EventLoop::Loop()
{
    while (!require_quit_) {
        // EPOLLEVENTS chs = ep_->Poll();
        // for (EPOLLEVENTS::iterator it = chs.begin(); it != chs.end(); ++it) {
        //     if (((*it)->events() & EPOLLET) > 0)
        //         thread_poll_->Add(std::bind(&Channel::HandleEvent, *it));
        //     else
        //         (*it)->HandleEvent();
        // }
        for (auto& ch : ep_->Poll())
            ch->HandleEvent();
    }
}