#pragma once

class XTask {
public:
    struct event_base *base_ = 0;
    int sock_ = 0; // 新连接回调时候保存
    int threadId_ = 0; // XThread::AddTask时候保存
    // 初始化任务
    virtual bool Init() = 0; // ReadNotify 线程读取消息并执行回调中执行

};