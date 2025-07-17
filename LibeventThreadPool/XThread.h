#pragma once
#include <event2/event.h>
#include <list>
#include <thread>
#include <mutex>

#include "XTask.h"

class XThread {
public:
    XThread();
    ~XThread();

    void Start();
    void Main();
    bool Setup();
    bool Notify(); // 激活线程
    bool ReadNotify(); // 读取消息
    bool Run(); // 接受消息并执行任务

    // 添加处理的任务，一个线程同时处理多个任务，共用一个event_base
    void AddTask(XTask *task);

    int tId_ = 0; // 线程ID

private:
    // 将 EventNotifier 的功能直接实现为私有成员
    bool InitNotifier();
    bool ListenNotifier(event_base* base, event_callback_fn cb, void* arg);

    struct event_base* base_ = nullptr;
#ifdef _WIN32
    evutil_socket_t notifyFds_[2] = {INVALID_SOCKET, INVALID_SOCKET};
#else
    int efd_ = -1;
#endif

    // 任务列表
    std::list<XTask*> tasks_;
    // 锁
    std::mutex tasksMutex_;
};