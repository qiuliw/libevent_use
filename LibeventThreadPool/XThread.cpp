
#include "XThread.h"
#include "XTask.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <event2/event_compat.h>
#include <mutex>
#include <unistd.h>
#include <event2/event.h>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <event2/util.h>
#else
#include <sys/eventfd.h>
#endif

// 激活线程任务的回调函数
static void NotifyCB(evutil_socket_t fd, short which, void *arg){
    XThread* t = (XThread*)arg;
    
    bool re = t->Run();
    if(!re)
        return;
    
    std::cout << t->tId_ << ": 线程被唤醒，NotifyCB " << std::endl;
}

XThread::XThread() {
    // 构造函数仅初始化基本成员
}

XThread::~XThread() {
    // 清理资源
#ifdef _WIN32
    if (notifyFds_[0] != INVALID_SOCKET) closesocket(notifyFds_[0]);
    if (notifyFds_[1] != INVALID_SOCKET) closesocket(notifyFds_[1]);
#else
    if (efd_ != -1) close(efd_);
#endif
}

// 初始化通知管道
bool XThread::InitNotifier() {
#ifdef _WIN32
    if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, notifyFds_) < 0) {
        perror("socketpair create error");
        return false;
    }
    evutil_make_socket_nonblocking(notifyFds_[0]);
    evutil_make_socket_nonblocking(notifyFds_[1]);

    std::cout << "XThread::Setup() notifyFds_[0]=" << notifyFds_[0] 
            << ", notifyFds_[1]=" << notifyFds_[1] << std::endl;
#else
    efd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (efd_ == -1) {
        perror("eventfd");
        return false;
    }

    std::cout << "XThread::Setup() efd_=" << efd_ << std::endl;
#endif

    return true;

}

// 注册管道到base 监听
bool XThread::ListenNotifier(event_base* base, event_callback_fn cb, void* arg) {
    if (!base) {
        fprintf(stderr, "ListenNotifier: base is nullptr\n");
        return false;
    }

#ifdef _WIN32
    evutil_socket_t fd = notifyFds_[0];
#else
    int fd = efd_;
#endif

    struct event* ev = event_new(base, fd, EV_READ | EV_PERSIST, cb, arg);
    if (!ev) {
        fprintf(stderr, "ListenNotifier: event_new failed\n");
        return false;
    }

    if (event_add(ev, nullptr) < 0) {
        fprintf(stderr, "ListenNotifier: event_add failed\n");
        event_free(ev);
        return false;
    }

    return true;
}

bool XThread::Notify() {
    // 整合原来的 EventNotifier::Notify() 功能
#ifdef _WIN32
    char buf = '1';
    if (send(notifyFds_[1], &buf, 1, 0) != 1) {
        perror("send");
        return false;
    }
#else
    uint64_t one = 1;
    if (write(efd_, &one, sizeof(one)) != sizeof(one)) {
        perror("write");
        return false;
    }
#endif
    std::cout << "Notify" << std::endl;
    return true;
}

// 接受消息，执行队列中的任务
bool XThread::ReadNotify() {
    // 整合原来的 EventNotifier::ReadEvent() 功能
    std::cout << "XThread::ReadNotify" << std::endl;
#ifdef _WIN32
    char buf;
    if (recv(notifyFds_[0], &buf, 1, 0) == 1) {
        return true;
    }
    return false;
#else
    uint64_t count;
    if (read(efd_, &count, sizeof(count)) == sizeof(count)) {
        return true;
    }
    return false;
#endif
}

// 读取消息并执行任务
bool XThread::Run() {
    if(!ReadNotify()) return true; 

    std::unique_lock<std::mutex> lk(tasksMutex_);
    XTask* task = nullptr;
    if (tasks_.empty()) {
        return true;
    }
    task = tasks_.front(); // 先进先出
    tasks_.pop_front();
    lk.unlock();
    task->Init();
    return true;
}


// 给线程添加task
void XThread::AddTask(XTask *task)
{
    if(!task)return;
    task->base_ = base_;
    
    // 添加任务的线程可能和执行任务的线程竞争，需要锁
    std::lock_guard<std::mutex> lk(tasksMutex_);
    tasks_.push_back(task);
}


// 初始化通知机制
bool XThread::Setup() {
    if (!InitNotifier()) {
        return false;
    }

    event_config* cfg = event_config_new();
    event_config_set_flag(cfg, EVENT_BASE_FLAG_NOLOCK);
    base_ = event_base_new_with_config(cfg);
    event_config_free(cfg);
    if (!base_) {
        perror("event_base_new_with_config");
        return false;
    }

    if (!ListenNotifier(base_,NotifyCB, this)) {
        return false;
    }

    return true;
}

void XThread::Start() {
    std::thread th(&XThread::Main, this);
    std::cout << tId_ << ": XThread::Main() Start" << std::endl;
    th.detach();
}

void XThread::Main() {
    std::cout << "XThread::Main() begin" << std::endl;

    event_base_dispatch(base_);
    
    std::cout << "XThread::Main() end" << std::endl;
}