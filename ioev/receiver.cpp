#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <event.h>
#include <unistd.h>

// ev事件回调
void callback(evutil_socket_t fd, short what, void * arg)
{
    struct event* ev = (struct event*)arg;

    char buf[32] = {0};
    int ret = read(fd, buf, sizeof(buf)); 
    if(-1 == ret){
        perror("read");
        exit(1);
    }else if(0 == ret){
        close(fd);  // 关闭文件描述符
        event_del(ev);
        std::cout << "client closed" << std::endl;
    }else{
        std::cout << "read from fifo: " << buf << std::endl;
    }
}   
/*
    receiver
*/
int main(int, char**){
    const char* fifo_path = "fifo.tmp";

    // 1. 尝试直接打开 FIFO（如果已存在）
    int fd = open(fifo_path, O_RDONLY); // 如果另一端没有创建，则打开会阻塞
    if (fd != -1) {
        // FIFO 已存在，直接打开成功
        std::cout << "FIFO already exists, opened successfully." << std::endl;
    } else {
        // 2. 如果打开失败，检查是否是因为 FIFO 不存在（errno == ENOENT）
        if (errno == ENOENT) {
            // FIFO 不存在，尝试创建它
            int ret = mkfifo(fifo_path, 0700);
            if (ret == -1) {
                perror("mkfifo");
                exit(1);
            }
            std::cout << "FIFO created successfully." << std::endl;

            // 3. 创建成功后，再次尝试打开
            fd = open(fifo_path, O_WRONLY);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            std::cout << "FIFO opened after creation." << std::endl;
        } else {
            // 其他错误（如权限不足等）
            perror("open");
            exit(1);
        }
    }
    // 初始化事件集合
    /*
        初始化事件集合，其实就是调用了event_base_new_with_config()，创建event_base对象，
        并且赋值给了全局变量 struct event_base *current_base;
    */
    event_init();

    // 创建事件
    struct event ev;

    // 初始化事件（把事件ev和fd的感兴趣的事件绑定，fd发生事件时会触发回调。相当于muduo的channel）
    // 不持久化，程序退出，fd自动被回收，触发对端SIGPIPE信号，结束对端进程。
    // EV_PERSIST 持久化事件，如果不持久化，fd事件触发一次就会结束，要再次监听，必须event_add(&ev, NULL) 重新添加事件到事件循环。
    // event_self_cbarg() 实际上就是标记自身结构体，调用回调时，传递自己的指针。
    event_set(&ev, fd, EV_READ | EV_PERSIST,callback,event_self_cbarg()); // 事件ev、文件描述符fd、感兴趣的事件、回调、回调参数

    // 把事件添加到事件集合中，并设置超时时间（如果设置，超时会触发EV_TIMEOUT标志通知用户）
    event_add(&ev, NULL);

    // 开始监听，开始事件循环
    event_dispatch(); // 死循环，但如果集合没有事件可以监听，则返回

    return 0;
}

/*
    pipe_fd 管道对端即使关闭，任然是可读状态，会持续触发回调。
*/
