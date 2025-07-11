#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <iostream>
#include <event.h>

using namespace std;

int signal_count = 0;

void callback(evutil_socket_t fd, short, void* arg) {
        cout << "receive signal: " << fd << endl; 

        event *ev = (event*)arg;

        if(++signal_count>=3){
            event_del(ev);
        }
    }

int main(int, char**)
{
    // 创建事件轮询
    event_base* base = event_base_new();

    // 创建事件
    struct event ev;
    /*
        event
        event_base
        signal 要监听的信号
        events 事件类型 EV_SIGNAL标识监听的是信号类型
    */
    // 绑定事件，事件轮询，感兴趣的事件类型
    event_assign(&ev, base, SIGINT, EV_SIGNAL | EV_PERSIST, callback, event_self_cbarg());
    event_add(&ev, NULL); // 添加到轮询
    event_base_dispatch(base); // 启动事件轮询，事件列表清空自动结束
    

    event_base_free(base);

    return 0;
}
