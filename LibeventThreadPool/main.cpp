#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <event2/event.h>
#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <memory.h>
#include <string>
#include <unistd.h>

#include "XThread.h"
#include "XTthreadPool.h"
#include "XTaskImp.h"


using namespace std;

int server_port = 20010;

// 读回调
void read_cb(struct bufferevent *bev, void *ctx){

    // 局部栈上参数作地址传递可能丢失，回调与设置函数并非嵌套调用关系，栈上参数可能会弹出丢失

    // int fd = bufferevent_getfd(bev); // 通过bev获取fd
    int fd = (int)(intptr_t)ctx;  

    char buf[128] = {0};
    size_t ret = bufferevent_read(bev, buf, sizeof(buf));
    if(ret < 0){
        perror("bufferevent_read");
    }
    else{
        cout << "read from " << fd <<":" << buf << endl;
    }
}
// socket链接发生异常时触发，读写以外的其他事件类型
void event_cb(struct bufferevent *bev,short what, void *ctx){
    int fd = (int)(intptr_t)ctx;   
    if(what & BEV_EVENT_CONNECTED){ 
        cout << "connect success fd:"<< fd << endl;
    }else if(what & BEV_EVENT_ERROR){
        cout << "connect failed" << endl;
    }else if(what & BEV_EVENT_EOF){
        cout << "connect close fd:" << fd << endl;
        bufferevent_free(bev);
    }else{
        cout << "connect error what:" << what << endl;
    }
}   

// 新连接回调
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, void * arg){
    cout << "receive client connected, fd:" << fd << endl;
    XTask *task = new XTaskImp();
    XThreadPool::Get()->Dispatch(task);
    struct event_base *base = (struct event_base *)arg;

    // 用于创建一个套接字事件对象​（bufferevent），它封装了底层的 socket 操作（如读、写、事件回调），并提供了 ​自动缓冲​ 和 ​事件驱动​ 的能力。
    // 对已存在的socket对象进行封装
    // struct bufferevent *bufferevent_socket_new(
    //     struct event_base *base,      // 事件循环上下文
    //     evutil_socket_t fd,           // 已存在的 socket 文件描述符
    //     int options                   // 选项标志（如 BEV_OPT_CLOSE_ON_FREE）
    // );
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE); // bev 带缓冲区的ev，BEV_OPT_CLOSE_ON_FREE对象释放时关联的socket也会被关闭
    if(bev == NULL){
        perror("bufferevent_socket_new");
        exit(1);
    }
    // 设置读写回调、其他事件回调
    bufferevent_setcb(bev, read_cb, NULL, event_cb, (void*)fd);
    // 设置监听事件类型
    bufferevent_enable(bev, EV_READ);
}

int main(int, char**){

    // 1. 初始化线程池
    XThreadPool* pool =XThreadPool::Get();
    pool->Init();

    // 事件轮询
    struct event_base *base = event_base_new();
    if(!base){
        perror("event_base_new()");
        exit(1);
    }
    // addr
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(server_port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡（0.0.0.0）

    /*
        socket
        bind    ==>  evconnlistener_new_bind
        listen
        accept
        其内封装创建socket、绑定、监听、接受连接。
        accept也是在其内部完成的，cb回调是在accept后调用的
    */
    // 创建socket、绑定、监听、接受连接
    // 在一个给定地址上分配一个evconnlistener去监听tcp连接
    // Allocate a new evconnlistener object to listen for incoming TCP connections on a given address.
    // 会event_add添加到添加到event_base中,不用像底层事件一样手动添加
    struct evconnlistener *listener = evconnlistener_new_bind(
        base, // 绑定的事件轮询上下文
        listener_cb, // 回调函数
        base, // 回调函数的参数
        // LEV_OPT_CLOSE_ON_FREE 释放监听器时，关闭socket
        // LEV_OPT_REUSEABLE 允许端口复用
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        10, // TCP队列长度
        (struct sockaddr*)&sin,
        sizeof(sin)
    );

    if (listener == NULL) {
        perror("evconnlistener_new_bind()");
        exit(1);
    }
    // 开启轮询
    event_base_dispatch(base);
    // 释放两个对象
    if(listener)
        evconnlistener_free(listener);
    if(base)
        event_base_free(base);

    return 0;
}