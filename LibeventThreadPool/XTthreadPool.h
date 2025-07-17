#pragma once

#include <mutex>
#include <vector>

class XThread;
class XTask;

class XThreadPool { 
public:
    ~XThreadPool();

    static XThreadPool* Get();
    // 创建好线程XThread对象
    bool Init(int threadCount = 10);
    // 分配任务到线程，使用基础轮询算法
    // 每个线程处理多个任务
    // 添加任务 XTask
    // 激活线程去处理
    void Dispatch(XTask *task);
    XThread *GetThread();
private:
    XThreadPool();

    // 线程数量
    int threadCount_ = 0;
    // 轮询线程
    int lastThreadId_ = -1;
    // 线程池线程
    std::vector<XThread*> threads_;
    std::mutex mutex_; // 线程池队列锁
};




