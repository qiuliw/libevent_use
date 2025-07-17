#pragma once
#include <event2/event.h>

class XThread {
public:
    XThread();
    ~XThread();

    void Start();
    void Main();
    bool Setup();
    bool Notify();
    bool ReadNotify();

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
};