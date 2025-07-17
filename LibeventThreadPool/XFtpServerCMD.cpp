#include "XFtpServerCMD.h"
#include <iostream>

XFtpServerCMD::XFtpServerCMD()
{
    
}

// 初始化任务
bool XFtpServerCMD::Init()
{
    std::cout << "XFtpServerCMD::Init" << std::endl;
    // 监听socket bufferevent
    return true;
}
