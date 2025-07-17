参考memcached

为了跨平台，进程通信使用一对socket

```cpp
class XThreadPool{
    // 初始化线程
    // 创建好线程XThread对象
    bool Init(int threadCount = 10);
    // 分配任务到线程，使用基础轮询算法
    // 每个线程处理多个任务
    // 添加任务 XTask
    // 激活线程去处理
    bool Dispatch(XTask *task);
}

class XThread{

    // 安装线程，初始化后的libevent的事件
    // 创建用于线程激活的管道（windows用互相通信的socket socketpair）
    bool Setup();

    // 开始线程的运行，用C++11的thread
    void Start();

    void Main(); // 线程函数，libevent事件循环

    void Activate(); // 向此线程发出激活的管道消息

    void AddTask(XTask *task); // 添加处理任务到线程的任务队列，一个线程可以同时处理多个任务，他们公用一个event_base； 用了锁线程安全

    void Notify(evutil_socket_t fd); // 激活这个对象的线程，让他马上开始处理任务

}

class XTask {
    virtual void Init() = 0; //任务初始化接口
    struct event_base *base;
    int sock;
}

```